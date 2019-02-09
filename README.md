# VR-CAD
Software to enable CAD usage for Stuckeman Center for Design Computing stereoscopic 3D table.

## Git Repository Workflow
In this Git repository, the master branch is merge protected; each merge into master requires a code review from at least one other person. To work on a feature or bug fix, please create a new branch as your working space and merge that branch to master once your task is completed.  This helps to maintain a clean codebase, which will be imperitive for collaboration.

Releases will be drafted from the master branch with each folder in the master branch corresponding to a plugin for one CAD program.  This will allow for the project to be continued by future groups if needed.  By drafting releases, users may update their plugin as new versions are developed and functional versions of the project can be saved easily.

## User Interaction

## Application Design

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
