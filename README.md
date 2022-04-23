# gummy

CLI screen manager for X11 that allows automatic and manual brightness/temperature adjustments, via backlight (currently only for embedded displays) and gamma. Multiple monitors are supported.

Automatic brightness is based on screen contents or ambient light sensor data.

## Installation

DEB packages:
- provided in each new [release.](https://github.com/Fushko/gummy/releases)

AUR packages:
- [stable](https://aur.archlinux.org/packages/gummy/)
- [bleeding edge](https://aur.archlinux.org/packages/gummy-git/)

RPM packages:
- to be added

A reboot may be needed after the installation to ensure udev rules for backlight adjustments are loaded.

## Building from source

Requirements:

- C++17 compiler
- X11
- XCB
- XCB-Randr
- XLib-Shm
- sdbus-c++
- libudev

#### Apt packages

`sudo apt install build-essential cmake libxext-dev libxcb-randr0-dev libsdbus-c++-dev libudev-dev`

### Installation

```
git clone https://github.com/Fushko/gummy.git
cd gummy
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE="Release"
cmake --build .
sudo make install
```

## Usage

Type `gummy -h` to print a help message.

Quick guide:

`gummy start` starts the daemon responsible for screen adjustments.

`gummy -B 1` enables screenshot-based auto brightness, on all screens.

`gummy -B 2` enables ALS-based auto brightness (if available).

`gummy -B 0 -s 1` disables automatic brightness on the second screen.

`gummy -b 60 -s 1` sets the brightness to 60% on the second screen.

`gummy -t 3400` sets the temperature to 3400K on all screens.

`gummy -T 1 -y 06:00 -u 16:30` enables auto temperature on all screens, with the sunrise set to 06:00 and sunset to 16:30.

## Donations

You can buy me a coffee at: https://coindrop.to/fushko

## License

Copyright 2022 Francesco Fusco

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

See [LICENSE](https://github.com/Fushko/gummy/blob/master/LICENSE).

