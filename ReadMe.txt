PeTrack Version 0.8
===================

Software for tracking pedestrians.


Installation, Uninstallation:
-----------------------------
Just execute PeTrackInstaller_<version>.exe and follow the instructions.
To uninstall PeTrack execute PeTrackUninstaller.exe in the PeTrack application folder.


Usage:
------
The command line options can be displayed by calling
petrack -help
Key bindings can be found in the help menu of the program.

For more information see http://www.fz-juelich.de/jsc/petrack/.

Normal usage is interactive usage.


Example:
--------
To generate trajectories from a single image sequence starting with frame000.jpg to
with settings stored in project.pet, export tracker file tracker_file and exit
with saving the project to project.pet again:

petrack.exe -project project.pet -sequence frame000.jpg -autoTrack tracker_file -autoSave project.pet


History:
--------
v0.1 : only camera calibration is supported; import and export of video and image sequence
v0.2 : add first recognition
v0.3 : recognition; tracking; color mapping
v0.4 : tracking import and export; manual adding and deleting of trackpoints
v0.5 : improve robustness of tracking with interpolation and extrapolation; first analysis functionality
v0.6 : bug fixes; improve speed; expand visualization possibilities; add variable tracking region adapted to head size; provide program combine to merge trajectories
v0.7 : introduce stereo cameras; alternate height
       background subtraction especially for stereo cameras; recognition of unmarked pedestrians; integration of 4 disparity algorithms
       filter to swap picture; more video codecs; color blobs as marker using HSV triangle
       marker with two dots added; export of viewing direction
v0.8 : slanted view for external calibration, coordinate system and grid is possible, introduced multicolor marker with black dot, video support for various codecs

Using 
-----
* Qwt         (https://qwt.sf.net)
* Qt          (https://www.qt.io/)
* OpenCV      (https://opencv.org/)