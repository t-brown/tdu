# tdu
## Tree Disk Usage


`tdu` is a command line tool to obtain a tree like disk usage.


## Prerequisites

A C compiler, for example [gcc](http://gcc.gnu.org/).

## Building / Installation

`tdu` relies on the GNU build system [autoconf](http://www.gnu.org/software/autoconf/)
and [automake](http://www.gnu.org/software/automake/).

To install `tdu` with the default options:

    ./configure
    make
    make install

This will install the binrary tool in `/usr/local/bin` and man pages in
`/usr/local/man`. To specify a different installation prefix, use the
`--prefix` option to configure:

    ./configure --prefix=/opt
    make
    make install

Will install mcds in `/opt/{bin,man}`.

## Usage

The utility `tdu` walks a directory tree to obtain the total disk usage
and the disk usage that has not been accessed within a certain number
specified days (the default is 45).
