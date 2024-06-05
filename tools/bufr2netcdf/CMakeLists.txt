LIST(APPEND _deps
  bufr_query
)

LIST(APPEND _srcs
  bufr2netcdf.cpp
)

ecbuild_add_executable( TARGET  bufr2netcdf.x
                        SOURCES ${_srcs}
                        LIBS    ${_deps})
