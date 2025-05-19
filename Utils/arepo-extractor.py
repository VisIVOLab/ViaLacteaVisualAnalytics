#!/usr/bin/env python

import yt
import numpy as np
from astropy import constants as c
from astropy.io import fits

import argparse
from pathlib import Path


def parse_args():
    parser = argparse.ArgumentParser(description="Process AREPO simulation data.")

    # [--level <level>]
    parser.add_argument(
        "--level",
        type=int,
        default=7,
        help="The resolution of the extracted cube: cells = (2^level)^3 (default: 7)",
    )

    # [--particle <particle>]
    parser.add_argument(
        "--particle",
        type=str,
        default="gas",
        help="Which particle type will be extracted (default: gas)",
    )

    # [--fields <field> [<field ...>]]
    parser.add_argument(
        "--fields",
        nargs="+",
        default=["density"],
        help="Fields to extract (default: density)",
    )

    # [--win_size <x> <y> <z>]
    parser.add_argument(
        "--win_size",
        nargs=3,
        metavar=("SIZE_X", "SIZE_Y", "SIZE_Z"),
        type=float,
        default=[20.0, 20.0, 20.0],
        help="Window size in kpc (default: 20.0)",
    )

    # [--offset <x> <y> <z>]
    parser.add_argument(
        "--offset",
        nargs=3,
        metavar=("OFFSET_X", "OFFSET_Y", "OFFSET_Z"),
        type=float,
        default=[0.0, 0.0, 0.0],
        help="Offset of the center of the Window Size from the center of the simulation (kpc).",
    )

    # <file>
    parser.add_argument("file", help="Input file to process")

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    print(f"\nParsed arguments: {args}")

    # window size and fraction of domain that the window takes
    ds = yt.load(args.file)
    win_kpc = np.array([args.win_size[0], args.win_size[1], args.win_size[2]])
    win_frac = win_kpc / ds.domain_width.in_units("kpc").v

    # left corner of the cutout region, needed by yt as anchor point
    ctr_kpc = ds.domain_center.in_units("kpc").v + args.offset
    left_kpc = ctr_kpc - 0.5 * win_kpc
    right_kpc = ctr_kpc + 0.5 * win_kpc

    # convert back to code units
    pc = c.pc.cgs.value
    kpc = pc * 1e3
    left = left_kpc * kpc / ds.parameters["UnitLength_in_cm"]

    # number of cells for the given window
    N_win = np.asarray(np.round(win_frac * 2**args.level), dtype=np.int32)

    # delta x of the cutout cube
    dx = win_kpc / N_win

    # extract cube
    cube = ds.covering_grid(level=args.level, left_edge=left, dims=N_win)

    data = {}
    for i, ix in zip([0, 1, 2], ["x", "y", "z"]):
        data[ix + "glob"] = np.linspace(
            left_kpc[i] + 0.5 * dx[i], right_kpc[i] - 0.5 * dx[i], N_win[i]
        )
        data[ix + "g_bnds"] = np.linspace(left_kpc[i], right_kpc[i], N_win[i] + 1)
        data[ix + "ctr"] = np.linspace(
            -0.5 * win_kpc[i] + 0.5 * dx[i],
            0.5 * win_kpc[i] - 0.5 * dx[i],
            N_win[i],
        )
        data[ix + "c_bnds"] = np.linspace(
            -0.5 * win_kpc[i], 0.5 * win_kpc[i], N_win[i] + 1
        )

    for f in args.fields:
        el = (args.particle, f)
        data[el] = cube[el]

        sizexy = 25000
        sizez = 5000
        deltaxy = sizexy / (2**args.level)
        deltaz = sizez / (2**args.level)
        hdu = fits.PrimaryHDU([512, 512, 512])
        hdu.header["NAXIS"] = 3
        hdu.header["WCSAXES"] = 3
        hdu.header["CRPIX1"] = 0.0
        hdu.header["CRPIX2"] = 0.0
        hdu.header["CRPIX3"] = 0.0
        hdu.header["CDELT1"] = deltaxy
        hdu.header["CDELT2"] = deltaxy
        hdu.header["CDELT3"] = deltaz
        hdu.header["CUNIT1"] = "pc"
        hdu.header["CUNIT2"] = "pc"
        hdu.header["CUNIT3"] = "pc"
        hdu.header["CTYPE1"] = "x"
        hdu.header["CTYPE2"] = "y"
        hdu.header["CTYPE3"] = "z"
        hdu.header["CRVAL1"] = -sizexy / 2
        hdu.header["CRVAL2"] = -sizexy / 2
        hdu.header["CRVAL3"] = -sizez / 2
        hdu.header["LATPOLE"] = 90.0
        # hdu.header["ORIGIN"] = "ROBIN-824"
        hdu.header["CREATOR"] = "YT"
        hdu.header["XCENTER"] = 0.5
        hdu.header["YCENTER"] = 0.5
        hdu.header["ZCENTER"] = 0.5
        hdu.header["ABS_X"] = 1.0
        hdu.header["ABS_Y"] = 0.0
        hdu.header["ABS_Z"] = 0.0
        hdu.header["ORD_X"] = 0.0
        hdu.header["ORD_Y"] = 1.0
        hdu.header["ORD_Z"] = 0.0
        hdu.header["LOS_X"] = 0.0
        hdu.header["LOS_Y"] = 0.0
        hdu.header["LOS_Z"] = 1.0
        # hdu.header["BUNIT"] = "g/cm3"

        fin = Path(args.file)
        fout = f"{fin.resolve().parent}/{fin.stem}-lv{args.level}-{args.particle}-{f}.fits"
        hduout = fits.PrimaryHDU((data[args.particle, f].value), header=hdu.header)
        hduout.writeto(fout, overwrite=True)
        print(fout)
