FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

# Update the repositories
RUN apt-get update && apt-get -y install tzdata

# Install the needed tools
RUN echo "deb [trusted=yes] http://apt.llvm.org/jammy/ llvm-toolchain-jammy-14 main" | tee /etc/apt/sources.list.d/clang.list

RUN apt-get update && \
    apt-get install -y \
        software-properties-common \
        build-essential \
        wget \
        gcc \
        g++ \
        gdb \
        clang \
        make \
        ninja-build \
        cmake \
        autoconf \
        automake \
        libtool \
        valgrind \
        parallel \
        rsync \
        tar \
        mesa-common-dev \
        libglu1-mesa-dev \
        clang-14 \
        clang-format-14 \
    && apt-get clean


# Set clang-14 as default compiler
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-14 100 &&\
    update-alternatives --install /usr/bin/cc cc /usr/bin/clang-14 100

# Install python
RUN apt-get update && apt-get install -y \
        python3 \
        python3-pip \
        python3-dev \
    && apt-get clean

RUN pip install pytest aqtinstall

# Install Qt 5.15
RUN aqt install-qt linux desktop 5.15.2
ENV PATH="/5.15.2/gcc_64/lib/cmake/Qt5:${PATH}"

# Install the needed libraries
RUN apt-get update && apt-get install -y \
	libqwt-qt5-dev \
	libopencv-dev \
	libopencv-contrib4.5
