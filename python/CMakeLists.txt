
include(GNUInstallDirs)
include(Targets)

# Location of .pycodestyle for norm checking within IODA-converters
set( BUFR_PYLINT_CFG_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

list (APPEND PYTHON_SRCS
  DataObjectFunctions.h
  DataObjectFunctions.cpp
  py_bufr.cpp
  py_result_set.cpp
  py_data_container.cpp
  py_query_set.cpp
  py_file.cpp
  py_parser.cpp
  py_data_cache.cpp
  py_encoder_base.cpp
  py_encoder_description.cpp
  py_netcdf_encoder.cpp
  py_mpi.h
  py_mpi.cpp
)

# We cannot call ecbuild_add_library here since it conflicts with pybind11_add_module.
pybind11_add_module(bufr_python ${PYTHON_SRCS})

target_link_libraries(bufr_python PUBLIC bufr_query)
target_link_libraries(bufr_python PUBLIC Eigen3::Eigen)
set(pyver python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR})

AddPyLib(bufr_python bufr ${pyver})

# Determine INSTALL_RPATH relative to the python module location
file(RELATIVE_PATH _py_relpath
    "${CMAKE_INSTALL_FULL_LIBDIR}/${pyver}/site-packages/bufr"
    "${CMAKE_INSTALL_FULL_LIBDIR}")
if(APPLE)
    set(_full_rpath "@loader_path/${_py_relpath}")
else()
    set(_full_rpath "\$ORIGIN/${_py_relpath}")
endif()

set_target_properties(bufr_python PROPERTIES
        BUILD_RPATH_USE_ORIGIN TRUE
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "${_full_rpath}")

add_custom_command(
    TARGET bufr_python
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/bufr
            $<TARGET_FILE_DIR:bufr_python>
    )

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bufr
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/${pyver}/site-packages
        COMPONENT Python
        FILES_MATCHING PATTERN "*.py")
