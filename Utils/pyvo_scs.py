#!/usr/bin/env python3

import pyvo as vo

import pathlib
import sys
import uuid

def search(output_dir, cone_url, ra, dec, radius, verbosity):
    """
    Submit a Simple Cone Search query and save
    the results (csv and votable)

    Parameters
    ----------
    cone_url : str
        Cone Search URL
    ra : float
        Right Ascension (degrees)
    dec : float
        Declination (degrees)
    radius : float
        Search radius (degrees)
    verbosity : int
        The volume of columns to return

    Returns
    -------
    0 on success, csv absolute path in stdout
    1 on exceptions, error message in stdout
    """
    cone_search = vo.dal.SCSService(cone_url)
    try:
        rs = cone_search.search(
            pos=(ra, dec), radius=radius, verbosity=verbosity)
        name = uuid.uuid4()
        votable_file = pathlib.Path(output_dir).absolute() / f'{name}.xml'
        csv_file = pathlib.Path(output_dir).absolute() / f'{name}.csv'
        rs.votable.to_xml(votable_file.as_posix())
        rs.to_table().write(csv_file.as_posix())
        print(csv_file.as_posix(), end='')
        return 0
    except Exception as ex:
        print(str(ex), end='')
        return 1


if __name__ == "__main__":
    if len(sys.argv) < 6 or len(sys.argv) > 7:
        print(f'Usage: {sys.argv[0]} <output_dir> <url> <ra> <dec> <radius> [verbosity]')
        sys.exit(1)
    output_dir, cone_url, ra, dec, radius = sys.argv[1:6]
    verbosity = sys.argv[6] if len(sys.argv) == 7 else 2
    sys.exit(search(output_dir, cone_url, float(ra), float(dec), float(radius), int(verbosity)))
