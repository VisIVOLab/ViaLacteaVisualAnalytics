# Dependencies installation
Prerequisites:
- CMake (>= 3.19.4).
- Qt 5.15.2 - Additional modules: Qt WebEngine and Qt Network Authorization.

## libcfitsio 3.49
1. `$ wget http://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/cfitsio-3.49.tar.gz`
1. `$ tar xzf cfitsio-3.49.tar.gz`
1. `$ cd cfitsio-3.49`
1. `$ ./configure --prefix=/usr/local`
1. `$ make`
1. `$ sudo make install`

## VTK 9.0.3
1. `$ wget https://www.vtk.org/files/release/9.0/VTK-9.0.3.tar.gz`
1. `$ tar xzf VTK-9.0.3.tar.gz`
1. `$ mkdir VTK-9.0.3/build && cd VTK-9.0.3/build`
1. Configure VTK (set your Qt path):

     `$ cmake -DVTK_QT_VERSION:STRING=5 -DVTK_Group_Qt:BOOL=ON -DVTK_GROUP_ENABLE_Qt:STRING=YES -DQT_QMAKE_EXECUTABLE:PATH=<path-to-qt-root>/bin/qmake -DCMAKE_PREFIX_PATH:PATH=<path-to-qt-root>/lib/cmake -DBUILD_SHARED_LIBS:BOOL=ON ..`
1. `$ make`
1. `$ sudo make install`

## Boost 1.75.0 (Headers only)
1. `$ wget https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz`
1. `$ tar xzf boost_1_75_0.tar.gz`
1. `$ cd boost_1_75_0`
1. `$ sudo cp -r boost /usr/local/include`

# Building ViaLacteaVisualAnalytics
1. `$ git clone https://github.com/NEANIAS-Space/ViaLacteaVisualAnalytics.git`
1. `$ cd ViaLacteaVisualAnalytics/Code`
1. `$ qmake VisIVODesktop-vtk6_qt5.pro`
1. `$ make`
