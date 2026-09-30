FROM archlinux:latest

RUN pacman -Syu --noconfirm \
    gcc \
    cmake \
    make \
    gdb \
    git \
    pkgconf \
    sdl3 \
    wayland \
    libxkbcommon \
    libdecor

WORKDIR /app