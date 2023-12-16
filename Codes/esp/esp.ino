
#include <Wire.h>
#include "MAX30100.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "ESPAsyncWebSrv.h"
#include "ArduinoJson.h"
#include <WiFiUdp.h>

#define SERIAL_BAUD 115200
const char * udpAddress = "192.168.4.4";
const int udpPort = 4444;

char msg[25];


WiFiUDP udp;


#include <Firebase_ESP_Client.h>


#include "addons/TokenHelper.h"
#include "time.h"
#include "addons/RTDBHelper.h"

#define SAMPLING_RATE                       MAX30100_SAMPRATE_100HZ

#define IR_LED_CURRENT                      MAX30100_LED_CURR_50MA
#define RED_LED_CURRENT                     MAX30100_LED_CURR_27_1MA

#define PULSE_WIDTH                         MAX30100_SPC_PW_1600US_16BITS
#define HIGHRES_MODE                        true


#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"


#define API_KEY "API_KEY"


#define USER_EMAIL "EMAIL"
#define USER_PASSWORD "PASSWORD"

#define DATABASE_URL "https://heart-monitor-d1702-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

String name = "sandeep";
HTTPClient http;


// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String v_path = "/vata";
String p_path = "/pitta";
String k_path = "/kapha";

String timePath = "/timestamp";


String parentPath;

String ssid = WIFI_SSID;
String password = WIFI_PASSWORD;

int timestamp = 0;
FirebaseJson json;

const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

const char* ntpServer = "pool.ntp.org";

#define REPORTING_PERIOD_MS     50

typedef struct {
  uint16_t v;
  uint16_t p;
  uint16_t k;
} Data;

xQueueHandle xQueue;

MAX30100 pox1, pox2, pox3;

uint32_t tsLastReport = 0;

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}


void onBeatDetected()
{
    Serial.println("Beat!");
}

void selectbus(uint8_t bus) {
  Wire.beginTransmission(0x70);
  Wire.write(1<<bus);
  Wire.endTransmission();
}

AsyncWebServer server(6969);
void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    Wire.begin();

    int i = 0;
    while(WiFi.status() != WL_CONNECTED && i < 500) {
      delay(500);
      Serial.print(".");
      i++;
    }

  server.on("/get_data", HTTP_GET, [](AsyncWebServerRequest *request){
        // Code that holds the request
        Serial.println("Get received"); // Just for debug
        request->send(200, "text/plain", "debug get");
    });

    // Example of function that holds and HTTP_POST request on 192.168.4.1:80/set_data
    server.on("/set_data", HTTP_POST, [](AsyncWebServerRequest *request){
      int params = request->params();
      Serial.println(params);
      for (int i = 0; i < params; i++) {
        AsyncWebParameter* p = request->getParam(i);
        if(strcmp("name", p->name().c_str()) == 0) {
          Serial.print("1");
          name = p->value().c_str();
        }
        else {
          Serial.println("2");
        }
        Serial.print(p->name().c_str());
        Serial.println(p->value().c_str());
      }
      request->send(200, "text/plain", "debug post"); 

    });

    server.begin();
    http.begin("http://192.168.0.100/");
    Serial.println(WiFi.localIP());

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  if(WiFi.status() == WL_CONNECTED) {
    // Getting the user UID might take a few seconds
    Serial.println("Getting User UID");
    while ((auth.token.uid) == "") {
      Serial.print('.');
      delay(1000);
    }
    Serial.print("User UID: ");
  }

  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid;

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


    xQueue = xQueueCreate(300, sizeof(Data));

    Serial.print("Initializing pulse oximeter..");

    selectbus(1);
    bool state1 = pox1.begin();
    selectbus(2);
    bool state2 = pox2.begin();
    selectbus(3);
    bool state3 = pox3.begin();

    if (!state1 || !state2 || !state3) {
      if(!state1) Serial.println("Bus 0");
      if(!state2) Serial.println("Bus 2");
      if(!state3) Serial.println("Bus 7");
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }

    selectbus(1);
    pox1.setMode(MAX30100_MODE_SPO2_HR);
    pox1.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT);
    pox1.setLedsPulseWidth(PULSE_WIDTH);
    pox1.setSamplingRate(SAMPLING_RATE);
    pox1.setHighresModeEnabled(HIGHRES_MODE);

    selectbus(2);
    pox2.setMode(MAX30100_MODE_SPO2_HR);
    pox2.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT);
    pox2.setLedsPulseWidth(PULSE_WIDTH);
    pox2.setSamplingRate(SAMPLING_RATE);
    pox2.setHighresModeEnabled(HIGHRES_MODE);

    selectbus(3);
    pox3.setMode(MAX30100_MODE_SPO2_HR);
    pox3.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT);
    pox3.setLedsPulseWidth(PULSE_WIDTH);
    pox3.setSamplingRate(SAMPLING_RATE);
    pox3.setHighresModeEnabled(HIGHRES_MODE);


    xTaskCreatePinnedToCore(
      read, 
      "read", 
      10000, 
      NULL, 
      1, 
      NULL, 
      1
    );
}


void read(void * parameters){
  Data data;
  BaseType_t xStatus;


  while(1) {
    
    if(WiFi.status() == WL_CONNECTED) {
      xStatus = xQueueReceive( xQueue, &data, 0);

      if(xStatus == pdPASS){

        timestamp = getTime();

      parentPath= databasePath + "/" + name + "/" + String(timestamp);

      json.set(v_path.c_str(), String(data.v));
      json.set(p_path.c_str(), String(data.p));
      json.set(k_path.c_str(), String(data.k));
      json.set(timePath, String(timestamp));
      Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json);
      }
    }

  }
}

void loop()
{
selectbus(1);
    pox1.update();

    selectbus(2);
    pox2.update();

    selectbus(3);
    pox3.update();


    uint16_t val1, val2, val3, temp;
    selectbus(1);
    while (pox1.getRawValues(&val1, &temp)) {
        // Serial.print("Vaata:");
        
        selectbus(1);
        pox1.getRawValues(&val1, &temp);


        selectbus(2);
        pox2.getRawValues(&val2, &temp);


        selectbus(3);
        pox3.getRawValues(&val3, &temp);
        
        Serial.println(String(val1)+","+String(val2)+","+String(val3));


      udp.beginPacket(udpAddress, udpPort);


    sprintf(msg, "%d,%d,%d", val1, val2, val3);
    
    udp.print(msg);

    udp.endPacket();

      Data data;
      BaseType_t xStatus;

      data.v = val1;
      data.p = val2;
      data.k = val3;

      xStatus = xQueueSendToFront( xQueue, &data, 0);



    }
    
}
