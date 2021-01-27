# PeTrack

Software for tracking pedestrians.


## Installation, Uninstallation:
To obtain the installer of the latest stable version, follow the steps at https://www.fz-juelich.de/ias/ias-7/EN/Expertise/Software/PeTrack/petrackDownload.html.
Afterwards just execute `petrack-installer_<version>.exe` and follow the instructions.
To uninstall PeTrack execute `Uninstall.exe` in the PeTrack application folder.


## Usage:
The command line options can be displayed by calling
```bash
petrack -help
```

Key bindings can be found in the help menu of the program.

For more information see https://www.fz-juelich.de/ias/ias-7/EN/Expertise/Software/PeTrack/petrack.html.

Normal usage is interactive usage.


## Example:
To generate trajectories from a single image sequence starting with frame000.jpg to
with settings stored in project.pet, export tracker file tracker_file and exit
with saving the project to project.pet again:

```bash
petrack.exe -project project.pet -sequence frame000.jpg -autoTrack tracker_file -autoSave project.pet
```

## License
This project is licensed under the terms of the GPLv3 license. For further information see [LICENSE](./LICENSE).

## Changelog:
v0.8 : slanted view for external calibration, coordinate system and grid is possible, introduced multicolor marker with black dot, video support for various codecs <br/>
v0.7 : introduce stereo cameras; alternate height <br/>
       background subtraction especially for stereo cameras; recognition of unmarked pedestrians; integration of 4 disparity algorithms <br/>
       filter to swap picture; more video codecs; color blobs as marker using HSV triangle <br/>
       marker with two dots added; export of viewing direction <br/>
v0.6 : bug fixes; improve speed; expand visualization possibilities; add variable tracking region adapted to head size; provide program combine to merge trajectories <br/>
v0.5 : improve robustness of tracking with interpolation and extrapolation; first analysis functionality <br/>
v0.4 : tracking import and export; manual adding and deleting of trackpoints <br/>
v0.3 : recognition; tracking; color mapping <br/>
v0.2 : add first recognition <br/>
v0.1 : only camera calibration is supported; import and export of video and image sequence <br/>

## Dependencies:
* Qwt         (https://qwt.sf.net)
* Qt          (https://www.qt.io/)
* OpenCV      (https://opencv.org/)
