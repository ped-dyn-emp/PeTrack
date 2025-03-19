# Testing
In PeTrack we use unit tests and regression tests to ensure PeTrack runs as intended.

## Unit tests
Unit tests are used to test single functions and classes without having to go through the whole workflow
of setting up a project.
As a unit testing framework we use [Catch2](https://github.com/catchorg/Catch2). It allows to easily construct
test cases and uses a simple structure based on plain english to describe the structure of the specific test. <br>
To run the tests execute the follwing command from the PeTrack directory:
```
[/home/dev/petrack/build-debug] $ ./tests/unit_test/petrack_tests
```
:::{note}
This may take a few seconds to finish. <br>
If this doesn't work, it is probably because you didn't compile the unit tests.
Make sure you compile PeTrack with the `-DBUILD_UNIT_TESTS=ON` option.
:::

## Regression tests
:::{important}
Make sure you cloned the required files via `git lfs pull`.
:::
Regression tests are used to test the whole PeTrack application by loading certain project files and
expecting the correct output files. <br>
We use the [pytest](https://pytest.org) framework in Python to implement our regression tests. <br>
To run all regression tests, you need to switch to the regression test directory and call python to run the tests.
Make sure you use the right build directory for the `--path` option
Be aware that these tests may run up to half an hour depending on the machine!
```
[/home/dev/petrack/build-debug] $ cd ../tests/regression_test/tests
[/home/dev/petrack/tests/regression_test/tests] $ python -m pytest --path=../../../build-debug/petrack
```
:::{note}
If you only want to one a specific test you can add the filename of that test to the previous command:
```
[/home/dev/petrack/tests/regression_test/tests] $ python -m pytest test.py --path=../../../build-debug/petrack
```
:::