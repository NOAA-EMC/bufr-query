LIST(APPEND _deps
  bufr_query
  eckit_mpi
)

LIST(APPEND _srcs
  bufr2netcdf.cpp
)

include(GNUInstallDirs)
include(Targets)

ecbuild_add_executable( TARGET  bufr2netcdf.x
                        SOURCES ${_srcs}
                        LIBS    ${_deps})

AddApp(bufr2netcdf.x)
set_target_properties(bufr2netcdf.x PROPERTIES BUILD_RPATH "${CMAKE_BINARY_DIR}/lib")
