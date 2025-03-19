# Correction Workflow

First, open the newly saved PeTrack file (pet.) – you might need to import the .trc file if you have moved the files after the automated tracking. Simply check if the number of tracked people is set to 0 (then you need to import the .trc) or not.
Uncheck the boxes `Perform` in the recognition tab and `online calculation` in the tracking tab.

 :::{Important}
If you don’t uncheck `Perfom` and `Online Calculation` you will overwrite the previously tracked trajectories!
:::

Now, go back to the tracking tab and scroll down to the `path` section. Check the `Show only people` box. Here you can select which persons’ path you would like to see in the picture. Usually, it is best to go through the people one by one to better filter out any errors. In specific cases, you can also correct people in batches – enable `show people list` and chose the ID’s of the people that you would like to see – but it is very easy to overlook mistakes when trying to correct multiple people at once.

Start with person 1 and find the corresponding path on the picture on the left-hand side. If there are any large irregularities, you will be able to see this right away. Irregularities can be sharp peaks, uneven spaced tracking points, or just funny looking paths. Note that these can be irregularities, but they don’t have to be!

:::{tip}
If you have used the multicolor marker, the filled in dots are points at which PeTrack is able to read out the code marker. It is less likely that these points are wrong, but it can still happen. Empty red dots are points where the code marker is not read out. Here the chance of errors is much higher!
:::

Start by setting the frame bar at the bottom of the page to 0. Now check the start position. It can happen that PeTrack picks a wrong start point for the trajectory on the head! Then you can click through the frames one by one or just press `space` to let the video play in full speed. Clicking on `start`or `end` will set the current frame to the beginning or end of the trajectory, while `mid` will jump to the middle of the trajectory.

If you want to visualize the complete path of a person, you might need to increase the values in `frames before actual position` and `frames after actual position`, or use the option `show complete path`.

:::{note}
If you have several pedestrians, make sure to change to values in `frames before/after actual position` back to smaller values, before you try to visualize more or all persons trajectories. Otherwise PeTrack will noticeably slow down!
:::

:::{note}
Be aware that `show complete path` only works if `show only people (list)` is selected!
:::

Now, check the trajectory for irregularities. Depending on the kind of errors you might come across, there are different operations for correction, which are listed and explained in the section below.  

Repeat these steps for every single identified person in the video. Don’t forget to check the `start` and `end` positions every single time to make sure a good starting/ending point is selected. 

Throughout the correction process you can press `export` at the top of the tracking tab. This will ensure that the new trajectories will be saved. Click “yes” to overwrite the old trajectory file if desired. `trc` is the working file format of `.trc`. Once all the errors are corrected you can export a `txt` file of the trajectories including various export options, see [](/export/trajectory.md).

:::{Important}
Do not forget to manually save your progress! You should export in the `.trc`-format, since it is the working file format of PeTrack.
:::


## Possible Operations for corrections
All of these correction options are accessible through a menu, e.g. reachable through a right click with the mouse, or by using the corresponding shortcut listed in the `keybindings`, which can be found under `help`. As the shortcuts differ depending on the operating system, here only the names of the features are listed. 
<!-- insert image of keybinding? Once it is readable...   -->

### Trajectory manipulation / creation:
Create new trajectory
: In case a person is not tracked, you can create a new trajectory. Go to the first frame in which this person is supposed to be tracked, disable both `show only people` and `show only people list`, and right click on the head postion of the untracked pedestrian. You can now choose `create new trajectory`. This adds the first track point to a new trajectory. To continue the tracking, follow the steps explained in "Retrack trajectory".

Retrack trajectory
: If you have deleted a part of the tracking points or a person was not tracked in a certain frame range and you want to track this portion new, you have to jump to the end of the existing trajectory. Make sure you have only selected this person in `show only people`! Check the box for `online calculation` in the tracking tab to enable the tracking again and create new tracking points by simply playing the video or moving forward frame by frame.  This only works inside the `region of interest`! After you have created all new tracking points for this one person you have to uncheck the `online calculation` box again to avoid overwriting your corrected points in the future.

Retrack with recognition
: If you also want to enable the recognition, check the `perform` box in the recognition tab again (Note, that this is optional if you already have a recognized person and only want to keep tracking it). Hereby you want to only have this one person visible (otherwise you overwrite the other trajectories again as well and could create new errors). Usually, starting by using the arrow keys to go forward frame by frame until you are sure that PeTrack follows the right way can be helpful. Sometimes PeTrack will start by just following a wrong path and if you move frame by frame you can catch the error right away.

Move trajectory point
: You can correct mildly  misplaced points by going to the corresponding 
frame, and moving your cursor to the new location and pressing `⌃ Ctrl /⌘ Cmd + left double click`. Note that you can only move the point inside the head area (circle around the head).
 
: If you would like to move a point further away, you can use `Alt /⌥ Option + hold left click` to drag a trajectory point to a new location.

: If you are already seeing many errors, you might want to delete all past or future tracking 
points and retrack to save you some time.


Split trajectory
: You can split a trajectory in two by `right click` on the head and selecting the corresponding action. This will assign all track points after the current frame to a new person, while the old person keeps the past trajectory. 

Merge trajectories
: If you have two trajectories, that overlap for at least one frame, you can merge these into one continuous trajectory. Use `show only people list` with the two IDs you wish to merge. Go to the frame before the merge point, enable `merge` and `online calculation` at the top of the tracking tab, and go forward frame by frame. Once you reached the merge point, the trajectories are merged. After this make sure to switch off `online calculation` and `merge`!

### Trajectory deletion:
Delete whole trajectory
: This can be used to delete the path of a given pedestrian completely, e.g. because a person not belonging to the experiment was tracked or one person was tracked several times. This action is accessible through a `right click` with the mouse on the corresponding head. 

Delete past of a trajectory
: This deletes all trajectory points of the selected person tracked before the current frame. Accessible by `right click` on the head.

Delete future of a trajectory
: This deletes all trajectory points of the selected person tracked after the current frame. Accessible by `right click` on the head.

Delete past or future of all trajectories
: This deletes all trajectory points of all persons  after/before the current frame. Accessible through the `Edit-menu`. 


Delete trj. moving through ROI
: Deletes the complete trajectories of persons moving through the recognition ROI. Accessible through the `Edit-menu`. 

Delete part of trj. inside ROI
: Deletes the part of a trajectory which is inside the recognition ROI. Accessible through the `Edit-menu`. 

<!-- already in User Interface
### Visualization options:
- jump to frame of track point under curser 

 - select nr./show only
- What visualization options are there and how can they help?
-->
