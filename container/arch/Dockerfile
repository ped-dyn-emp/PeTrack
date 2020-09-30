FROM archlinux:20200705

# Update the repositories
RUN pacman -Syy

# Install the needed tools
RUN pacman -Syu --needed --noconfirm \
	git \
	gcc \
	cmake \
	make \
	base-devel \
	glew

# Install the needed libraries
RUN pacman -Syu --needed --noconfirm \
	qt5-base \
	qwt \
	catch2

# Install python
RUN pacman -Syu --needed --noconfirm \
    python \
    python-pip
RUN pip install pytest

# Install opencv 4.2 (last command to avoid any updates by pacman)
RUN pacman --needed --noconfirm -U \
    https://archive.archlinux.org/packages/h/hdf5/hdf5-1.10.5-1-x86_64.pkg.tar.xz \
    https://archive.archlinux.org/packages/v/vtk/vtk-8.2.0-9-x86_64.pkg.tar.xz \
    https://archive.archlinux.org/packages/g/glew/glew-2.1.0-1-x86_64.pkg.tar.xz \
    https://archive.archlinux.org/packages/o/openexr/openexr-2.4.0-6-x86_64.pkg.tar.xz \
    https://archive.archlinux.org/packages/o/opencv/opencv-4.2.0-1-x86_64.pkg.tar.xz

