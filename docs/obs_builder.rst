.. _obs-builder:

Obs Builder
===========

Obs builder is a framework that provides a simple and consistent way to create "mapping" files that
are used to read BUFR files (when just a YAML file is not enough).

.. uml:: uml/BUFR_ObsBuilder.puml
    :width: 75%
    :align: center
    :alt: Big Picture Sequence Diagram

As can be seen from the diagram, the ObsBuilder is the core object of the framework. The intent is
for client applications to create a subclasses of the ObsBuilder and make a custom version of it
by overriding the methods that are needed. This can be especially useful when you are creating a
set of mapping files for a family of related observation types that have a lot of commonality.

Once the ObsBuilder is created, the framework provides a method called `add_main_functions` which
automatically creates the main functions that are expected on a mapping file. Once executed the
files can be executed via the command line (ex: `python ./bufr_satwnd_amv_avhrr.py input.bufr
output.nc`).

Common Overrides
----------------

The following methods are the most common methods that are overridden in the ObsBuilder class.
Overriding each method is optional, as the ObsBuilder base class provides default implementations.

.. autoclass:: bufr.obs_builder.ObsBuilder
   :members: make_obs
   :private-members: _make_description

   .. automethod:: make_obs

      This method is the main method that can be overriden (optional). Its objective is to read the
      bufr file and to create an DataContainer object and return it. The data container is used
      in conjunction with the encoder Descriptor to encode the data into the format of your choice. Here
      is an typical example for what an override might looke like:

      .. code-block:: python

         def make_obs(self, comm, input_path):
            # Get container from mapping file first
            container = super().make_obs(comm, input_path)

            uob, vob = self._compute_wind_components(wdir, wspd)
            paths = container.get_paths('windSpeed', cat)

            container.add('windEastward', uob, paths, cat)
            container.add('windNorthward', vob, paths, cat)

            return container

      .. note::

         The make obs method calls its base classes implementation for the make_obs method. If this is not
         desired, you could create the DataContainer object yourself by using the
         `bufr.Parser(input_path, mapping_path).parse(comm)` method. The suggested behavior allows
         the base class to add common parameters to the DataContainer object.

   .. automethod:: _make_description

      This method is used to make the description that the data Encoder will use to encode the data.
      Here is an example of what an override might look like:

      .. code-block:: python

         def _make_description(self):
            description = super()._make_description()

            description.add_variables([
              {
                  'name': 'ObsType/windEastward',
                  'source': 'windEastward',
                  'units': '1',
                  'longName': 'Eastward Wind Component',
              },
              {
                  'name': 'ObsType/windNorthward',
                  'source': 'windNorthward',
                  'units': '1',
                  'longName': 'Northward Wind Component',
              }])

            return description

      .. note::

         The make obs method calls its base classes implementation for the make_obs method. If this is not
         desired, you could create the DataContainer object yourself by using the
         `bufr.Parser(input_path, mapping_path).parse(comm)` method. The suggested behavior allows
         the base class to add common parameters to the DataContainer object.


Add Main Functions
------------------

The ObsBuilder class provides a method called `add_main_functions` which automatically creates the main
functions that are expected on a mapping file.

.. autofunction:: bufr.obs_builder.add_main_functions
