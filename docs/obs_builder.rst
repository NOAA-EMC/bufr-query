.. _obs-builder:

Obs Builder
===========

Obs builder is a framework that provides a simple and consistent way to create "mapping" files that
are used to read BUFR files (when a YAML file is not enough).

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
output.nc`). The `add_main_functions` methods creates the expected module level functions using the
the class definitions for create_obs_file and create_obs_group (their signatures) as the template.

Common Overrides
----------------

The following methods are the most common methods that are overridden in the ObsBuilder class.
Overriding each method is optional, as the ObsBuilder base class provides default implementations.

.. autoclass:: bufr.obs_builder.ObsBuilder

   .. automethod:: __init__
      :noindex:

   .. automethod:: make_obs

   .. automethod:: create_obs_file

   .. automethod:: create_obs_group

   .. automethod:: _make_description


Here is a simple example of a class that implements the ObsBuilder class and demonstrates its
intended usage:

.. code-block:: python

   MAPPING_PATH = map_path('bufr_satwnd.yaml')

   class SatWndObsBuilder(ObsBuilder):
      def __init__(self):
         super().__init__(MAPPING_PATH, log_name=os.path.basename(__file__))

      def make_obs(self, comm,  input : Union[str, dict])
         assert isinstance(input, str), "Input must be a string"

         container = super().make_obs(comm, input)

         wdir = container.get('windDirection')
         wspd = container.get('windSpeed')

         uob, vob = self._compute_wind_components(wdir, wspd)
         paths = container.get_paths('windSpeed')

         container.add('windEastward', uob, paths)
         container.add('windNorthward', vob, paths)

         container.apply_mask(wspd > 0.0)
         return container

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

Add Main Functions
------------------

The ObsBuilder class provides a method called **add_main_functions** which automatically creates adds
the main functions that are expected on a mapping file.

Here is a list of the functions that are created:

- **if __name__ == '__main__'**: Defenition that allows the file to be executed as a script.

- **create_obs_file**: Function that takes the input file and creates an output file.

- **create_obs_group**: Function that is used to create an IODA obsgroup object. It is used by the
                      IODA script interface.

.. note::

   The add_main_functions method will create the functions with the same signature as the
   overridden methods. If you do not override the methods, the default signatures will be used.
   The default signatures are:

   - create_obs_file(input, output, type='netcdf', append=False, config=None):
   - create_obs_group(input, env, category:list=None, cache_categories:list=None, config=None):

   `config` is a dictionary that is used to initialize the ObsBuilder object.