# PeTrack

PeTrack (**Pe**destrian **Track**ing) is a software to extract the trajectories of pedestrians from videos. It encompasses the entire workflow from calibration, recognition, tracking and manual correction.

The software supports various types of data input (image sequences, videos, stereo videos) and can even handle visibly distorted videos correctly.
PeTrack utilizes different kinds of markers (e.g. with height information, head direction, individual code). When using a marker with an individual code trajectories from PeTrack can be connected to other data sources, such as questionnaires. 

An executable of `PeTrack` is available [here](https://go.fzj.de/petrack-download).

![Flowchart of PeTrack Workflow](/images/PeTrack-Flowchart.png)


::::{grid} 2  
:gutter: 1

:::{grid-item-card}  Getting Started
:class-card: intro-card
:link: getting_started
:link-type: doc
:link-alt: To getting started
A guide through a small demo project
:::

:::{grid-item-card}  Planning of Experiments
:class-card: intro-card
:link: planning/planning
:link-type: doc
:link-alt: To planning experiments
All things to consider **before** using PeTrack.
:::

:::{grid-item-card}  How to cite
:class-card: intro-card
:link: misc/citation
:link-type: doc
:link-alt: Publications to cite when using PeTrack

Publications to cite when using PeTrack
:::

:::{grid-item-card}  Software Guide
:class-card: intro-card
:link: user_interface/user_interface
:link-type: doc
:link-alt: Start of detailed software guide

Start of the detailed software guide.
:::
::::


:::{toctree}
:maxdepth: 1
:hidden:

getting_started
planning/planning
user_interface/user_interface
calibration/calibration
recognition/recognition
tracking/tracking
correction/correction
annotationgroups/annotation_grouping
export/export
misc/misc
citation/citation
contribute/contribute
:::
