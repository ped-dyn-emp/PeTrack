# Set Up Tracking

## General Settings

At the top of the tracking tab, you can find the general 
settings (image) of tracking. In the end you have two 
options to 
apply the tracking to your project.

online calculation
: Enables the tracking, but you manually have to navigate 
though the video.

calculate all
: Enables the tracking and runs the entire video 
forward and backward automatically

**However, other settings should be fixed beforehand.**

repeat below quality 
: If the pedestrian already has a tracked trajectory (e.g.
when running the video backwards, after running it forwards), tracking is only
repeated if this is checked and the quality is below the threshold. Otherwise,
the existing point is not overwritten.

extrapolation for big diff.
: If there is a huge difference in speed from one
frame to another, discard the value and instead extrapolate. Note that
extrapolation is **at most applied 3 times** in a row. After this, the
trajectory is terminated.

merge
: Allows you to merge two trajectories into one 
continuous trajectory. See [more](\correction\correction_workflow.md).
*link to Miras page /correction/workflow/merge/*

only selected
: Enables that tracking settings will only be applied to 
the selected subjects. This works only in combination 
with the `show only people` or the `show only 
people list` option in the `path` section at the 
bottom of the tracking tab.


export
: Exports trajectories in desired 
format.

import
: Imports trajectories from files.

reset 
: Deletes all trajectories. This is useful during tuning of parameters to
allow a view of the current settings (recognition and tracking) applied to the
current frame.

## Region Of Interest

The region of interest (ROI) is the part of the image 
where the tracking is performed. For the ROI we see the 
options

show
: Shows the ROI as a blue rectangle on the image.

fix
: "Fixes" the rectangle, such that it cannot be moved 
with the mouse.

full image
: Sets the ROI to the full image.

adjust automatically
: Sets the ROI as one head size larger than the 
recognition ROI

:::{warning}
The tracking ROI must be larger than the recognition ROI.
Any detection outside the tracking ROI will create an 
entirely new, one point trajectory.
:::

## Export Settings

In this section, various export settings can be chosen.
For more information about the settings as well as 
information on importing and exporting files, check out the 
[Trajectory Import and Export](\export\trajectory.md) section.


## Search Region

scale
: Adapts the window size of the search region in which the 
Lucas-Kanade[^1] method is applied.
[^1]: Jean-Yves Bouguet. [Pyramidal implementation of the affine lucas kanade feature tracker description of the algorithm.](https://www.semanticscholar.org/paper/Pyramidal-implementation-of-the-lucas-kanade-Bouguet/b7c23b6c391af773821d66b2f9f7625db66e4e29?utm_source=direct_link) Intel Corporation, 5, 2001.

levels
: Adapts the number of search regions.

max. error
: Adapts the error acceptance value. A person is no longer 
tracked if the calculated error of the tracking is higher 
than an acceptance value.

show pyramidal search size
: Enables the visual representation of the search regions.

adaptive level
: Enables the re-tracking at a lower pyramid level for failed 
tracking attempts. Mostly leads to poorer trajectories!

:::{Tip}
If you are tracking pedestrians with colored hats, make sure 
to include textured areas (e.g. the border of the hats) into 
the search region. There the hat displays different intensity 
levels of color which are essential to the Lukas-Kanade method.
:::


** After you have set all of these settings, you can 
run the tracking with either `online calculation` or 
`calculate all` as mentioned on top of this page.**


## Path Settings

In this section, the depiction of the trajectories as well as 
additional information about them can be adjusted.

show only people
: Only the trajectory of the selected person will be 
displayed. The number represents the PeTrack ID which are 
assigned in an ascending order.

show only people list
: Only the trajectories of selected people will be displayed. 
You can either select the PeTrack IDs from the `list` or 
write them in the box separated by commas. Ranges are also possible,
i.e. `7,15-20` is a valid entry and shows person Nr 7 and persons
from 15 to 20 (including ends).

frames before/after actual position
: Number of frames before/after the actual position for which 
the trajectories should be visible.