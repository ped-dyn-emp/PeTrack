FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

# Update the repositories
RUN apt-get update && apt-get -y install tzdata

# Install the needed tools
RUN echo "deb [trusted=yes] http://apt.llvm.org/jammy/ llvm-toolchain-jammy-14 main" | tee /etc/apt/sources.list.d/clang.list

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    tzdata \
    software-properties-common \
    build-essential \
    wget \
    gcc \
    g++ \
    gdb \
    clang \
    clang-14 \
    clang-format-14 \
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
    ffmpeg \
    mesa-common-dev \
    libglu1-mesa-dev \
    libavcodec-dev libavformat-dev libswscale-dev \
    libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev \
    libpng-dev libjpeg-dev libopenexr-dev libtiff-dev libwebp-dev \
    python3 \
    python3-pip \
    python3-dev \
    unzip \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set clang-14 as default compiler
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-14 100 &&\
    update-alternatives --install /usr/bin/cc cc /usr/bin/clang-14 100

# Install OpenCV 4.10
ARG OPENCV_VERSION=4.10.0 
RUN cd /opt && \
    wget https://github.com/opencv/opencv/archive/${OPENCV_VERSION}.zip && \
    unzip ${OPENCV_VERSION}.zip && \
    rm ${OPENCV_VERSION}.zip && \
    cd opencv-${OPENCV_VERSION} && \
    mkdir build && \
    cd build && \
    cmake -G Ninja -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D BUILD_EXAMPLES=OFF \
    -D BUILD_TESTS=OFF \
    -D BUILD_PERF_TESTS=OFF \
    -D BUILD_opencv_core=ON \
    -D BUILD_opencv_calib3d=ON \
    -D BUILD_opencv_video=ON \
    -D BUILD_opencv_videoio=ON \
    -D BUILD_opencv_highgui=ON \
    -D BUILD_opencv_imgproc=ON \
    -D BUILD_opencv_objdetect=ON \
    -D WITH_FFMPEG=ON \
    .. && \
    cmake --build . --config Release --parallel $(nproc) && \
    cmake --install . && \
    ldconfig && \
    rm -rf /opt/opencv-${OPENCV_VERSION}

# Install Qt 6.8.3
RUN pip install pytest numpy aqtinstall && aqt install-qt linux desktop 6.8.3 -O /opt/qt6 && \
    rm -rf /root/.cache/pip
ENV PATH="/opt/qt6/6.8.3/gcc_64/bin:${PATH}"

# Install Qwt
ARG QWT_VERSION=6.3.0
RUN cd /opt && \
    wget https://sourceforge.net/projects/qwt/files/qwt/${QWT_VERSION}/qwt-${QWT_VERSION}.zip && \
    unzip qwt-${QWT_VERSION}.zip && \
    rm qwt-${QWT_VERSION}.zip && \
    cd qwt-${QWT_VERSION} && \
    sed -i "s+QWT_INSTALL_PREFIX[[:space:]]*=.*$+QWT_INSTALL_PREFIX=/usr+g" qwtconfig.pri && \
    qmake qwt.pro && \
    make -j$(nproc) && \
    make install && \
    cd / && rm -rf /opt/qwt-${QWT_VERSION}


# create the ci user
RUN ["useradd", "ci"]
USER "ci"
