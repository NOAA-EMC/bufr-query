.. _quick-start:
   :tocdepth: 3

Quick Start
===========

Installation
-------------

Prerequisites
^^^^^^^^^^^^^

    - `ecbuild`_ (ecmwf)
    - openMP
    - openMPI
    - Eigen3
    - NetCDF4 (c++ version)
    - `gsl-lite`_
    - `eckit`_ (ecmwf)
    - `wxflow`_ (NOAA-EMC)
    - `NCEPLibs-bufr`_ (NOAA-EMC)

.. _ecbuild: https://github.com/ecmwf/ecbuild
.. _gsl-lite: https://github.com/gsl-lite/gsl-lite
.. _eckit: https://github.com/ecmwf/eckit
.. _wxflow: https://github.com/NOAA-EMC/wxflow
.. _NCEPLibs-bufr: https://github.com/NOAA-EMC/NCEPLIBS-bufr

Build and Install
^^^^^^^^^^^^^^^^^

Option 1 (Manual Install)
+++++++++++++++++++++++++

.. code-block:: bash

    git clone https://github.com/NOAA-EMC/bufr-query.git
    mkdir bufr-query/build
    cd bufr-query/build
    ecbuild -DCMAKE_INSTALL_PREFIX=<my install dir> -DPython3_EXECUTABLE=`which python` ..
    make install

    # Extend the python path (may want to add to your shell configuration files)
    export PYTHONPATH=<my install dir>/lib64/python<version>/site-packages:$PYTHONPATH

.. note::

    You may also need to include other cmake arguments, such as -DCMAKE_PREFIX_PATH, if you installed
    any of the requirements manually.

Option 2 (via ObsForge)
+++++++++++++++++++++++

Bufr-Query is installed as part of the NOAA EMC `ObsForge <obsforge>`_ project. Please follow their instructions.

.. _obsforge: https://github.com/NOAA-EMC/obsForge


Using It
--------

There are many ways to run the BUFR-Query lib either on the command line, via your own python or C++ code,
or via the `JCSDA IODA <ioda>`_ framework to ingest data directly into a `JEDI <jedi>`_ application.

Basically there are several steps involved in reading a BUFR file.
    1) Identify the data fields (by their queries) you want to read (see `Query Path <query_path>`_).
       Its recommended to compile your set of queries into a description :doc:`yaml` file.
    2) Either use your YAML file directly, create an :doc:`ObsBuilder <obs_builder>` module
       (recommended), or write a custom Python :doc:`script <python_api>` to use your queries.

Command Line
^^^^^^^^^^^^

bufr2netcdf.x
+++++++++++++

Executable that processes a BUFR file based on the provided yaml mapping file.

Examples:

.. code-block:: bash

    # Single threaded
    bufr2netcdf.x gdas.1bmhs.bufr_d mhs_mapping.yaml mhs.nc

    # Using mpi
    mpirun -n 4 bufr2netcdf.x gdas.1bmhs.bufr_d mhs_mapping.yaml mhs.nc

    # Using slurm
    srun -A <my account> -n 4 bufr2netcdf.x gdas.1bmhs.bufr_d mhs_mapping.yaml mhs.nc

    # Splitting according to sat ID in yaml spec
    bufr2netcdf.x gdas.1bmhs.bufr_d split_mhs_mapping.yaml mhs-{splits/satId}.nc

    # Running WMO bufr file requires extra arguments
    bufr2netcdf.x -t <BUFR table dir> wmo.bufr_d wmo_mapping.yaml wmo.nc

ObsBuilder
++++++++++

:doc:`Obsbuilder <obs_builder>` modules can be executed from the command line directly by running them via python.

.. code-block:: bash

    python my_obsbuilder.py  --input gdas.1bmhs.bufr_d  --output mhs.nc

The arguments will mirror those defined in the ObsBuilder method create_obs_file which you can
optionaly override.

You can of coarse also import your obsbuilder in a custom python script and call it directly.

IODA
^^^^

`BUFR (yaml) interface <bufr>`_

`SCRIPTS <script>`_

.. note::

    ObsBuilder modules add the necessary function to the obsbuilder file without writing extra code.

.. _bufr: https://jointcenterforsatellitedataassimilation-jedi-docs.readthedocs-hosted.com/en/latest/inside/jedi-components/ioda/format-bufr.html
.. _script: https://jointcenterforsatellitedataassimilation-jedi-docs.readthedocs-hosted.com/en/latest/inside/jedi-components/ioda/format-script.html

Examples
--------

There are many examples available. Here is a list of a few places to look:

    - The test/testinput directory of **bufr-query**.
    - The NOAA EMC `SPOC <spoc>`_ project (dump directory).
    - The NOAA EMC `Ocelot <ocelot>`_ project (data_prep/mapping directory).

.. _spoc: https://github.com/NOAA-EMC/spoc
.. _ocelot: https://github.com/NOAA-EMC/ocelot/tree/main/data_prep/mapping
