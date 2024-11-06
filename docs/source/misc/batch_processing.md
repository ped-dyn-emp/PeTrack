# Batch Processing

## Template Projects

You can create _template_-projects for all videos with the same calibration (intrinsic and extrinsic). These are `.pet`-files that contain calibration, recognition as well as tracking settings and can be re-used by changing the video. For this, save your chosen settings in a `template.pet` file and load in different videos via `File > Open Sequence`. Then you can run the tracking with `calculate all`. Save your resulting trajectories with an video-specific name. After this you can continue with the next video via `File > Open Sequence`.

:::{Note}
The results might not be ideal for _all_ videos. Manually check and readjust settings for specific videos if necessary. Remember to save this as new projects as well.
:::

## Command Line Interface


If you have many files to process, it can be more convenient to use the command line interface of PeTrack. After you created the `template.pet`, you can run PeTrack with these settings and different videos using the CLI. For exact usage and all possible options, run `petrack -help`. One example usage could be

```bash
petrack.exe -project template.pet -sequence exp4.mp4 -autoTrack exp4_trajectories -autoSave exp4.pet
```

This command tracks the video `exp4.mp4` using the settings from `template.pet` and saved the results in `exp4_trajectories.trc`. It also creates a new project file `exp4.pet`, so you know what settings were used for export.
