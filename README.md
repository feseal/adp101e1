# adp101e1ngu
Board [adp101e1ngu](http://insys.ru/products/dsp/adp101e1) communication software

# Installation

## Installing apt dependencies
```bash
$ sudo apt-get install libboost-system-dev libboost-thread-dev libboost-filesystem-dev libboost-program-options-dev
```
## Installing elfio library
```bash
$ wget http://sourceforge.net/projects/elfio/files/ELFIO-sources/ELFIO-2.2/elfio-2.2.tar.gz
$ tar xvzf elfio-2.2.tar.gz
```
## Building and installing
```bash
$ bjam --elfio-path=<elfio-path> --prefix=<prefix>
```

```<elfio-path>``` - path to elfio library

```<prefix>``` - installation prefix

Example:

Suppose we whant install utils to $(HOME)/bin folder, then

```bash
$ sudo apt-get install libboost-system-dev libboost-thread-dev libboost-filesystem-dev libboost-program-options-dev
$ wget http://sourceforge.net/projects/elfio/files/ELFIO-sources/ELFIO-2.2/elfio-2.2.tar.gz
$ tar xvzf elfio-2.2.tar.gz
$ git clone  https://github.com/feseal/adp101e1ngu.git
$ cd adp101e1ngu
$ bjam --elfio-path=../elfio-2.2 --prefix=$HOME
```