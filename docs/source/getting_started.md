# Getting Started 

This page goes through an exemplary project. You can download this example project [here (~200MB)](https://go.fzj.de/petrack-demo-download).

## Experimental Setup

Before PeTrack is used, experiments need to be planned (see [](/planning/planning.md)), executed and of course recorded. Part of this process is the calibration (see [](/planning/calibration.md)). At first the intrinsic calibration has to be performed, which corrects for the lens distortion of the camera. Second, the extrinsic calibration is performed, which creates known points in 3D space. Therefore we record images of a chessboard for intrinsic calibration and images of measured points for extrinsic calibration.

## Using PeTrack

The usage and some settings will be explained with the provided demo project ('demo' folder), the folder contains all necessary files to set up a PeTrack project. 

### Calibration

The first step to setup a PeTrack project is the calibration of the camera, which mainly consists of two parts [intrinsic](intrinsic_calib) and [extrinsic](extr_calib) calibration. Additionally some color correction and the addition of borders can be set up here.

(intrinsic_calib)=
#### Intrinsic calibration

<!--![]()


:::{figure} /images/getting_started_extrinsic.mp4
An embedded video with a caption!
:::-->

1. Open PeTrack. The program should automatically open in the calibration tab.
2. Click on “files” and browse to the snapshots out of the video for intrinsic calibration (chessboard), select ALL of those snapshots (for multi-selection hold `⌃ Ctrl/⌘ Cmd` and sequentially click on the files). The snapshots for the demo are located in: `demo/00_files/calibration/intrinsic`.
3. Click on “auto” to start intrinsic calibration
4. See how the parameters in the "intrinsic calibration" section have changed
5. Check “apply” and save the project to save your progress so far

![Video of the intrinsic calibration procedure in PeTrack](/images/intrinsic.mp4)

_`demo/01_calibration/01_intrinsic.pet` is the resulting PeTrack project file after these steps._

(extr_calib)=
#### Extrinsic Calibration

For the extrinsic calibration, we need to create a `.3dc` file based on the points from our sketch of the experimental setup (see [](/calibration/extrinsic_calibration.md) for more detail). Create the file in the following format:
The first line is the total number of points. Starting from the second line, all coordinates should be listed in centimeter according to their X, Y and Z coordinates. An example of the `.3dc` file can be found as `demo/01_calibration/before.3dc`.

1. Continue with PeTrack after the intrinsic calibration. Load your combined [extrinsic calibration image](/calibration/extrinsic_calibration.md#image-point-file-preparation).
2. Open the image and mark the points from your `.3dc` file **in the order, they appear in the file**. Markers can be set with `⌃ Ctrl/⌘ Cmd + double left click` and deleted with `⌃ Ctrl/⌘ Cmd + double right click`.
3. Load the points from the `.3dc` file by pressing load and browsing to the file.
4. Click “fetch”. This will use your selected image points and the points from the `.3dc` to calibrate.
5. You can check the result with the buttons “show” and “error”. Additionally, you can have PeTrack draw the virtual world coordinate system by clicking on the “show” checkbox in the “coordinate system” section.
6. Click “save” to make PeTrack add the pixel coordinates to the `.3dc` file.

![Video of the extrinsic calibration procedure in PeTrack](/images/extrinsic.mp4)

_`demo/01_calibration/02_extrinsic.pet` is the resulting PeTrack project file after these steps._ 


### Recognition


After the calibration is done, load the video file via `File/Open Sequence`. The next step is to set up the recognition. Switch to the tab “Recognition” and check “show”. Press (`⌃ Ctrl/⌘ Cmd` + `1`) to adjust view. (Alternatively, you can use the menu-bar, View > Fit in region of interest)

1. Load the video file via `File/Open Sequence` and switch to the tab "Recognition"
2. In the section "region of interest" check "show". Mouse over any image-edge (top, bottom, left, right) and click and drag the green line. In the region between the green lines recognition is performed. Only if the `fix` box is unchecked, it is possible to adjust the borders of this region.
3. Select the marker you used in the experiment. The demo experiment uses the multicolor marker with an additional code.
4. After selecting the marker type, click on "parameter" to set up the marker.
5. "open radius" and "close radius" should be deactivated when selecting colors, as the image mask does not accurately depict the selected color. To see which color range currently is detected, select "mask" and set "opacity" to 50. Then close the parameter window.
6. In the "map" section color and corresponding height of the marker can be defined:
   1. Clicking on "color range" opens a dialog where you can choose the range of colors for the currently selected group.
   2. Check "perform" to see the areas, which are included in the color range.
   3. The "height" in the "map" section represents the height that is assumed for all participants with hats in the selected color region. [See here](/recognition/setting-up.md#height-based-options) for more info on different options to get the pedestrian height.
7. Go back to the "parameter"-window. Now adjust the "open radius" and "close radius" as well as other parameters to your needs. For details, see [](/recognition/multicolor.md). 
8. In our case, since we are using the additional code marker, check "use code marker" and open the "parameter" box via the button to its right. Select the correct dictionary (in our demo case "DICT_6X6_1000") and adjust the parameter to your needs. For details, see [](/recognition/aruco.md).
9. Close the parameter window of the code marker. Then deselect the mask and close the parameter window of the multicolor marker as well. Make sure to save your progress by saving the project.

![Video of setting up the marker in PeTrack](/images/recognition_color.mp4)

_`demo/02_recognition/04_recognition.pet` is the resulting PeTrack project files after these steps._

### Tracking

After recognition is done, go to the "tracking" tab in PeTrack.

1. In the section "region of interest" check "show". You can automatically adjust the tracking region of interest (blue line) by clicking "adjust automatically" or manually adjust the lines via click and drag. If you manually adjust make sure that the tracking ROI is larger than the recognition ROI.
2. In the section "search region" adjust the parameters to your needs. For more details, see [](/tracking/setting-up.md).
3. Now start the tracking by either clicking `calculate all` which runs the tracking forward and backwards over the complete video. Or by checking `online calculation` and pressing play.
4. After the tracking is complete, make sure to save your progress in a new project file `.pet` and also save your tracked trajectories in a [`.trc` file](/export/trajectory.md).

![Video of tracking procedure in PeTrack](/images/tracking.mp4)

### Corrections

After the tracking is complete you should manually check and correct the resulting trajectories.

1. Make sure to uncheck "perform" (recognition) and "online calculation" (tracking) to not overwrite manual changes.  
2. Adjust the visualization of trajectories by the different settings in the "path" section. For a detailed description of the options, see [](/user_interface/visualization.md).
3. There are different ways for correcting trajectories, [see here](/correction/correction.md) for a detailed description.
4. To save your progress during correction, export the trajectories as `.trc` file (the internal file format of PeTrack). Once you are finished with the correction, additionally export a [`.txt` file](/export/trajectory.md). 
