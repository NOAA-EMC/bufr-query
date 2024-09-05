# (C) Copyright 2023 NOAA/NWS/NCEP/EMC

import bufr

def test_field_val():
    DATA_PATH = 'testdata/gdas.t00z.1bhrs4.tm00.bufr_d'

    # Make the QuerySet for all the data we want
    q = bufr.QuerySet()
    q.add('latitude', '*/CLON')

    # Open the BUFR file and execute the QuerySet
    with bufr.File(DATA_PATH) as f:
        r = f.execute(q)

    try:
        r.get('lattittude')
    except Exception as e:
        return  # Success!

    assert False, "Did not throw exception for invalid field name."

def test_groupby_field_val():
    DATA_PATH = 'testdata/gdas.t00z.1bhrs4.tm00.bufr_d'

    # Make the QuerySet for all the data we want
    q = bufr.QuerySet()
    q.add('latitude', '*/CLON')

    # Open the BUFR file and execute the QuerySet
    with bufr.File(DATA_PATH) as f:
        r = f.execute(q)

    try:
        r.get('latitude', 'latittude')
    except Exception as e:
        return  # Success!

    assert False, "Did not throw exception for invalid groupby field name."


if __name__ == '__main__':
    test_field_val()
    test_groupby_field_val()
