# Planning of Experiments

**PeTrack** allows the automatic extraction of data from video recordings. To ensure that the quality of this data is sufficient for subsequent data analysis and to minimize the effort of manual correction, the appropriate conditions should already be in place when planning and conducting the experiments. At this point, some recommendations should be given and sources of errors be pointed out, because well-planned experiments significantly influence the quality of the extracted data, if not even make it possible at all. A more detailed discussion of this topic can be found [here](https://juser.fz-juelich.de/record/187743) (Section 4.1, German).

First of all, based on the research question, it should be determined which data, to what extent, and with what **level of accuracy** should be collected, for example, only the presence of a person, approximate route, or the exact position in space. Does the position need to be accurate to 0.05 m or 0.5 m (detailed error description see [here](https://juser.fz-juelich.de/record/824345))? The sensors must be selected accordingly and the location for the experiments determined. In the following description, we focus in particular on finding the walking paths or, more precisely, the **head trajectory** of each individual person. The top head position represents the position of a person, as in densely crowded areas, for sensors like cameras that require a line of sight to the object, usually only the head and possibly the shoulders are visible. This means that the trajectory also includes the swaying of the upper body when walking and additional head-only movements. However, subsequent smoothing, which considers the human gait apparatus, allows the main direction of movement to be approximately calculated.

### Contents
:::{toctree}
:maxdepth: 1
camera
surrounding
combining
workflow
:::