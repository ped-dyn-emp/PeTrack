set -ex

qt_version="6.9.0"
qwt_version="6.3.0"

opencv_version="5.1.0"
ffmpeg_version="8.1.2"

POSITIONAL=()
while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    --install-path)
      install_path="$2"
      shift # past argument
      shift # past value
      ;;
    *)    # unknown option
      POSITIONAL+=("$1") # save it in an array for later
      shift # past argument
      ;;
  esac
done

if [ "${install_path}" ]; then
    if [ ! -d "${install_path}" ]; then
        mkdir -p "${install_path}"
    fi
    install_path="$(cd "${install_path}"; pwd)"
fi

if [[ "$OSTYPE" == "linux-gnu" ]]; then
    CPUS=$(nproc)
elif [[ "$OSTYPE" == "darwin"* ]]; then
    CPUS=$(sysctl -n hw.logicalcpu)
fi

function setup_qt {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd "${temp_folder}"

    # Install Qt via aqt a Python module for installing Qt via CLI
    python3 -m venv qt_venv
    source qt_venv/bin/activate
    python3 -m pip install aqtinstall
    python3 -m aqt install-qt mac desktop ${qt_version} --outputdir "${install_path}"/qt-${qt_version}
    deactivate


    cd "${install_path}"/qt-${qt_version}/${qt_version}
    cd "$(ls)"
    qt_dir=$(pwd)
    export qt_dir

    # To make sure to use the newly installed version of Qt from now on
    export PATH="${qt_dir}/bin:${PATH}"

    cd "${root}"
    rm -rf "${temp_folder}"
}

function setup_qwt {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd "${temp_folder}"

    qmake="${install_path}"/qt-${qt_version}/${qt_version}/macos/bin/qmake
    wget --no-check-certificate https://sourceforge.net/projects/qwt/files/qwt/${qwt_version}/qwt-${qwt_version}.tar.bz2
    tar xf qwt-${qwt_version}.tar.bz2
    cd qwt-${qwt_version}
    # Set the install dir for Qwt correctly
    sed -i '' "s+QWT_INSTALL_PREFIX[[:space:]]*=.*$+QWT_INSTALL_PREFIX=${install_path}\/qwt-${qwt_version}+g" qwtconfig.pri
    sed -i ''  "/QwtDll/d" qwtconfig.pri
    sed -i '' "/QwtDesigner/d" qwtconfig.pri
    sed -i '' "/QwtFramework/d" qwtconfig.pri
    cat qwtconfig.pri
    $qmake qwt.pro
    make install -j "${CPUS}"

    cd "${root}"
    rm -rf "${temp_folder}"
}

function setup_opencv {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd "${temp_folder}"

    wget --no-check-certificate -O opencv_${opencv_version}.tar.gz https://github.com/opencv/opencv/archive/refs/tags/${opencv_version}.tar.gz
    tar xf opencv_${opencv_version}.tar.gz

    wget --no-check-certificate -O opencv_contrib_${opencv_version}.tar.gz https://github.com/opencv/opencv_contrib/archive/refs/tags/${opencv_version}.tar.gz
    tar xf opencv_contrib_${opencv_version}.tar.gz

    cd opencv-${opencv_version}
    mkdir build
    cd build

    cmake -G Ninja -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_PREFIX="${install_path}/opencv-${opencv_version}" \
    -D FFMPEG_INCLUDE_DIRS="${install_path}"/ffmpeg-${ffmpeg_version}/include \
    -D CMAKE_PREFIX_PATH:PATH="${install_path}/ffmpeg-${ffmpeg_version}" \
    -D VIDEOIO_PLUGIN_LIST=ffmpeg \
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
    -D BUILD_opencv_dnn=ON \
    -D WITH_FFMPEG=ON \
    -D BUILD_ZLIB=OFF \
    ..

    cmake --build . --config Release --parallel
    cmake --install .

    cd "${root}"
    rm -rf "${temp_folder}"
}

function setup_ffmpeg {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd "${temp_folder}"

    wget --no-check-certificate https://github.com/FFmpeg/FFmpeg/archive/refs/tags/n${ffmpeg_version}.tar.gz
    tar xf n${ffmpeg_version}.tar.gz
    cd FFmpeg-n${ffmpeg_version}
    ./configure \
      --prefix="${install_path}"/ffmpeg-${ffmpeg_version} \
      --extra-libs="-lpthread -lm" \
      --enable-shared \
      --enable-gpl \
      --enable-gnutls \
      --enable-libass \
      --enable-libfreetype \
      --enable-libopus \
      --enable-libvpx \
      --enable-libx264 \
      --enable-libx265
    make install -j"${CPUS}"
    export PKG_CONFIG_PATH="${install_path}"/ffmpeg-${ffmpeg_version}/lib/pkgconfig/

    cd "${root}"
    rm -rf "${temp_folder}"
}

# was manually installed using online installer
# setup_qt


#setup_qwt

setup_ffmpeg
setup_opencv
