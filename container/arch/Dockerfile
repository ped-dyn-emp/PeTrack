from archlinux

# Update the repositories
RUN pacman -Syy

RUN pacman -S --needed --noconfirm \
	git \
	gcc \
	clang \
	cmake \
	make \
	ninja \
	base-devel \
	glew

RUN pacman -S --needed --noconfirm \
	qt5-base \
	opencv hdf5 vtk\
	qwt \
	catch2

