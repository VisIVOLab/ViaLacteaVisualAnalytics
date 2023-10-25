# Dependencies installation on Unix
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

# Dependencies installation on Windows
## Prerequisites
The following must be installed on your system before you can build and install the next dependencies:

### Windows Media Feature Pack
If you’re running an N version of Windows, which doesn’t include the media package by default, you might need to manually install it. 

Follow this steps:
1. Open **Settings**.
2. Click on **Apps**.
3. Under **Apps & features**, click on **Optional features**.
4. Click on **Add a feature**.
5. Scroll down and find **Media Feature Pack**. (65mb)
6. Click on it and then click on Install.
7. Restart your computer 

### Python 3: latest release (>= 3.11)
| “Add Python x.x to PATH” \
| Add python to environment variables 

Python3 is required for certain functionalities with specific Python packages. Install the following packages:
* `jmespath`
* `pyvo`
* `numpy`
* `scipy`

### Visual Studio Community: (>= 17 2022)
It is required for the installation of C/C++ compiler on Windows and for further dependencies

### CMake
| "Add CMake to the system PATH for all users"

## Qt 5.15.2: Open Source Development
Download and run: [online installer](https://d13lb3tujbc8s0.cloudfront.net/onlineinstallers/qt-unified-windows-x64-4.6.1-online.exe) \
| Create an account. \
| "I am an individual person not using Qt for any company" \
| Custom installation. \
| Additional modules: MSVC 2019, Qt Creator, Qt WebEngine and Qt Network Authorization. \
| Package categories select: LTS and Latest release.

Once installed, set Qt in your path environment variables. \
On System variables create a new variable. \
**Variable name**: `QTDIR` \
**Variable value**: `...\Qt\5.12.2\msvc2019_64` (the compiler folder) \
On System variables edit `Path` variable and add new `%QTDIR%\lib` and `%QTDIR%\bin` 

## CFITSIO 4.1.0
The compression library "zlib" is required in order to build CFITSIO. Visit the [site](https://zlib.net) to download the latest distribution. Unpack it, then build and install it from a parallel directory, for example:

```bash
mkdir zlib.build
cd zlib.build
cmake ..\zlib-1.3 -DCMAKE_INSTALL_PREFIX="C:\Users\myname\zlib"
```
Select your version: Building for Visual Studio 17 2022
```bash
cmake --build . --config Release
cmake --install .
```
Once installed, set zlib in your path environment variables. \
On System variables create a new variable. \
**Variable name**: `zlib` \
**Variable value**: `C:\Users\myname\zlib` \
On System variables edit `Path` variable and add new `%zlib%\bin` and `%zlib%\lib`

The cmake comands below will use the path "C:\Users\myname\zlib" as an example for the installed zlib location.

Unzip the CFITSIO .zip
[Download CFITSIO 4.1.0](https://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/cfit-4.1.0.zip)
You can see your list of all the available CMake Generators by executing the command
```bash
cmake /?
```
In the same directory of cfitsio-4.1.0 folder:
```bash
mkdir cfitsio.build
cd cfitsio.build
cmake -G "Visual Studio 17 2022" -A x64 ..\cfitsio-4.1.0 -DCMAKE_INSTALL_PREFIX=C:\cfitsio -DCMAKE_PREFIX_PATH="C:\Users\myname\zlib"
cmake --build . --config Release
cmake --install .
```

Comment line 223 in `cfitsio\include\fitsio.h` 
```bash
// #define TBYTE 11
```

## VTK 9.1.0
Unzip VTK .zip and create a new folder inside it `/build` \
Open CMake GUI and specify the VTK path in the \
**source code**:`"C:/Users/myname/.../VTK-9.1.0"`\
**binary code**:`"C:/Users/myname/.../VTK-9.1.0/build"` \
and press *Configure*

Select the Visual Studio compiler version and format (x32 or x64) and press *Finish*\
Upon completion of the loading, enable from the list `VTK_GROUP_Qt` `CMAKE_CXX_MP_FLAG`\
You can choose a different `CMAKE_INSTALL_PREFIX` \
Press *Configure*, once finished press *Generate*, and once that’s finished press *Open Project*. It will open Visual Studio. 
To build select from the dropdown menu at the top ‘Debug’ and the solution platform (x32 or x64). Right-click on *ALL_BUILD* and select *Build*. Right-click on *INSTALL* and select *Build*

Once installed, set VTK in your path environment variables. \
On System variables create a new variable. \
**Variable name**: `VTK_DIR` \
**Variable value**: `...\VTK` specify on CMAKE_INSTALL_PREFIX \
On System variables edit `Path` variable and add new `%VTK_DIR%\bin`

## Boost 1.78 C++ Libraries
Unzip Boost

## Building ViaLacteaVisualAnalytics
Build the project using CMake of QT Creator build settings. You may need to specify the following CMake variables:
- `BOOST_ROOT`: Boost installation prefix.
- `CFITSIO_ROOT_DIR`: CFITSIO non-standard installation path.
- `VTK_DIR`: The directory containing a CMake configuration file for VTK.
- `PYTHON_EXECUTABLE`: Path to python3 executable.