# PeTrack

For the understanding of the dynamics inside crowds reliable empirical data are needed enabling an increase of safety and comfort for pedestrians and the design of models reflecting the real dynamics. PeTrack (Pedestrian Tracking) automatically extracts accurate trajectories of marked pedestrians from video recordings. The joint trajectories of all pedestrians provide data like velocity, flow and density at any time and position. With such a tool extensive experimental series with a large number of persons can be analyzed. Individual codes enables personalized trajectories with static information of each participant (e.g. age, gender).
The program deals with wide angle lenses and a high density of pedestrians. Lens distortion and perspective view are taken into account. The procedure includes calibration, recognition, tracking and height detection.
Different kinds of markers (e.g. with height information, head direction, individual code) are implemented. With a stereo camera more accurate height measurements and also markerless tracking is possible.


## Installation, Uninstallation:
To obtain the installer of the latest stable Windows version, follow the steps in our [registration form](https://go.fzj.de/petrack-download).
Afterwards just execute `petrack-installer_<version>.exe` and follow the instructions.
To uninstall PeTrack execute `Uninstall.exe` in the PeTrack application folder.


## Usage:
The command line options can be displayed by calling
```bash
petrack -help
```

Key bindings can be found in the help menu of the program.

For more information see https://go.fzj.de/petrack-wiki.

For setting up an extraction process PeTrack is used interactively. 
To perform the extraction for multiple experiments the command line functionality is useful.


## Example:
To generate trajectories from a single image sequence starting with frame000.jpg
with settings stored in project.pet, export tracker file tracker_file and exit
with saving the project to project.pet again:

```bash
petrack.exe -project project.pet -sequence frame000.jpg -autoTrack tracker_file -autoSave project.pet
```

## Tutorial & demo
A small demo project is provided in the `demo` folder. It contains all necessary files for setting up a project with
PeTrack. A tutorial leading through all steps can be found in the
[Wiki](https://jugit.fz-juelich.de/ped-dyn-emp/petrack/-/wikis/home#for-users-of-petrack).
The intermediate project files for each step are also included in the `demo` folder.

## How to cite
Please cite the general paper and the corresponding software version in your publications if PeTrack helps your research.

In general:
- Boltes, M. and Seyfried, A.: Collecting Pedestrian Trajectories; In: Neurocomputing, Special Issue on Behaviours in Video, vol. 100, pp. 127-133 (2013)
```
@article{BOLTES2013127,
    title = {Collecting pedestrian trajectories},
    journal = {Neurocomputing},
    volume = {100},
    pages = {127-133},
    year = {2013},
    note = {Special issue: Behaviours in video},
    issn = {0925-2312},
    doi = {https://doi.org/10.1016/j.neucom.2012.01.036},
    author = {Maik Boltes and Armin Seyfried},
    keywords = {Pedestrian detection, Laboratory experiment},
}
```

Software version (v0.9):
```
@software{boltes_maik_2021_5126562,
  author       = {Boltes, Maik and
                  Boomers, Ann Katrin and
                  Adrian, Juliane and
                  Brualla, Ricardo Martin and
                  Graf, Arne and
                  Häger, Paul and
                  Hillebrand, Daniel and
                  Kilic, Deniz and
                  Lieberenz, Paul and
                  Salden, Daniel and
                  Schrödter, Tobias},
  title        = {PeTrack},
  month        = jul,
  year         = 2021,
  publisher    = {Zenodo},
  version      = {v0.9},
  doi          = {10.5281/zenodo.5126562},
  url          = {https://doi.org/10.5281/zenodo.5126562}
}
```

A list of all versions can be found [here]( https://doi.org/10.5281/zenodo.5078176).
To find your corresponding version, check the `about` menu or look into the terminal output of PeTrack.

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
