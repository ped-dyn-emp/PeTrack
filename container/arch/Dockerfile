FROM archlinux

# Update the repositories
RUN pacman -Syy

RUN pacman -S --needed --noconfirm \
	git \
	gcc \
	cmake \
	make \
	base-devel \
	glew

RUN pacman -S --needed --noconfirm \
	qt5-base \
	opencv hdf5 vtk\
	qwt \
	catch2

RUN pacman -S --needed --noconfirm \
    python \
    python-pip
RUN pip install pytest

