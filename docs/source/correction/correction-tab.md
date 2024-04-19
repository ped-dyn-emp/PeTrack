# Correction Tab
The correction tab can help you to find errors that need some manual correction. The tests give you a list of IDs and/or frames in which errors might be present. Clicking on the corresponding line of the output, directs you to the person and frame in which the test found an irregularity. Now you can check whether this is an error, as depending on the type of experiment you performed it doesn't have to be! In case you e.g. decide to remove a trajectory while going the test's output, be aware that the remaining pedestrians with a higher ID number will be renumbered by PeTrack. Four different tests are available:

## Equality
Checks if two trajectories are close to each other. The distance in which these are considered as candidates for equality can be set in `distance [headsize]`. Be aware that this test has the longest running time of all the available tests.  


## Velocity
Checks the velocity (distance between position of two frames): if the velocity is variating fast, the corresponding frames of the person will be listed. Currently the threshold is fixed and cannot be changed.

## Length
Checks the length of the trajectory. All trajectories shorter than the number of frames set in `min.length` will be listed. The value of  `min.length` should be chosen in a way that it is lower than the number of frames a person needs to e.g. pass through the experimental area. 

## Inside
This test checks if the start and end of the trajectory are inside the `recognition region of interest`. 



