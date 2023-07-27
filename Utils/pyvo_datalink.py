import pyvo as vo
from pyvo_utils import *


def _soda_cutout(url, service_def):
    """Send Datalink Request and extract SODA Cutout information

    Parameters
    ----------
    url : str
        Datalink URL obtained from ObsCore
    service_def : str
        Should be either 'soda-sync' or 'soda-async'

    Returns
    -------
    dict
        A dictionary with the following keys:
            - url: SODA endpoint
            - id: Dataset ID
    """
    datalink = vo.dal.adhoc.DatalinkQuery(prepare_datalink_request(url))
    try:
        rs = datalink.execute()
        els = [el for el in rs.bysemantics(
            '#cutout') if el.service_def == service_def]

        if len(els) == 0:
            raise Exception('No element found.')

        el = els.pop()
        payload = dict(url=el.access_url, id=str(el.id))
        return make_response(exit_code=0, payload=payload)
    except Exception as ex:
        return make_response(exit_code=1, payload=str(ex))


def soda_cutout_sync(url):
    """Send Datalink Request and extract SODA Cutout information (sync)

    Parameters
    ----------
    url : str
        Datalink URL obtained from ObsCore

    Returns
    -------
    dict
        A dictionary with the following keys:
            - url: SODA endpoint
            - id: Dataset ID
    """
    return _soda_cutout(url=url, service_def='soda-sync')


def soda_cutout_async(url):
    """Send Datalink Request and extract SODA Cutout information (async)

    Parameters
    ----------
    url : str
        Datalink URL obtained from ObsCore

    Returns
    -------
    dict
        A dictionary with the following keys:
            - url: SODA endpoint
            - id: Dataset ID
    """
    return _soda_cutout(url=url, service_def='soda-async')
