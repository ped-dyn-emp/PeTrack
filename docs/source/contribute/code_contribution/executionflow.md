# PeTrack's execution flow 
This site contains a rough sketch of the program flow of PeTrack.
This may help understanding the architecture a little bit better when 
figuring out how to develop new features.

The "main-loop" of PeTrack is the player. In the Player::playVideo method, we move form image to image - Either frames of a video or literal images of a sequence - process it and then display it. New images are retrieved here and then forwarded to methods for processing and displaying.

The heavy lifting of processing is done in the Petrack::updateImage method. First we apply different filters to the image.

* Swapping - mSwapFilter
    * Swapping the image, if demanded by the user
* Brightness & Contrast - mBrightContrastFilter
    * Improving brightness and contrast of the image
* Border
    * Adding a border to the image
    * **Important**: Changes pixel coordinates
    * border-width is user-given
    * must be activated by the user
* Undistortion - mCalibFilter
    * Undistorts the image
    * **If distortion parameters change or the calib filter is toggled on/off, all trajectories get deleted**
    * must be activated by the user
* Background Substraction - mBackgroundFilter
    * Performs background substraction
    * must be activated by the user - mostly off
    * was used in stereo-videos


After these filters were applied, tracking is executed. We execute tracking first, so new points from the recognition step can be evaluated immediately. Tracking happens in the `Tracker::track` method.

After tracking recognition is performed. The method of recognition is decided by the user. These new points then are added to existing trajectories or start a new one. See [How does a TrackPoint get added](tracking.md) for more details.

After all of this, the filtered Image is copied to the ImageItem's QImage and the GraphicsView orchestrating the GUI gets updated/redrawn.

