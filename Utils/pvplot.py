import matplotlib
import matplotlib.pyplot as plt

from astropy import wcs, units as u
from spectral_cube import SpectralCube
from pvextractor import extract_pv_slice, Path

import pathlib
import sys

matplotlib.use('Agg')
plt.rcParams['figure.facecolor'] = 'w'
plt.rcParams['image.cmap'] = 'gray'


def extract_pv_plot(fin, slice, line, outdir):
    """Extract Position-Velocity Slice from a cube

    Parameters
    ----------
    fin : str
        Input filepath
    slice: int
        Slice number
    line : list
        List of tuples
    outdir : str
        Output folder

    Returns
    -------
    str
        Path to generated FITS file
    """

    try:
        name = pathlib.Path(fin).stem
        cube = SpectralCube.read(fin)
        path = Path(line)

        # Path on slice
        ax = plt.subplot(111, projection=cube.wcs.celestial)
        ax.imshow(cube[slice].value)
        path.show_on_axis(ax, spacing=1, color='r')
        ax.set_xlabel(cube.wcs.wcs.lngtyp)
        ax.set_ylabel(cube.wcs.wcs.lattyp)
        plt.savefig(pathlib.Path(outdir) / (f'path_{name}.png'))
        plt.clf()

        # PV as FITS
        pvdiagram = extract_pv_slice(cube=cube, path=path, spacing=1)
        fout = pathlib.Path(outdir) / (f'pv_{name}.fits')
        pvdiagram.writeto(fout, overwrite=True)

        # PV as PNG
        ax = plt.subplot(111, projection=wcs.WCS(pvdiagram.header))
        im = ax.imshow(pvdiagram.data)
        cb = plt.colorbar(mappable=im)
        cb.set_label(cube.unit)
        ax.coords[0].set_format_unit(u.arcmin)
        ax.set_xlabel('Offset [arcmin]')
        ax.coords[1].set_format_unit(u.m/u.s)
        ax.set_ylabel('Velocity [m/s]')
        plt.savefig(pathlib.Path(outdir) / (f'pv_{name}.png'))
        print(fout.absolute().as_posix())
    except Exception as ex:
        print(str(ex), end='')
        return 1

    return 0


if __name__ == "__main__":
    if len(sys.argv) < 8:
        print(f'Usage: {sys.argv[0]} <input> <slice> <x1> <y1> <x2> <y2> <output_dir>')
        sys.exit(1)
    fin, nslice, x1, y1, x2, y2, output_dir = sys.argv[1:8]
    sys.exit(extract_pv_plot(fin, int(nslice), [(float(x1), float(y1)), (float(x2), float(y2))], output_dir))