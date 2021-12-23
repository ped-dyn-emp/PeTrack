FROM ubuntu:20.04

# Update the repositories
RUN apt-get update && apt-get install -y software-properties-common wget

# Install the needed tools
RUN echo "deb [trusted=yes] http://apt.llvm.org/focal/ llvm-toolchain-focal-13 main" | tee /etc/apt/sources.list.d/clang.list

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
        clang-13 \
        clang-format-13

# Set clang-13 as default compiler
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-13 100 &&\
    update-alternatives --install /usr/bin/cc cc /usr/bin/clang-13 100

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
	libopencv-contrib4.2

# Install catch2 from source (not in package manager)
RUN mkdir deps && \
    cd deps && \
    git clone https://github.com/catchorg/Catch2.git &&\
    cd Catch2 && \
    git fetch --all --tags &&\
    git checkout tags/v2.13.7 &&\
    mkdir build &&\
    cd build &&\
    cmake .. -DCATCH_BUILD_TESTING=OFF -DCATCH_BUILD_EXAMPLES=OFF -DCATCH_BUILD_EXTRA_TESTS=OFF && \
    cmake --build . --target install &&\
    cd ../..

# Install trompeloeil from source (not in package manager)
RUN git clone https://github.com/rollbear/trompeloeil.git &&\
    cd trompeloeil &&\
    git fetch --all --tags &&\
    git checkout tags/v38 &&\
    mkdir build &&\
    cd build && \
    cmake .. && \
    cmake --build . --target install


