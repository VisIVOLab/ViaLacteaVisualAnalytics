import os
import pyvo as vo
from pyvo_utils import *

import sys

def availability(url):
    """Check if the service is available

    Parameters
    ----------
    url : str
        TAP URL

    Returns
    -------
    bool
        True if the service is available, False otherwise
    """
    try:
        tap = vo.dal.TAPService(url)
        return 0 if tap.available else 1
    except Exception as ex:
        print(str(ex), end='')
        return 1


def band_tables(url):
    """Return a list of compact sources' tables

    Parameters
    ----------
    url : str
        TAP URL

    Returns
    -------
    list
        List of tables
    """
    tap = vo.dal.TAPService(url)
    try:
        tables = [t.name[t.name.rfind('.')+1:] for t in tap.tables if t.name.find(
            'compactsources.band') > -1 and t.name.endswith('um')]
        print(",".join(tables))
        return 0
    except Exception as ex:
        print(str(ex), end='')
        return 1


def search(url, query, out_file, out_format):
    """Submit a TAP query and save the result to a file

    Parameters
    ----------
    url : str
        TAP URL
    query : str
        ADQL Query
    out_file : str
        Absolute path of output file
    out_format : str
        Output format (e.g. "ascii.tab", "ascii.csv", ...)

    Returns
    -------
    dict
        A dictionary with the following keys:
            - nels: number of rows
            - filepath: Absolute path of output file
    """
    tap = vo.dal.TAPService(url)
    try:
        rs = tap.search(query)
        table = rs.to_table().filled()
        nels = len(rs)
        print(nels)
        if nels != 0:
            table.write(out_file, format=out_format)
            print(os.path.abspath(out_file), end='')
        return 0
    except Exception as ex:
        print(str(ex), end='')
        return 1


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f'Usage: {sys.argv[0]} <avail|tables|query> [args...]')
        sys.exit(1)
    
    if sys.argv[1] == "avail":
        if len(sys.argv) < 3:
            print(f'Usage: {sys.argv[0]} avail <url>')
            sys.exit(1)
        sys.exit(availability(sys.argv[2]))
    
    if sys.argv[1] == "tables":
        if len(sys.argv) < 3:
            print(f'Usage: {sys.argv[0]} tables <url>')
            sys.exit(1)
        sys.exit(band_tables(sys.argv[2]))
    
    if sys.argv[1] == "query":
        if len(sys.argv) < 6:
            print(f'Usage: {sys.argv[0]} query <url> <query> <out_file> <out_format>')
            sys.exit(1)
        url, query, out_file, out_format = sys.argv[2:6]
        sys.exit(search(url, query, out_file, out_format))
    
    print(f'Unknown operation')
    sys.exit(1)