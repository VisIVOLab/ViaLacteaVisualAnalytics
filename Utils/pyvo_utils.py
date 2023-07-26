def make_response(exit_code, payload):
    """Make response for C++ code

    Parameters
    ----------
    exit_code : int
        It should be 0 on a successful request
    payload : any
        Request outcome

    Returns
    -------
    dict
        Dictionary object for C++ code.

        If exit_code is 0, a 'result' key will contain the payload.
        If exit_code is not 0, an 'error' key will contain the exception message
    """
    key = 'result' if exit_code == 0 else 'error'
    return dict([('exit_code', exit_code), (key, payload)])
