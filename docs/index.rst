BUFR Query
==========

BUFR Query is a library that provides a simple interface to read BUFR files using queries. The queries
are basically just the path string to the element you want.

Simple example
--------------
**\*/BRITCSTC/TMBR**

The path components are the NCEP mnemonics for the elements in the path.
(see :ref:`Query Path` for details).

A simple way to list all the possible queries for a BUFR file is also provided. This way you can
just copy and paste the elmenents you want (see :ref:`Show Queries` for details).

Contents
--------

.. toctree::
    :maxdepth: 2

  query_path
  yaml
  python_api
  software_architecture
