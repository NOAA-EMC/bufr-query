.. _bufr-yaml:

Mapping YAML File
=================

The mapping YAML describes which BUFR fields to extract and how they will be
written to an output file.  It is divided into two top level sections:
``bufr`` and ``encoder``.  The ``bufr`` section controls what data are read
from the file while ``encoder`` describes how these data are encoded.  Complete
examples are available in ``test/testinput``.

BUFR section
------------

The ``bufr`` section controls how data are retrieved from the BUFR file.  It
supports grouping repeated sequences, splitting the dataset into categories and
filtering rows before encoding.  A trimmed example looks like:

.. code-block:: yaml

  bufr:
    group_by_variable: longitude  # Optional
    subsets:
      - NC004001
      - NC004002
      - NC004003
    variables:
      timestamp:
        datetime:
          year: "*/YEAR"
          month: "*/MNTH"
          day: "*/DAYS"
          hour: "*/HOUR"
          minute: "*/MINU"
          second: "*/SECO"  # default assumed zero if skipped or found as missing
          hoursFromUtc: 0  # Optional

      # Or, sometimes BUFR data use an offset time related to model analysis/cycle.
      timestamp:
        timeoffset:
          timeOffset: "*/PRSLEVEL/DRFTINFO/HRDR"
          transforms:
            - scale: 3600
          referenceTime: "2020-11-01T12:00:00Z"

      satellite_id:
        query: "*/SAID"
        type: int64
      longitude:
        query: "*/CLON"
        transforms:
          - wrap: [ -180.0, 180.0 ]
      latitude:
        query: "*/CLAT"
      channels:
        query: "[*/BRITCSTC/CHNM, */BRIT/CHNM]"
      radiance:
        query: "[*/BRITCSTC/TMBR, */BRIT/TMBR]"

    splits:
      satId:
        category:
          variable: satellite_id
          map:
            _3: sat_1  # can't use integers as keys
            _5: sat_2
            _8: sat_3

    filters:
      - bounding:
          variable: longitude
          upperBound: -68  # optional
          lowerBound: -86.3  # optional

``bufr`` keys
^^^^^^^^^^^^^

``group_by_variable`` *(optional)*
    Name of a variable used to group observations when expanding repeated
    sequences.

``subsets`` *(optional)*
    List of subset names to read.  When omitted all subsets matching the
    queries are processed.

``variables``
    Mapping of arbitrary names to variable descriptions.  These names are later
    referenced by the ``encoder`` section.  A variable description can be one of
    the following:

    - ``query`` – direct query into the BUFR tree.  Numeric results may apply
      ``offset``, ``scale`` or ``wrap`` transforms and the type may be forced to
      ``int``, ``int64``, ``float`` or ``double``.
    - ``datetime`` – combine mnemonics for year, month, day, hour and minute
      (and optionally second and hoursFromUtc) into an epoch time stored as
      seconds since ``1970-01-01T00:00:00Z``.
    - ``timeoffset`` – like ``datetime`` but the value is relative to a
      ``referenceTime``.  Transforms may be used to convert units.
    - specialised forms such as ``sensorScanAngle`` or
      ``remappedBrightnessTemperature`` used in some satellite mappings.

``splits`` *(optional)*
    Splits divide the dataset into categories.  The ``category`` split type
    partitions the data by the value of a variable and can map integer values to
    string names.

``filters`` *(optional)*
    Filters remove rows prior to encoding.  The ``bounding`` filter keeps rows
    whose values fall between ``lowerBound`` and ``upperBound`` (at least one of
    these bounds must be supplied).

Encoder section
---------------

The ``encoder`` section describes how the exported data should be written.  A
shortened example is shown below:

.. code-block:: yaml

  encoder:
    type: netcdf
    dimensions:
      - name: nchans
        paths:
          - "*/BRIT"
          - "*/BRITCSTC"
        source: variables/channels  # optional
        labels: "1-5, 8, 10-20"  # optional
    globals:
      - name: "platformCommonName"
        type: string
        value: "ATMS"
    variables:
      - name: "MetaData/dateTime"
        source: "variables/timestamp"
        longName: "dateTime"
        units: "seconds since 1970-01-01T00:00:00Z"

      - name: "MetaData/latitude"
        source: "variables/latitude"
        longName: "Latitude"
        units: "degrees_north"
        range: [-90, 90]

      - name: "MetaData/longitude"
        source: "variables/longitude"
        longName: "Longitude"
        units: "degrees_east"
        range: [-180, 180]

      - name: "ObsValue/radiance"
        coordinates: "longitude latitude nchans"
        source: "variables/radiance"
        longName: "Radiance"
        units: "K"
        range: [120, 500]
        chunks: [1000, 15]
        compressionLevel: 4

Encoder keys
^^^^^^^^^^^^

``type`` *(optional)*
    Output format such as ``netcdf`` or ``zarr``.

``dimensions`` *(optional)*
    List of named dimensions.  Each entry contains:

    - ``name`` – dimension name.
    - ``paths`` or ``path`` – queries used to determine the dimension.
    - ``source`` *(optional)* – exported data used to label the dimension.
    - ``labels`` *(optional)* – manual list of labels.  Use either ``source`` or ``labels``.

``variables``
    List of variables to create in the output file.  Each item includes:

    - ``name`` – path ``group/variable``.
    - ``source`` – reference to a variable defined in the ``bufr`` section.
    - ``coordinates`` *(optional)* – names of coordinate variables.
    - ``longName`` *(optional)* – descriptive name.
    - ``units`` *(optional)* – units string.
    - ``range`` *(optional)* – valid range ``[min, max]``.
    - ``chunks`` *(optional)* – chunk sizes for chunked outputs.
    - ``compressionLevel`` *(optional)* – gzip level ``0-9``.

``globals`` *(optional)*
    Global attributes to attach to the output file.  Each definition provides
    ``name``, ``type`` (``string``, ``int``, ``float``, ``intVector`` or
    ``floatVector``) and ``value``.

.. warning::

   ``MetaData/dateTime`` must use units ``seconds since 1970-01-01T00:00:00Z``.

