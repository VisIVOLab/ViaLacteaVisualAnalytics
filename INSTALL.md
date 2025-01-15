# Installing dependencies
Prerequisites:
- CMake ($\ge$ 3.16)
- Qt 5.15.2
     - Qt WebEngine
     - Qt Network Authorization

## CFITSIO 4.1.0
1. `$ wget http://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/cfitsio-4.1.0.tar.gz`
1. `$ tar xzf cfitsio-4.1.0.tar.gz`
1. `$ cd cfitsio-4.1.0`
1. `$ ./configure --prefix=/usr/local --enable-reentrant`
1. `$ make`
1. `$ sudo make install`

**Note**: On Apple Silicon Macs use the following configure command instead:

`$ ./configure --prefix=/usr/local --enable-reentrant CC=clang CFLAGS="-arch x86_64"`

## VTK 9.1.0
1. `$ wget https://www.vtk.org/files/release/9.1/VTK-9.1.0.tar.gz`
1. `$ tar xzf VTK-9.1.0.tar.gz`
1. `$ mkdir VTK-9.1.0/build && cd VTK-9.1.0/build`
1. `$ cmake -DVTK_SMP_IMPLEMENTATION_TYPE:STRING=STDThread -DVTK_GROUP_ENABLE_Qt:STRING=YES -DCMAKE_PREFIX_PATH:PATH=<qt-root-path> ..`
1. `$ make`
1. `$ sudo make install`

**Note**: On Apple Silicon Macs add the following argument to cmake:

`-DCMAKE_OSX_ARCHITECTURES=x86_64`

## Python
Python3 is required for certain functionalities using specific Python packages. Install the following packages:
- `jmespath`
- `pyvo`
- `numpy`
- `scipy`
- `pvextractor`
- `spectral-cube`

# Building ViaLacteaVisualAnalytics
Build the project using CMake with your preferred generator. You may need to specify the following CMake variables
- `CFITSIO_ROOT_DIR`: CFITSIO non-standard installation path.
- `VTK_DIR`: The directory containing a CMake configuration file for VTK.

After a successful build, copy the contents of [Utils](Utils) next to the ViaLacteaVisualAnalytics executable.

# Building on Windows

> [!WARNING]
> Experimental.

When building on Windows, there are additional dependencies and some changes to the libraries are necessary to compile a successful build.
## Additional prerequisites
The following must be installed on your system before you can build and install the dependencies:

### Windows Media Feature Pack
If you're running an N version of Windows, which doesn't include the media package by default, you might need to manually install it:

1. Open **Settings**.
1. Click on **Apps**.
1. Under **Apps & features**, click on **Optional features**.
1. Click on **Add a feature**.
1. Scroll down and find **Media Feature Pack**. (65mb)
1. Click on it and then click on Install.
1. Restart your computer 

### Microsoft Visual Studio (VS 2022)
It is required for the installation of C/C++ compiler on Windows and for further dependencies.
Use the Visual Studio Installer and select `Desktop development with C++` to install MS Visual C++ (MSVC).

From now on, use "Visual Studio 17 2022" as generator for CMake.

### OpenSSL 1.1.1
Qt does not provide OpenSSL binaries for Windows.
You can download and install the dll libraries from [here](https://wiki.openssl.org/index.php/Binaries).

## Changes
### CFITSIO 4.1.0
The compression library [zlib](https://zlib.net) is required in order to build CFITSIO on Windows. 
Follow the [README.win](https://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/README.win) from the CFITSIO website.

Due to a `#define` conflict with `winrt.h` that prevents the compilation of ViaLacteaVisualAnalytics, comment out the TBYTE definition in `cfitsio/include/fitsio.h`.

```C
//#define TBYTE 11
```

It does not have any effect on VLVA as it does not use any FITS Tables routines.

### VTK 9.1.0
Same as above.

**Note**: Match VTK's `CMAKE_BUILD_TYPE` to the VLVA's `CMAKE_BUILD_TYPE` you plan to build.
