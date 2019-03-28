import threading
import socket
from System.Windows.Forms import Screen
import scriptcontext
from Rhino.Display import DefinedViewportProjection, RhinoView, RhinoViewport
from System.Drawing import Rectangle
import re

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

#TODO: handle payload more sophisticatedly
def handlePayload(payload):
	"""Returns True if valid payload, else returns false"""
	return True

def inboundProcessor(client, address):
	while True:
		try:
			resp = None
			data = client.recv(2048)
			if data:
				post = data.decode('utf-8')
				if not post.startswith('POST'):
					resp = '403 FORBIDDEN \n\r\n\r'
				elif not post.startswith('POST /UI HTTP/1.1'):
					resp = '404 NOT FOUND\n\r\n\r'
				else:
					if payload = post.split("\n\r\n\r")[1]:
						resp = '200 OK\n\r\n\r'
					else:
						resp = '422 UNPROCESSABLE ENTITY'
				
				client.send(resp.encode())
				
			else:
				break;
		except:
			client.close()
			return False
	
def inboundHandler():
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.bind(('localhost', 11000))
	while True:
		sock.listen(2)
		while True:
			client, address = sock.accept()
			threading.Thread(target=inboundProcessor, args=(client, address).start()
			
threading.Thread(target=inboundHandler).start()

#TODO: handle updating eye positions
def updateEyes(data):
	pass

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 25565))
while True:
	sock.sendall('GET /TableToUser HTTP/1.1\n\rHost: 127.0.0.1\n\rUser-Agent: Rhino\n\rAccept: text, application/json\n\rAccept-Language: en-us\n\rAccept-Charset: utf-8\n\r\n\r')
	data = sock.recv(4096)
	payload = data.decode('utf-8').split('\n\r\n\r')[1]
	updateEyes(payload)