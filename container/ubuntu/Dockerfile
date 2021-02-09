FROM ubuntu:20.10

# Update the repositories
RUN apt-get update && apt-get install -y software-properties-common

# Install the needed tools
RUN apt-get update && apt-get install -y wget \
                       git \
                       g++ \
                       make \
                       ninja-build \
                       cmake

# Install the needed libraries
RUN apt-get update && apt-get install -y \
	qt5-default \
	libqwt-qt5-dev \
	libopencv-dev \
	libopencv-contrib4.2 \
	catch2

# Install python
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip
RUN pip install pytest

# Install trompeloeil from source (not in package manager)
RUN mkdir deps && \
    cd deps && \
    git clone https://github.com/rollbear/trompeloeil.git &&\
    cd trompeloeil &&\
    git fetch --all --tags &&\
    git checkout tags/v38 &&\
    mkdir build &&\
    cd build && \
    cmake .. && \
    cmake --build . --target install
