LIST(APPEND _deps
  bufr_lib
)

LIST(APPEND _srcs
  bufr2ioda.cpp
)

ecbuild_add_executable( TARGET  bufr2ioda.x
                        SOURCES ${_srcs}
                        LIBS    ${_deps})
