# This Python file uses the following encoding: utf-8
import os
import sys
import typing
import socket
import requests
import threading
import pandas as pd
import numpy as np
import subprocess
from PySide2.QtGui import QGuiApplication
from PySide2.QtQml import QQmlApplicationEngine
from PySide2.QtWidgets import QMessageBox, QWidget, QGridLayout, QApplication
from PySide2.QtCore import QObject, QTimer, Signal
from PySide2 import QtGui
from qt_material import apply_stylesheet
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg, NavigationToolbar2QT
from firebase_admin import db, credentials
import firebase_admin
import time
import serial
import csv
import serial.tools.list_ports

USER_NAME = "sandeep"
USER_LIST = []

ser = None

def saveData():
    data_ref = db.reference('/UsersData/RmUzAJ1yqyeC5AVOa7N63bXyePx1/' + USER_NAME + '/')
    data = data_ref.order_by_child("timestamp").get()
    r = []
    with open(f"{USER_NAME}.csv", "w+") as outfile:
        for i in data.values():
            row = []
            row.append(i["vata"])
            row.append(i["pitta"])
            row.append(i["kapha"])
            r.append(row)
        csvwriter = csv.writer(outfile)
        csvwriter.writerows(r)

def closePlot():
    # plt.close('all')
    pass

def execute_command(command):
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    output, error = process.communicate()
    
    if output:
        print("Output:")
        print(output)
    if error:
        print("Error:")
        print(error)
    
def openPlot():
    # p.ani = animation.FuncAnimation(p.fig, p.update, frames=range(100), init_func=p.init, blit=True)
    # plt.show(block=True)
    # p.ani.resume()
    command_to_execute = "python ok.py"  # Replace this with your desired terminal command

    # Create a thread to execute the command
    command_thread = threading.Thread(target=execute_command, args=(command_to_execute,))
    command_thread.start()
    

def updatePorts():
    for i in USER_LIST:
        root.add(i)

def add_name(s):
    if s in USER_LIST:
        return
    else:
        USER_LIST.append(s)
        root.add(s)
        # root.comboBox.currentText = s
        n = len(USER_LIST)
        root.change_combo(n)
        users_names_ref = db.reference('/UsersData/RmUzAJ1yqyeC5AVOa7N63bXyePx1/users/')
        users_names_ref.update({
            n:s
        })
        r = requests.post('http://192.168.102.107:6969/set_data', data={'name':s})

def change_name_esp(s):
    global USER_NAME
    USER_NAME = USER_LIST[s]
    r = requests.post('http://192.168.102.107:6969/set_data', data={'name':USER_LIST[s]})
    


if __name__ == "__main__":
    cred = credentials.Certificate("keys.json")
    f_app = firebase_admin.initialize_app(cred, {
        'databaseURL': 'DATABASE_URL',
          "storageBucket": "name"
    })
    
    app = QApplication(sys.argv)
    engine = QQmlApplicationEngine()
    apply_stylesheet(app, theme='dark_red.xml')
    engine.load(os.fspath(os.path.join(os.path.dirname(__file__), "main.qml")))
    root = engine.rootObjects()[0]
    root.openPlot.connect(openPlot)
    root.closePlot.connect(closePlot)
    root.saveData.connect(saveData)
    root.add_name.connect(add_name)
    root.change_name_esp.connect(change_name_esp)
    # root.update_ports.connect(updatePorts)
    clickMe = root.findChild(QObject, "ts2")
    popup = root.findChild(QObject, "popup")
    
    users_names_ref = db.reference('/UsersData/RmUzAJ1yqyeC5AVOa7N63bXyePx1/users/')
    users_list = users_names_ref.get()
    
    users_list.pop(0)
    
    USER_LIST = users_list
    USER_NAME = USER_LIST[0]
    
    updatePorts()
    root.change_combo(1)
    
    # timer2 = QTimer(interval=1)
    # timer2.timeout.connect(wrapper(root))
    # timer2.start()
    
    if not engine.rootObjects():
        sys.exit(-1)
    sys.exit(app.exec_())
