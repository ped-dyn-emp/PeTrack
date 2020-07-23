import pytest
import subprocess


def pytest_addoption(parser):
    parser.addoption("--path", action="store", default="../../../build/petrack.exe")




# "codeMarker"
@pytest.fixture(params=["markerCasern", "multicolor", "markerJapan", "multiColorMarkerWithAruco"], scope='session')
def petrack_on_testdata(request, pytestconfig):
    petrack_path = pytestconfig.getoption("path")

    """Does execute PeTrack on the test data to generate the trc and txt Files"""
    # TODO an letzendliche Projektstruktur anpassen
    test_path = "../data/" + request.param + "_test"
    truth_path = "../data/" + request.param + "_truth"
    project = "../data/" + request.param + ".pet"
    output = test_path
    subprocess.run([petrack_path, "-project", project, "-autotrack", output, "-platform", "offscreen"], check=True, stdout=subprocess.DEVNULL)

    yield test_path, truth_path


