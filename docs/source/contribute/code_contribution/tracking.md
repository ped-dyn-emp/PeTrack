# Tracking

:::{note}
:name: notiz
This site is about the technical aspect of the tracking functionality for better understanding when
contributing to the source code.
If you are a user searching for documentation see [here](/tracking/tracking).
:::

The tracker of PeTrack is implemented on top of the [Lucas-Kanade-Algorithm](https://en.wikipedia.org/wiki/Lucas%E2%80%93Kanade_method) for determining optical flow.

Each person has an associated feature point in each frame it occurs in. This feature point gets tracked via Lucas-Kanade.[^1] Each person gets tracked with its own WindowSize (see `Petrack::winSize`). There are currently two tracking methods and multiple refinements.

## Tracking Methods

* Adaptive
    * When tracking adaptive, after a unsuccessful attempt the pyramid level of the Lucas-Kanade-Method is lowered. Sometimes this enables us to continue tracking. This gets repeated till the feature point gets successfully tracked or the minimal level of 0 is reached
* Non-Adaptive
    * When tracking non-adaptive, only one attempt with one pyramid level is executed

## Refinements

Refinements only get executed if the error in normal tracking is too high

* Track colorpoint
    * This refinement tried to track the color-point instead of the feature point. Remember that there are color points different from feature points with markers like the casern-marker or the combined multicolor-aruco-marker.
* Near dark point
    *  This method is only used with the Casern and Hermes Markers, which both have an black middle. The target of this method is to find this dark point in the middle of the marker and track that instead of the feature point.
*  BackgroundFilter - gets executed regardless of error, but needs to be enabled by user
    *  Deletes trajectories which have more successive frames in the background than a user-specified maximum

## How does a TrackPoint get added?

### Tracking

First, tracking occurs in `Tracker::track`. As one of the last statement in this function, `Tracker::insertFeaturePoints` is called. If the point is in the original image (without border) and/or has a small tracking error and is in the picture, it will be added to the tajectory, by using `TrackPerson::insertAtFrame`, which will only accept the point, if the quality increases. If merging is activated, we search for another trajectory, less then one head size away from the current one and merge these, if they werent more distant to each other in neighbouring frames.

For clarification: The point that gets tracked is already part of a trajectory and the new one gets added to the same trajectory.

### Recognition
For teh following section: If no class is mentioned, the function probably lies in recognition.cpp

First the markers are detected in their respective `find*Marker`-function. These points have coordinates respective to the reco-ROI. This difference gets corrected in the `getMarkerPos`-function. If the background-filter is enabled, it is enforced in `getMarkerPos` as well, deleting detected points which are in the background.

Then the detected points are added. This process starts at `Tracker::addPoints`, which delegates each point to `Tracker::addPoint`. Here all persons are searched whose current TrackPoint is less than one head width away from the detected point. In case of a multicolor marker with a black dot, this range is a bit bigger to take care of the difference between detected points and their position when the person is detected with or without the black dot.

If multiple people are within range of the newly detected point, we use the closest point. After the point is selected, we add them to the associated TrackPerson via `TrackPerson::insertAtFrame`.

### TrackPerson::insertAtFrame

In `TrackPerson::insertAtFrame`, points are handled differenently, depending on wether the call wants to append, prepend (mostly tracking) or replace (mostly recognition) a point. If the point to add is more than one frame away from the current first/last frame (e.g. appending a point from frame 100 to a person whose latest point was from 97), the points in between are linearly inerpolated. So the points are contigous.

If the point being appended/prepended is further away than the speed (between the last two points) would reasonably allow, the point is either not inserted (if this was the case for the last points as well) or extrapolated. The extrapolated point has a quality of 0. [Extrapolation can be turned off by the user]

If we get a point from the multicolor-marker with black-dot or aruco marker, it might get shifted. If the point before or after was shifted or recongnized the dot/code and the current point only works via the multicolor-marker, the color-detected TrackPoint gets shifted by the difference between dot/code-point and color-point. This corrects stuttering that would occur when sometimes recognizing via color and sometimes via code/dot, since these usually result in different points.

If the point to be added wants to replace a point, the quality of the points are compared. If the new point has the better quality, but is far away from the sourrounding ones, the way to the point is probably not correct and the quality of the trajectory before and after the new point gets adjusted (the part from the new point to the next properly recognized point in each direction).

The quality of manually inserted points (over 100) gets reduced to 100.

*******

[^1]: Actually, which persons get tracked specifically is determined in `Tracker::calculatePrevFeaturePoints`. This most importantly means filtering people out if "only selected") is used.