# 1.2

- Feature: Add the option to show specific markerIDs
- Not exporting colorpoint for an invalid color anymore due to internal change of trackpoints
- Dropping down elements now also works over the view area
- Fix: roi not shown on startup
- Fix: wrong margin for some UI boxes
- Fix: code marker ID 0 was not displayed
- Fix: border size slider freezed PeTrack
- Fix: zoom buttons not working

# 1.1

- Export trajectories and metadata to hdf5. For file format infos see [here](https://ped.fz-juelich.de/dataarchive/doku.php?id=info)
- Fix crash on windows on loading a machine learning model
- Update to Qt6
- Fix arrow of spinbox `show only` not updating after importing trajectories
- Creating manual trackpoints or calibration points is now more intuitive
- Regions of interest are now visible per default
- Minor improvements to machine learning marker
- `show` set to false implies `fix` is true now
- Better warnings on incorrect 3dc files
- Video export now exports the undistorted video
- Add shortctut for jumping to start of trajectory
- Text is not automatically selected anymore on input dialogs
- Numbering for pedestrians on txt import is now always the same

# 1.0

- Downloads are now available on GitLab and GitHub without prior registration

# v0.10

## 0.10.7 + 0.10.8

For complete comparison see [here](https://github.com/ped-dyn-emp/PeTrack/compare/5b4cc919...afb8a9f)

**Highlights**

- Intrinsic calibration from video
- Jump 3sec in video with `j` and `l` shortcut
  - Nice for annotation in social psychology context
- Support trailing whitespace in 3dc file
- fix scrubbing sometimes not showing the correct image

## 0.10.6

For complete comparison see [here](https://github.com/ped-dyn-emp/PeTrack/compare/54df2ac1...5b4cc919)

**Highlights**

- New recognition method: Machine Learning Marker (YOLOv5 oder YOLOv8 model)
- Fix that region of interest was movable despite being fixed in GUI

## 0.10.5

For complete comparison see [here](https://github.com/ped-dyn-emp/PeTrack/compare/e16be41b...54df2ac1)

**Highlights**

- Added new button `calc` to extrinsic calibration
  - Calibration only performed on click on that button; not as side-effect of load or fetch anymore
- `goto, start, end` is niw `start, mid, end`
- Added feature to annotate groups, see https://petrack.readthedocs.io/en/latest/annotationgroups/annotation_grouping.html
- Removed (dysfunctional) recording buttion; please use the export options under the `File` menu

## 0.10.4

For complete comparison see [here](https://github.com/ped-dyn-emp/PeTrack/compare/6dd0eadf...e16be41b)

**Highlights**

- Option to show complete trajectory of selected person
- Change shortcuts
- Adapt default export settings
- Throws errors on reading ill-formed .pet files
- Update to OpenCV 4.10

## 0.10.3

For complete comparison see [here](https://github.com/ped-dyn-emp/PeTrack/compare/4dca7dbc...6dd0eadf)

**Highlights**

- Updated Layout of keybinding window
- Start Sphinx-Documentation inside of repository, accessible on <https://go.fzj.de/petrack-docs>

## 0.10.2

For complete comparison see [here](https://github.com/ped-dyn-emp/PeTrack/compare/8f17e1af...4dca7dbc)

**Highlights**

- Fix opening project with space in path via double click on Windows
- Settings calibration settings like `fix center` now only apply after re-calibration
- Add "Sequence FPS" used for export of txt-files, independent of playback FPS

## 0.10.1

For complete comparison see [here](https://github.com/ped-dyn-emp/PeTrack/compare/e8b5afad...8f17e1af)

**Highlights**

- Added undo and redo via Ctrl+z and Ctrl+shift+z (or via the `Edit` menu) for manual changes to trajectories
- `Split trajectory` now possible from within the context menue (previously only via shortcut)
- Misc. bug fixes concerning intrinsic calibration

## 0.10.0

For complete comparison see [here](https://github.com/ped-dyn-emp/PeTrack/compare/ac443ac3...e8b5afad)

**Highlights**

- Add new tab `correction` for quick sanity checks on the trajectories
- `immutable` checkbox in the calibration tab
  - can be used to prevent accidental changes to the calibration values
- Fixed freezing of GUI on Mac

# v0.9

## v0.9.4

For complete comparison see [here](https://github.com/ped-dyn-emp/PeTrack/compare/3c490af3...ac443ac3)

**Highlights**

- Non-readable code marker is used for position in combined multicolor- and code marker.
- Enable manual transformation (i.e. translation, rotation) of MoCap data via PeTrack

## v0.9.3

For complete comparison see [here](https://github.com/ped-dyn-emp/PeTrack/compare/42a8a21b...3c490af3)

**Highlights**

- Fixed case sensitivity on file endings
- Added window for viewing logging output
- Make parameters of autosave (i.e. number of changes before next save) customizable by the user
- Stop tracking before reaching image border
- Fix various crashes
- Missing frames computation is cached in .pet-file

## v0.9.2

For complete comparison see [here](https://github.com/ped-dyn-emp/PeTrack/compare/aa0534e6...42a8a21b)

**Highlights**

- Export mp4 videos
- Added option to export "direction of view" from code markers
- Add Ctrl+scroll to change the id of currently selected pedestrian
- Make spinboxes only scrollable if they are selected to prevent accidental changes to setting, e.g. calibration parameters
- Change Ctrl+E to shortcut for export of trajectories

## v0.9.1

For complete comparison see [here](https://github.com/ped-dyn-emp/PeTrack/compare/582933fe...aa0534e6)

**Highlights**

- Correct order of widget traversal when using the tab-key to navigate the GUI
- Add shortcut to drag and drop a point of trajectory that is visible (not necessarily in the same frame, i.e. might be a future or past point)
- Add Autosave
- Update to OpenCV 4.5
- Remote analysis tab
- Add shortcut to move to the frame of a point of a trajectory by clicking on it
- Manually setting marker ID via context menu
- Update missing frame computation
- Improve MoCap visualization
- Add more intrinsic parameters

## v0.9.0

For complete comparison see [here](https://github.com/ped-dyn-emp/PeTrack/compare/4979a664...582933fe)

**Highlights**

- Export view via CLI
- Various improvements to code marker
- Various improvements to handling of MoCap files

# v0.8

**Highlights**

- Slanted view for external calibration
- Coordinate system and grid is possible
- Introduced multicolor marker with black dot
- Video support for various codecs
- Visualization of motion capturing files
- Improvements to re-tracking
- Implement code marker
- Implement combined code and multicolor marker
- Fix playback speed of vide
- Loop video
- Create installers for Windows in CI via NSIS
- Switch to CMake -> Enable Mac and Linux support (self-compiled)
- Update OpenCV from 3 to 4
- Store error of extrinsic calibration in .pet file
- Improvements to GUI layout, especially for high DPI displays
- Add possibility to set markerIDs via extra file
- Add possibility to set height via markerID
- Fix various crashes
- Add colorpicker
- Lots and lots of changes on development (e.g. switch to git; using of autoformatters, enabling and checking of warnings in a CI pipeline, addition of regression tests...)
- Some performance improvements

# v0.7

introduce stereo cameras; alternate height  
background subtraction especially for stereo cameras; recognition of unmarked pedestrians; integration of 4 disparity algorithms  
filter to swap picture; more video codecs; color blobs as marker using HSV triangle  
marker with two dots added; export of viewing direction

# v0.6

bug fixes; improve speed; expand visualization possibilities; add variable tracking region adapted to head size; provide program combine to merge trajectories

# v0.5

improve robustness of tracking with interpolation and extrapolation; first analysis functionality

# v0.4

tracking import and export; manual adding and deleting of trackpoints

# v0.3

recognition; tracking; color mapping

# v0.2

add first recognition

# v0.1

only camera calibration is supported; import and export of video and image sequence
