

list(APPEND _deps
  bufr_query)

list(APPEND _srcs
  main.cpp)

ecbuild_add_executable( TARGET  test.x
                        SOURCES ${_srcs}
                        LIBS ${_deps})

set_target_properties(test.x PROPERTIES INSTALL_RPATH "/opt/local/lib/libgcc")
