import bufr


def test_catch_exceptions():
    try:
        raise bufr.BadParameter("bad param")
    except bufr.BadParameter:
        pass
    else:
        assert False, "BadParameter not caught"

    try:
        raise bufr.BadValue("bad value")
    except bufr.BadValue:
        pass
    else:
        assert False, "BadValue not caught"

    try:
        raise bufr.MissingData("missing")
    except bufr.MissingData:
        pass
    else:
        assert False, "MissingData not caught"

    try:
        raise bufr.Exception("generic")
    except bufr.Exception:
        pass
    else:
        assert False, "Exception not caught"


if __name__ == "__main__":
    test_catch_exceptions()
