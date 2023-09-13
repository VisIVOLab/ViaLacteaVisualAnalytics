import os
import pyvo as vo
from pyvo_utils import *


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
    tap = vo.dal.TAPService(url)
    try:
        return make_response(exit_code=0, payload=tap.available)
    except Exception as ex:
        return make_response(exit_code=1, payload=str(ex))


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
        return make_response(exit_code=0, payload=tables)
    except Exception as ex:
        return make_response(exit_code=1, payload=str(ex))


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
        payload = dict(nels=nels)
        if nels != 0:
            table.write(out_file, format=out_format)
            payload['filepath'] = os.path.abspath(out_file)
        return make_response(exit_code=0, payload=payload)
    except Exception as ex:
        return make_response(exit_code=1, payload=str(ex))
