# Combining Data from Different Sensors

If the data extracted by PeTrack is intended to be further linked with other
data, e.g. a recognized position of a person with additional personal
information such as age or gender, possibly collected via questionnaires, each
person must be **coded individually** (
see [markers](/recognition/recognition.md)). If other sensors are to be merged,
the **time synchronization** of the data series and, in the case of
position-determining sensors, a **uniform spatial calibration**, e.g. in the
case of overlapping camera views, must be ensured (
see [extrinsic calibration](/calibration/extrinsic_calibration.md)).

The **unique marker** should remain identical throughout the entire series of
experiments in order to analyze individuals across experiments and to simplify
assignment to other sensors applied to this person.

For **time synchronization**, all sensors must be given a uniform time. Ideally,
this should be based on world time, so that it can also be used for the
documentation of the entire experiment. The accuracy to be applied depends on
the data rate of the sensors. For cameras, the aim should be to achieve frame
accurate synchronization. It should be kept in mind that the temporal drift
within the sensors can be very different, so that permanent or regular
synchronization may be necessary. In addition, frame drops can occur (especially
with industrial cameras). For the most accurate synchronization, images from
cameras should be recorded at the same time. This can be realized using time
code generators or communication protocols designed for this purpose. It can be
realized a little less precisely by temporarily recording a clock displaying
world time. For this purpose, video editing programmes can then be used to
record the time at which the seconds display changes.

