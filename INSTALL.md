# Dependencies installation
Prerequisites:
- CMake (>= 3.16).
- Qt 5.15.2 - Additional modules: Qt WebEngine and Qt Network Authorization.

## CFITSIO 4.1.0
1. `$ wget http://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/cfitsio-4.1.0.tar.gz`
1. `$ tar xzf cfitsio-4.1.0.tar.gz`
1. `$ cd cfitsio-4.1.0`
1. `$ ./configure --prefix=/usr/local`
1. `$ make`
1. `$ sudo make install`

#### NOTE: if you're on an Apple Silicon Mac use the following configure command instead:
`$ ./configure --prefix=/usr/local CC=clang CFLAGS="-arch x86_64"`

## VTK 9.1.0
1. `$ wget https://www.vtk.org/files/release/9.1/VTK-9.1.0.tar.gz`
1. `$ tar xzf VTK-9.1.0.tar.gz`
1. `$ mkdir VTK-9.1.0/build && cd VTK-9.1.0/build`
1. Configure VTK (set your Qt path):

     `$ cmake -DVTK_QT_VERSION:STRING=5 -DVTK_Group_Qt:BOOL=ON -DVTK_GROUP_ENABLE_Qt:STRING=YES -DQT_QMAKE_EXECUTABLE:PATH=<path-to-qt-root>/bin/qmake -DCMAKE_PREFIX_PATH:PATH=<path-to-qt-root>/lib/cmake -DBUILD_SHARED_LIBS:BOOL=ON ..`
1. `$ make`
1. `$ sudo make install`

#### NOTE: if you're on an Apple Silicon Mac add the following argument to cmake:
`-DCMAKE_OSX_ARCHITECTURES=x86_64`


## Boost 1.78.0 (Headers only)
1. `$ wget https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.tar.gz`
1. `$ tar xzf boost_1_78_0.tar.gz`
1. `$ cd boost_1_78_0`
1. `$ sudo cp -r boost /usr/local/include`

## Python
Python3 is required for certain functionalities using specific Python packages. Install the following packages:
- `jmespath`
- `pyvo`
- `numpy`
- `scipy` 

# Building ViaLacteaVisualAnalytics
Build the project using CMake with your preferred generator. You may need to specify the following CMake variables:
- `BOOST_ROOT`: Boost installation prefix.
- `CFITSIO_ROOT_DIR`: CFITSIO non-standard installation path.
- `VTK_DIR`: The directory containing a CMake configuration file for VTK.
- `PYTHON_EXECUTABLE`: Path to python3 executable.

After a successful build, copy the contents of [Utils](Utils) next to the ViaLacteaVisualAnalytics executable.