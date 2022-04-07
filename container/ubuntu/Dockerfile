FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

# Update the repositories
RUN apt-get update && apt-get install -y software-properties-common wget

# Install the needed tools
RUN echo "deb [trusted=yes] http://apt.llvm.org/jammy/ llvm-toolchain-jammy-14 main" | tee /etc/apt/sources.list.d/clang.list

RUN apt-get update && \
    apt-get install -y \
        wget \
        git \
        make \
        ninja-build \
        cmake \
        parallel \
        mesa-common-dev \
        libglu1-mesa-dev \
        clang-14 \
        clang-format-14

# Set clang-14 as default compiler
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-14 100 &&\
    update-alternatives --install /usr/bin/cc cc /usr/bin/clang-14 100

# Install python
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip

RUN pip install pytest aqtinstall

# Install Qt 5.15
RUN aqt install-qt linux desktop 5.15.2
ENV PATH="/5.15.2/gcc_64/lib/cmake/Qt5:${PATH}"

# Install the needed libraries
RUN apt-get update && apt-get install -y \
	libqwt-qt5-dev \
	libopencv-dev \
	libopencv-contrib4.5 \
    catch2

# Install trompeloeil from source (not in package manager)
RUN git clone https://github.com/rollbear/trompeloeil.git &&\
    cd trompeloeil &&\
    git fetch --all --tags &&\
    git checkout tags/v38 &&\
    mkdir build &&\
    cd build && \
    cmake .. && \
    cmake --build . --target install


