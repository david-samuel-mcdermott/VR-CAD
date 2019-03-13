# VR-CAD
Software to enable CAD usage for Stuckeman Center for Design Computing stereoscopic 3D table.

## Git Repository Workflow
In this Git repository, the master branch is merge protected; each merge into master requires a code review from at least one other person. To work on a feature or bug fix, please create a new branch as your working space and merge that branch to master once your task is completed.  This helps to maintain a clean codebase, which will be imperitive for collaboration.

Releases will be drafted from the master branch with each folder in the master branch corresponding to a plugin for one CAD program.  This will allow for the project to be continued by future groups if needed.  By drafting releases, users may update their plugin as new versions are developed and functional versions of the project can be saved easily.

## Repository Subdirectories
### RhinoPlugin
This directory contains the Python code used to communicate with Rhino3D.  This should contain one script any its necessary supporting files.  This script creates the viewports for rhino, launches a server to handle updates from the touch overlay, and connnects to the user tracking server to continuously update the user's position.
### TableToUser
This directory contains the Win32 and OpenCV code used to track the user and create the vector from the table to the user; this acts as a server, receiveing HTTP GET requests for user position and sending JSON responses to the Python code running on Rhino.
### TouchOverlay
This directory contains the .NET project used to create the Windows Froms application that lays over the Rhion windows to capture user touch gestures and display buttons.  This overlay can be developed using Visual Studio's graphical interface developer via `OverlayForm.Designer.cs` or in code via `OverlayForm.cs`.  This application will send HTTP POST data to the server running on the Rhino script.  

## Tracking working issues
Use Visual Studio's task list to identify tagged lines for places to develop at or improve code.  Common tags to look for are `TODO`, `NOTE`, and `HACK`.

## IDE Configuration
Microsoft's Visual Studio is available for [download](https://visualstudio.microsoft.com/vs/) on Winows 10 and MacOS X.  While not strictly necessary, the provided projects are in Visual Studio format since our sponsor is using a Windows 10 platform.  It is assumed that OpenCV is installed in `C:\\OpenCV`; if this is not the case, then reconfiguration will be required to compile OpenCV code.  

## Hardware Configurations
Presently, the table design calls for a stereoscopic 3D television embedded in a table, with a protective glass topping and a touch sensor placed on the glass.  An Xbox Kinect is used to track motion and a PC runs the host application with a USB connection to the Kinect.  THe table is run as a second display in Windows.
### 3D TV Configurations
By default, most displays will not be properly configured for immersive applications; there will be unusable areas of the screen and Windows will try to consume more screen area than it should.  
 * Remove display overscan
   * For our model this was done by setting `Picture Size` to `Screen Fit` in the menu
 * Disable the Windows taskbar on the display
   * For Windows 10, build 17134, this is done in the settings app by searching for `Taskbar settings` and setting `Show taskbar on all displays` to `off`
 * Disable display scaling on the display
   * For Windows 10, build 17134, this is done in the settings app under `Display settings`, selecting the 3D TV and setting `Change the size of text, apps, and other items` to `100%`  
