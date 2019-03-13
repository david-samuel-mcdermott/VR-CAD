
from System.Windows.Forms import Screen
import scriptcontext
from Rhino.Display import DefinedViewportProjection, RhinoView, RhinoViewport
from System.Drawing import Rectangle

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
for screen in Screen.AllScreens:
    if screen.DeviceName == DISPLAY_NAME:
        displayBounds = screen.Bounds
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

#TODO: launch new thread to handle inbound data from touch overlay

#TODO: loop requests to tabletouser to update camera locations
