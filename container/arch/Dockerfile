FROM archlinux:20200705

# Update the repositories
RUN pacman -Syy

RUN pacman -Syu --needed --noconfirm \
	git \
	gcc \
	cmake \
	make \
	base-devel \
	glew

RUN pacman -Syu --needed --noconfirm \
	qt5-base \
	opencv hdf5 vtk\
	qwt \
	catch2

RUN pacman -Syu --needed --noconfirm \
    python \
    python-pip
RUN pip install pytest

