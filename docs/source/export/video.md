# Video View Import and Export

## Import

PeTrack internally uses [ffmpeg](https://ffmpeg.org/) for reading video files. This should support virtually any video file format of practical relevance. PeTrack also supports reading in image series. In this case, the images need to be named with the same stem, followed by an increasing number. In the simplest case, the images are named by numbers only, e.g.

```
000100000.jpg
000100001.jpg
000100002.jpg
000100003.jpg
000100004.jpg
```

## Export

PeTrack allows you to export the video. All of the options related to video export are in the `File` menu from the menu bar.

Export Video
: Export video with undistortion and border, but without any overlays

Export Image
: Export currently shown frame with undistortion and border, but without any overlays

Export Image Sequence
: Export as image sequence with undistortion and border, but without any overlays. Can be used to get an image sequence out of a video.

Export View Video
: Export as video as is shown in PeTrack. That is undistortion is applied and all visualizations (e.g. trajectories) are part of the exported video

Export View
: Export currently shown frame with undistortion and overlays

Export View Sequence
: Export as image sequence with undistortion and overlays.
