from threading import Thread
from time import sleep
import socket
from System.Windows.Forms import Screen
import scriptcontext
from Rhino.Display import DefinedViewportProjection, RhinoView, RhinoViewport
from System.Drawing import Rectangle
import re
import json

#This is the name of the display to use; displays are named by their Windows display numbers
DISPLAY_NAME = "\\\\.\\DISPLAY2"

#Rhino does not position the windows as specified
#The following compensation values must be determined experimentally
COMPENSATION_X_LEFT = -6
COMPENSATION_X_RIGHT = -24
COMPENSATION_WIDTH = 16
COMPENSATION_Y = 0
COMPENSATION_HEIGHT = 5

#Stereo 3D settings
#Interocular distance for people has a mean of 63mm with std ~3-4mm
#This is irrelivant though because CAD isn't always in real units
INTEROCULAR_DISTANCE = 1.0

#Find our desired display by searching all displays available
found = False
for screen in Screen.AllScreens:
    if screen.DeviceName == DISPLAY_NAME:
        displayBounds = screen.Bounds
        found = True
        break
    else:
        continue
        
#Create Window sizing and positioning based on display selected
width = int(displayBounds.Width/2) + COMPENSATION_WIDTH
height = displayBounds.Height + COMPENSATION_HEIGHT
y = displayBounds.Y + COMPENSATION_Y
xleft = displayBounds.X + COMPENSATION_X_LEFT
xright = displayBounds.X+width + COMPENSATION_X_RIGHT

#Create the new Rhino views and capture their RhinoView instances
leftEye_View = scriptcontext.doc.Views.Add("LeftEye",DefinedViewportProjection.Perspective, Rectangle(xleft,y,width,height),True)
rightEye_View = scriptcontext.doc.Views.Add("RightEye",DefinedViewportProjection.Perspective, Rectangle(xright,y,width,height),True)
leftEye = leftEye_View.ActiveViewport
rightEye = rightEye_View.ActiveViewport

leftEye.SetCameraLocation(leftEye.CameraLocation-leftEye.CameraX*INTEROCULAR_DISTANCE/2.0, False)
rightEye.SetCameraLocation(rightEye.CameraLocation-rightEye.CameraX*INTEROCULAR_DISTANCE/2.0, False)

#Get Position and look at vector from external
#Expected response like: {"pos":[3,2,1], "lookAt":[1,2,3]}
lastPos = None
lastLook = None

def updateEyes(data):
    try:
        jdata = json.loads(data)
    except ValueError:
        print("no json", data)
        return
    userPos = (jdata['pos'][0], jdata['pos'][1], jdata['pos'][2])
    lookAt = (jdata['lookAt'][0], jdata['lookAt'][1], jdata['lookAt'][2])
    print("POSITION", userPos)
    print("LOOK_AT", lookAt)
    if lastLook is not None and lastPos is not None:
        #TODO: Update camera location by dPosition
        leftEye.SetCameraLocation(leftEye.CameraLocation-leftEye.CameraX*INTEROCULAR_DISTANCE/2.0, False)
        rightEye.SetCameraLocation(rightEye.CameraLocation-rightEye.CameraX*INTEROCULAR_DISTANCE/2.0, False)
        #TODO: Update camera rotation by dLookAt
        leftEye.SetCameraDirection(leftEye.CameraDirection, False)
        rightEye.SetCameraDirection(rightEye.CameraDirection, False)
    lastPos = userPos
    lastLook = lookAt

def eyeUpdate():
    print('attempting connect to vision server')
    while True:
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect(('localhost', 25565))
            sock.sendall("""GET /vector/ HTTP/1.1\nHost: localhost:25565\n\n""".encode())
            data = sock.recv(2048).decode()
            sock.close()
            l = data.split('\n')
            while '' in l:
                l.remove('')
            payload = l[-1]
            updateEyes(payload)
        except socket.error:
            print('Socket error, host unavailable')
            sleep(5)

thread1 = Thread(target=eyeUpdate)
thread1.start()

def handleUIevent(descriptor):
    print(descriptor)

#Handle inbound POSTS for UI
#Expects a payload like action=btnName
def serveUI():
    HOST, PORT = '127.0.0.1', 11000
    listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listen_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listen_socket.bind((HOST, PORT))
    listen_socket.listen(1)
    print 'Serving HTTP on port %s ...' % PORT
    while True:
        client_connection, client_address = listen_socket.accept()
        data = client_connection.recv(1024).decode()
        l = data.split('\n')
        while '' in l:
            l.remove('')
        payload = l[-1]
        payload = payload.replace('action=', '')
        handleUIevent(payload)
        if data.startswith("POST /UI"):
            http_response = """\
HTTP/1.1 200 OK

ACK
"""
        else:
            http_response = """\
HTTP/1.1 404 NOT FOUND

Only /UI/ works presently
"""
        client_connection.sendall(http_response)
        client_connection.close()

thread2 = Thread(target=serveUI)
thread2.start()