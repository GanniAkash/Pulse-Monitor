import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.12


Window {
    id: window
    width: 640
    height: 480
    visible: true
    signal openPlot;
    signal closePlot;
    signal saveData;
    property bool isSer: false;
    signal update_ports;
    signal add_name(string s);
    signal change_name_esp(int s)
    maximumWidth: 480
    minimumWidth: 480
    minimumHeight: 480
    maximumHeight: 480
    title: qsTr("Heart Rate Monitor")

    Popup {
            id: popup
            width: 300
            height: 200
            modal: true
            focus: true
            anchors.centerIn: parent

            Rectangle {
                width: popup.width
                height: popup.height
                color: "lightgrey"

                ColumnLayout {
                    anchors.centerIn: parent

                    Text {
                        text: "Enter your name:"
                        font.bold: true
                    }

                    TextInput {
                        id: userInput
                        width: 200

                        onAccepted: {
                            add_name(text)
                            popup.close()
                        }
                    }

                    Button {
                        text: "Close"
                        onClicked: popup.close()
                    }
                }
            }
        }

    Dial {

        id: dial
        width: 248
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 182
        anchors.horizontalCenterOffset: 0
        anchors.topMargin: 47
        anchors.horizontalCenter: parent.horizontalCenter
        Material.foreground: Material.Red
        value: 0
        to: 62000
        enabled: true

        Text {
            id: ts2
            objectName: "ts2"
            anchors.centerIn: parent
            text: qsTr("0")
        }
    }

        

        Button {
            id: button
            y: 378
            text: qsTr("Start")
            anchors.right: button2.left
            anchors.rightMargin: 100
            onClicked: window.openPlot()
        }

        Button {
            id: button2
            y: 378
            text: qsTr("Stop")
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: window.closePlot()

        }

        Button {
            id: button1
            y: 378
            text: qsTr("Save")
            anchors.left: button2.right
            anchors.leftMargin: 100
            onClicked: window.saveData()
        }

        ComboBox {
            id: comboBox
            x: 488
            y: 30
            width: 145
            height: 46
            focusPolicy: Qt.NoFocus
            font.kerning: false
            font.preferShaping: false
            model: ListModel {
                id: model
            }

            onCurrentIndexChanged: change_name_esp(currentIndex)

        }

        RoundButton {
            id: roundButton
            x: 40
            y: 30
            width: 49
            height: 47
            text: "+"
            onClicked: popup.open()
        }

    function updateText(t) {
        ts2.text = t;
    }

    function updateBar(v) {
        dial.value = v;
    }

    function test() {
        console.log("ji");
    }

    function pop() {
        popup.visible = true;
    }

    function add(s) {
        comboBox.model.append({text: s})
    }

    function returnPort() {
        return comboBox.currentText
    }

    function reset() {
        comboBox.currentIndex = 0;
    }

    function updatePorts() {
        comboBox.model.clear();
        add("Cloud");
        update_ports();
    }

    function change_combo(s) {
        comboBox.currentIndex = s-1
    }
}

