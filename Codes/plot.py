# Import libraries
import requests
from numpy import *
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph as pg
from PyQt5.QtWidgets import QApplication
import socket

def get_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(0)
    try:
        # doesn't even have to be reachable
        s.connect(('10.254.254.254', 1))
        IP = s.getsockname()[0]
    except Exception:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP

ip = get_ip()

server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_socket.bind((ip, 4444))
server_socket.settimeout(3)

r = requests.post('http://192.168.102.107:6969/change_ip', data={'ip':ip})
print(r)

app = QApplication([])

win = pg.GraphicsLayoutWidget(title="Realtime Pulse Monitor")
p1 = win.addPlot(title="Vatta", row=0, col=0)
p2 = win.addPlot(title="Pitha", row=1, col=0)
p3 = win.addPlot(title="Kapha", row=2, col=0)
curve1 = p1.plot()
curve2 = p2.plot()
curve3 = p3.plot()

windowWidth = 400
Xm3 = linspace(0,0,windowWidth)
Xm2 = linspace(0,0,windowWidth)
Xm1 = linspace(0,0,windowWidth)
ptr = -windowWidth


def update():
    global curve1, ptr, Xm1, curve2, curve3, Xm2, Xm3
    Xm1[:-1] = Xm1[1:]
    Xm2[:-1] = Xm2[1:]
    Xm3[:-1] = Xm3[1:]
    message, address = server_socket.recvfrom(1024)
    message = message.decode()    
    v, p, k = message.split(',')
    v, p, k = int(v), int(p), int(k)
    Xm1[-1] = float(v)
    Xm2[-1] = float(p)
    Xm3[-1] = float(k)
    ptr += 1
    curve1.setData(Xm1)
    curve2.setData(Xm2)
    curve3.setData(Xm3)
    QApplication.processEvents()



timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(0.5)

win.show()


QApplication.exec_()
