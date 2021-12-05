# Requirements

## Mac OS (Homebrew Required)

Run the commands below to install the packages needed to build and flash your device

```shell
# Uninstall existing packages to ensure avr-gcc is properly installed
brew uninstall avr-gcc avrdude

# Now install requirements
brew tap osx-cross/avr
brew install avr-gcc --build-from-source avr-gcc
brew install avrdude make
```

## Linux

First, install your distros "Development Tools" group of packages. This will include basic tools such as gcc and make.

For Fedora, the command is shown below:

```shell
dnf groupinstall 'Development Tools'
```

install the packages listed below. Note that the package names will likely differ between distros.

```shell
avr-libc
avr-binutils 
gcc-avr32-linux-gnu
gcc-c++-avr32-linux-gn
```