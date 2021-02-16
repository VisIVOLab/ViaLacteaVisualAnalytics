# Dependencies installation
Do not forget to set `INCLUDEPATH` and `LIBS` variables in VisIVODesktop-vtk6_qt5.pro.

## Qt 5.14.2 - QtCreator 4.14.0
1. `$ sudo add-apt-repository ppa:beineri/opt-qt-5.14.2-xenial`
1. `$ sudo apt update && sudo apt install -y qt514-meta-full mesa-common-dev libglu1-mesa-dev`
1. `$ echo ". /opt/qt514/bin/qt514-env.sh" >> ~/.bashrc`
1. Install QtCreator using the installer, download it from [here](https://download.qt.io/official_releases/qtcreator/4.14/4.14.0/qt-creator-opensource-linux-x86_64-4.14.0.run).

## CMake 3.19.4

1. `$ mkdir -p $HOME/software && cd $HOME/software`
1. `$ wget https://github.com/Kitware/CMake/releases/download/v3.19.4/cmake-3.19.4-Linux-x86_64.tar.gz`
1. `$ tar xzf cmake-3.19.4-Linux-x86_64.tar.gz`
1. Add `$HOME/software/cmake-3.19.4-Linux-x86_64/bin` to `PATH` env variable


## VTK 9.0.1
1. `$ mkdir -p $HOME/src && cd $HOME/src`
1. `$ wget https://www.vtk.org/files/release/9.0/VTK-9.0.1.tar.gz`
1. `$ tar xzf VTK-9.0.1.tar.gz`
1. `$ mkdir -p $HOME/software/VTK-9.0.1 && cd $HOME/software/VTK-9.0.1`
1. Configure VTK with the following variables set (change Qt path if needed):

     `$ ccmake -DVTK_QT_VERSION:STRING=5 -DVTK_Group_Qt:BOOL=ON -DVTK_GROUP_ENABLE_Qt:STRING=YES -DQT_QMAKE_EXECUTABLE:PATH=/opt/qt514/bin/qmake -DCMAKE_PREFIX_PATH:PATH=/opt/qt514/lib/cmake -DBUILD_SHARED_LIBS:BOOL=ON $HOME/src/VTK-9.0.1`
1. `$ make -j4`
1. `$ sudo make install`

## cfitsio 3.49
1. `$ cd $HOME/src`
1. `$ wget http://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/cfitsio-3.49.tar.gz`
1. `$ tar xzf cfitsio-3.49.tar.gz`
1. `$ cd cfitsio-3.49`
1. `$ mkdir -p $HOME/software/cfitsio-3.49`
1. `$ ./configure --prefix=$HOME/software/cfitsio-3.49`
1. `$ make -j4`
1. `$ make install`
1. Add `$HOME/software/cfitsio-3.49/lib` to `LD_LIBRARY_PATH` env variable

## Boost 1.75.0
1. `$ cd $HOME/src`
1. `$ wget https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz`
1. `$ tar xzf boost_1_75_0.tar.gz`
1. `$ cd boost_1_75_0`
1. `$ mkdir -p $HOME/software/boost-1.75.0`
1. `$ ./bootstrap.sh --prefix=$HOME/software/boost-1.75.0`
1. `$ ./b2 install`

