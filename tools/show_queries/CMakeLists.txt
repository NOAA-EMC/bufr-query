

list(APPEND _deps
            bufr_query)
            
list(APPEND _srcs
            show_queries.cpp
            QueryPrinter.h
            QueryPrinter.cpp
            NcepQueryPrinter.h
            NcepQueryPrinter.cpp
            WmoQueryPrinter.h
            WmoQueryPrinter.cpp)

ecbuild_add_executable( TARGET  show_queries.x
                        SOURCES ${_srcs}
                        LIBS ${_deps})

set_target_properties(show_queries.x PROPERTIES INSTALL_RPATH "/opt/local/lib/libgcc")
