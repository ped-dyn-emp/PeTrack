# PeTrack
For the understanding of the dynamics inside crowds reliable empirical data are needed enabling an increase of safety and comfort for pedestrians and the design of models reflecting the real dynamics. The existing database is small, sometimes inaccurate and highly contradictory. Manual procedures for collecting this data are very time-consuming and usually do not supply sufficient accuracy in space and time.

For this reason we are developing the tool named `PeTrack` (**Pe**destrian **Track**ing) to automatically extract accurate pedestrian trajectories from video recordings. The joint trajectories of all pedestrians provide data like velocity, flow and density at any time and position. With such a tool extensive experimental series with a large number of persons can be analyzed. Individual codes enables personalized trajectories with static information of each participant (e.g. age, gender).

The program has to deal with wide angle lenses and a high density of pedestrians. Lens distortion and perspective view are taken into account. The procedure includes calibration, recognition, tracking and height detection.

Different kinds of markers (e.g. with height information, head direction, individual code) are implemented. With a stereo camera more accurate height measurements and also markerless tracking is possible.

An executable of `PeTrack` is available [here](https://go.fzj.de/petrack-download).

<!-- The brief [documentation](usage/Usage) of using `PeTrack` cannot answer all questions. Thus you may read also the information [workflow](usage/Workflow-for-performing-experiments-to-be-analyzed-with-PeTrack) and [contact the author](https://go.fzj.de/petrack-boltes) before setting up experiments and automatic extraction with `PeTrack`. -->

<!--![Download Video](uploads/9a171c82e9bde1891c2a99eca38ded08/petrackAblaufArena.mp4)

![Download Video](uploads/2c50611b2fdbfcc3e4a33c609b85dffa/petrackArenaResult.mp4)

![Download Video](uploads/31c514747c6ad122b4d27b24a9ccbf67/petrackAblaufLarge.mp4)-->

:::{toctree}
:maxdepth: 1
getting_started
user_interface
planning/planning
calibration/calibration
recognition/recognition
tracking/tracking
correction/correction
misc/misc
contribute/contribute
section/MeinTest
:::
