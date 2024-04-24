# Camera Selection and Parameter Settings

The cameras with the corresponding lens are the centerpiece for collecting the raw data. The selection of the camera model and lens and the setting of the parameters must therefore be carried out very carefully and, if possible, tested in the planned or similar environment.

The aim of PeTrack is to determine the head trajectory of each person as accurately as possible. The perspective view of a camera can occlude people from each other or from the surrounding structures. Here the vertical view on a crowd of people from above minimizes this **occlusion**. A large focal length or a small **angle of view** further reduce the negative influence of the perspective view. To cover the experimental area, the mounting height must be increased if the angle of view is reduced (or an overlapping camera grid and a subsequent data combination is needed). The greater the difference in persons’ size, the greater the occlusion, so that it may make sense to limit the subject acquisition to a range of person sizes (if this does not bias the study results).

The larger the angle of view, the more important it is to take the **height of the person** into account. The error, which increases towards the border of the image, can be seen in [Figure 1](#persView). If the angle of view is small, it may be sufficient to assume the average person height of the test subjects. Otherwise, the size of the person must be measured, e.g. using stereo cameras or individual coding.  The assignment can be realized by color-coded height classes or by individual codes using questionnaires that include the person's height (see [markers](/recognition/recognition.md)). The person size must be specified relative to the coordinate system of the [extrinsic calibration](/calibration/extrinsic_calibration.md). If the origin is on the ground, the height of the person including shoes must be indicted. The varying head height caused by the bobbing movement of walking as well as non-planar movements, e.g. on stairs, can be realized using stereo cameras.

::::{figure-md} persView

:::{image} ../images/planning_perspectiveViewError.png
:width: 50%
:::

For a difference in size between two persons $d_h$ depending on the angle $\alpha$ to the camera plumb line or optical axis, the error in calculating the position in the movement plane is $e_h = |d_h\tan\alpha|$.
::::

The **image resolution** must be sufficient for the features to be extracted. If only colored caps must be localized, the resolution can be much lower than if codes applied to the caps have to be read (see [markers](/recognition/recognition.md)). For the code marker within a sharp image, at least 3 pixel or better 4 pixel in each direction per marker element or bit are required.

The **frame rate** is sufficient with standard rates of 24&#160;fps to 30&#160;fps, as a maximum of 7&#160;cm is moved between consecutive images when walking and the distance in time can be assumed to be linear. For running people, increasing the frame rate can improve tracking.

The **focus** must be at the subjects' head height and is usually easier to adjust on the floor using an object at the distance of the later heads. A small image sensor in cameras usually results in a large depth of field and makes it easier to find or set the focal point.

The **aperture size** should be set to a low value for a large depth of field. The sharpness is usually lower at the border of the image and must be tested there to see whether the marker elements to be detected can be read out. Both properties or settings that increase sharpness (small image sensor, small aperture size) have the disadvantage that the amount of light collected is reduced as a result.

A fast **shutter speed** or short **exposure time** also makes the image sharper as a whole and reduces motion blur, but also results in a darker image. An exposure time of 1/150&#160;s is sufficient for people walking in crowds and 1/500&#160;s for people running. It is also important to ensure that the shutter speed matches the ambient light conditions (see below).

A low **gain** or a low **ISO value** results in a sharper image, but may have to be increased due to the set aperture size and shutter speed. Increasing the gain increases the image noise. Aperture size, shutter speed and gain all influence the brightness of the image.

The **recording format** usually influences the sharpness of the image due to the compression and should therefore be selected so that the elements to be extracted are clearly recognizable. The size of the storage media must match the planned recording duration for the selected recording format. The video recording should be set to full frame or progressive scan and no interlaced recording for the sequence of sub-fields should be taken. 

All camera parameters must be set **manually** so that they do not change during the experiment and thus possibly change the intrinsic calibration or influence the parameters for marker recognition.

In summary, the following can be summarized in simple terms, even if the recommendations have a contradictory effect on each other:
-	Small angle of view 
-	High mounting height
-	High image resolution
-	High refresh rate
-	Small aperture size
-	Low exposure time
-	Low gain

The **[intrinsic calibration](/calibration/intrinsic_calibration.md)** must be carried out after changing the camera parameters like angle of view, image resolution, aperture site, focus or recording format. Switching on and off the camera, temperature variations or the transport of the camera can also affect the intrinsic calibration. Therefore, the calibration pattern should be recorded on site on the day of the experiments after setting the camera parameters. The calibration recordings are easier to carry out on the ground. The distance of the pattern to the camera must result in a sharp pattern recording but not lead to a change of focus.

The **[extrinsic calibration](/calibration/extrinsic_calibration.md)** must be performed after each change to the camera parameters or the position or orientation of the camera. Therefore, the calibration points should be recorded during the experiments after the final suspension and alignment. Recalibration may be necessary if the camera has been moved, e.g. by changing the storage medium, plugging in or unplugging cables or moving the superordinate suspension.

Ideally, the power supply, camera control and, if necessary, data transmission of the cameras should be carried out via **cable**. Depending on the length of the recording, for powering an internal battery or a connected power bank may also be sufficient. A required wireless control of the camera should be verifiable (status lights or monitor).

When suspending the cameras, it is essential to ensure that they are secured against falling by attaching **safety ropes**, especially as people will often be walking underneath them.

If possible, the recordings should be backed up twice on site after the experiments. For experiments lasting several days, it must be ensured that the **backup** can be completed and carried out overnight for all media or sensors.

The video recordings are often also used for a **qualitative review** and evaluation of the experiments. For this purpose, it can be useful to additionally record other perspectives on the experimental area, e.g. a view covering the whole experiment. Zudem ist eine Kamera mit Audioaufnahme sinnvoll, um Ansagen oder akustische Reaktionen etc. im Nachgang zu erfassen. A camera with audio recording is also useful afterwards, e.g. for capturing announcements or acoustic reactions.

There is a wide range of **camera types** and models. Roughly speaking, they can be divided into the classes industrial cameras and camcorders. Often for industrial cameras the recording is done on computers connected via cable and for camcorders directly at the camera. Industrial cameras have the advantage that they are more flexible, because they are assembled from individual components (camera, lens, cable, storage, computing unit), but this leads on the other hand to the fact that they are more error prone (frame drops, connection interruption). Industrial cameras are easier synchronizable and steerable and also allow a direct further processing of the data and an automation. The storage size is more limited for camcorders, but the handling is much easier and for the same video quality camcorders are cheaper. For not too complex scenarios we suggest using camcorders because of an easier handling and less technical challenges and problems.