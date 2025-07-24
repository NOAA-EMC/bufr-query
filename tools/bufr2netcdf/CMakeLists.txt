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

GetInstallRpath(_rp)
if(_rp)
    set_target_properties(bufr2netcdf.x PROPERTIES INSTALL_RPATH "${_rp}")
endif()
