import pyvo as vo
from pyvo_utils import *


def columns(cone_url):
    """Returns the available columns in this service

    Parameters
    ----------
    cone_url : str
        Cone Search URL

    Returns
    -------
    list
        List of tuples (name, description)
    """
    cone_search = vo.dal.SCSService(cone_url)
    try:
        cols = [(col.name, col.description or str())
                for col in cone_search.columns]
        return make_response(exit_code=0, payload=cols)
    except Exception as ex:
        return make_response(exit_code=1, payload=str(ex))


def search(cone_url, ra, dec, radius, verbosity=2):
    """Submit a Simple Cone Search query

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
    verbosity : int, optional
        The volume of columns to return, by default 2

    Returns
    -------
    dict
        A dictionary with the following keys:
            - nels: number of rows
            - columns: a list of tuples (name, description)
            - table: a list of lists with table contents
    """
    cone_search = vo.dal.SCSService(cone_url)
    try:
        rs = cone_search.search(
            pos=(ra, dec), radius=radius, verbosity=verbosity)
        cols = [(field.name, field.description or str())
                for field in rs.fielddescs]
        table = [[str(el) for el in row] for row in rs.to_table()]
        payload = dict(nels=len(rs), columns=cols, table=table)
        return make_response(exit_code=0, payload=payload)
    except Exception as ex:
        return make_response(exit_code=1, payload=str(ex))
