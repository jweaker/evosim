# Evosim

A simple evolution simulator

## Youtube video

https://youtu.be/z5KyCMiMJRY

## How to build
If you use Linux, install SFML's dependencies using your system package manager. On Ubuntu and other Debian-based distributions you can use the following commands:

```
sudo apt update
sudo apt install \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libfreetype-dev \
    libflac-dev \
    libvorbis-dev \
    libgl1-mesa-dev \
    libegl1-mesa-dev \
    libfreetype-dev
```

Using CMake from the command line is straightforward as well. Be sure to run these commands in the root directory of the project.

```
cmake -B build
cmake --build build
```
