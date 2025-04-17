# PeTrack

[![Documentation Status](https://readthedocs.org/projects/petrack/badge/?version=latest)](https://petrack.readthedocs.io/en/latest/?badge=latest)
[![pipeline status](https://jugit.fz-juelich.de/ped-dyn-emp/petrack/badges/master/pipeline.svg)](https://jugit.fz-juelich.de/ped-dyn-emp/petrack/-/commits/master) 
[![Latest Release](https://jugit.fz-juelich.de/ped-dyn-emp/petrack/-/badges/release.svg)](https://jugit.fz-juelich.de/ped-dyn-emp/petrack/-/releases)
[![download](https://img.shields.io/badge/download-here-brightgreen?link=https://go.fzj.de/petrack-download&link=https://go.fzj.de/petrack-download)](https://go.fzj.de/petrack-download)
[![DOI](https://zenodo.org/badge/DOI/10.1016/j.neucom.2012.01.036.svg)](https://doi.org/10.1016/j.neucom.2012.01.036)
[![DOI](https://zenodo.org/badge/doi/10.5078176/zenodo.5078176.svg)](https://doi.org/10.5281/zenodo.5078176)
[![License](https://img.shields.io/badge/License-GPLv3-blue)](https://opensource.org/license/gpl-3-0)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md) 

![PeTrack logo](/icons/logo.png)

For the understanding of the dynamics inside crowds reliable empirical data are needed enabling an increase of safety and comfort for pedestrians and the design of models reflecting the real dynamics. PeTrack (**Pe**destrian **Track**ing) automatically extracts accurate trajectories of marked pedestrians from video recordings. The joint trajectories of all pedestrians provide data like velocity, flow and density at any time and position. With such a tool extensive experimental series with a large number of persons can be analyzed. Individual codes enables personalized trajectories with static information of each participant (e.g. age, gender).

The program deals with wide angle lenses and a high density of pedestrians. Lens distortion and perspective view are taken into account. The procedure includes calibration, recognition, tracking and height detection.
Different kinds of markers (e.g. with height information, head direction, individual code) are implemented. With a stereo camera more accurate height measurements and also markerless tracking is possible.


## Download & installation:
We publish installers for windows. To get the latest installer, visit this
[webpage](https://go.fzj.de/petrack-download) and download the current installer. 

## How to use PeTrack

To learn how to use PeTrack, please have a look at our documentation [here](https://petrack.readthedocs.io/). 

> Note that PeTrack can only be used if some steps are considered **during the planning and execution of the experiments**. This includes, but is not limited to, calibration and selecting hats. See [Planning of Experiments](https://petrack.readthedocs.io/en/latest/planning/planning.html)

After all of that, enjoy successful recognition and tracking 😃
![Picture of pedestrians with colored hats with indications of detected heads and tracked past trajectory](/docs/source/user_interface/images/past_path.png)

Some documentation is built into PeTrack itself. For example, you can use the `help`-menu to find a list of all keybindings. For running in a "batch-mode", PeTrack has a CLI, whose options are listed by calling
```bash
petrack -help
```

## Tutorial & demo
A small demo project is provided in the `demo` folder. It contains all necessary
files for setting up a project with PeTrack. You can download these files via
[this link](https://go.fzj.de/petrack-demo-download). A tutorial leading through
all steps can be found in the
[Documentation](https://petrack.readthedocs.io/en/latest/getting_started.html).
The intermediate project files for each step are also included in the `demo`
folder.

## How to cite
Please cite the general paper and the corresponding software version in your publications if PeTrack helps your research.

General Paper:
- Boltes, M. and Seyfried, A.: Collecting Pedestrian Trajectories; In: Neurocomputing, Special Issue on Behaviours in Video, vol. 100, pp. 127-133 (2013)
<details><summary>BibTeX</summary>

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
</details>

Software Version:

<details><summary>BibTeX for current version</summary>

```
@software{boltes_2025_15119517,
  author       = {Boltes, Maik and
                  Kilic, Deniz and
                  Schrödter, Tobias and
                  Arens, Tobias and
                  Dreßen, Luke and
                  Adrian, Juliane and
                  Boomers, Ann Katrin and
                  Kandler, Alica and
                  Küpper, Mira and
                  Graf, Arne and
                  Salden, Daniel and
                  Brualla, Ricardo Martin and
                  Häger, Paul and
                  Hillebrand, Daniel and
                  Lieberenz, Paul and
                  Klein, Janine},
  title        = {PeTrack},
  month        = apr,
  year         = 2025,
  publisher    = {Zenodo},
  version      = {v1.0},
  doi          = {10.5281/zenodo.15119517},
  url          = {https://doi.org/10.5281/zenodo.15119517},
}
```
</details>

To find your corresponding version, check the `about` menu or look into the terminal output of PeTrack. To cite PeTrack as software without specifying a version, use the DOI [10.5281/zenodo.5078176](https://doi.org/10.5281/zenodo.5078176).


## License
This project is licensed under the terms of the GPLv3 license. For further information see [LICENSE](./LICENSE).

## Changelog:
See [CHANGELOG.md](./CHANGELOG.md)


## Dependencies:
* Qwt         (https://qwt.sf.net)
* Qt          (https://www.qt.io/)
* OpenCV      (https://opencv.org/)
