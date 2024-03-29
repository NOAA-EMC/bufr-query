
include(GNUInstallDirs)
include(Targets)

# Location of .pycodestyle for norm checking within IODA-converters
set( BUFR_PYLINT_CFG_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

set(pyver python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR})
set(PYDIR ${pyver}/bufr)

# Location of installed python iodaconv libraries
set( PYIODACONV_BUILD_LIBDIR   ${CMAKE_BINARY_DIR}/lib/${PYDIR} )
set( PYIODACONV_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib/${PYDIR} )


list (APPEND PYTHON_SRCS DataObjectFunctions.h
												DataObjectFunctions.cpp
												py_bufr.cpp
												py_result_set.cpp
												py_data_container.cpp
												py_query_set.cpp
												py_file.cpp
												py_parser.cpp
												py_ioda_description.cpp
												py_ioda_encoder.cpp
												py_data_cache.cpp)

# We cannot call ecbuild_add_library here since it conflicts with pybind11_add_module.
pybind11_add_module(bufr_python ${PYTHON_SRCS})

target_link_libraries(bufr_python PUBLIC bufr_lib)
target_link_libraries(bufr_python PUBLIC Eigen3::Eigen)
set(pyver python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR})

AddPyLib(bufr_python bufr ${pyver})

if(APPLE)
	set_target_properties(bufr_python PROPERTIES
		BUILD_RPATH_USE_ORIGIN TRUE
		BUILD_WITH_INSTALL_RPATH TRUE
		INSTALL_RPATH "@loader_path/../../"
		)
else()
	set_target_properties(bufr_python PROPERTIES
		BUILD_RPATH_USE_ORIGIN TRUE # $ORIGIN
		BUILD_WITH_INSTALL_RPATH TRUE
		INSTALL_RPATH "\$ORIGIN/../../"
		)
endif()

add_custom_command(TARGET bufr_python
									 POST_BUILD
									 COMMAND ${CMAKE_COMMAND} -E copy_directory
	                 				 ${CMAKE_CURRENT_SOURCE_DIR}/bufr
													 $<TARGET_FILE_DIR:bufr_python>)
