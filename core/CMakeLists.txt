# (C) Copyright 2020-2022 UCAR.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.

include(GNUInstallDirs)

## File definitions

list(APPEND BUFR_PUBLIC
	include/bufr/BufrTypes.h
	include/bufr/DataCache.h
	include/bufr/DataContainer.h
	include/bufr/DataObject.h
	include/bufr/BufrDescription.h
	include/bufr/BufrParser.h
	include/bufr/Export.h
	include/bufr/Filter.h
	include/bufr/Split.h
	include/bufr/Variable.h
	include/bufr/DataProvider.h
	include/bufr/NcepDataProvider.h
	include/bufr/WmoDataProvider.h
	include/bufr/File.h
	include/bufr/QuerySet.h
	include/bufr/QueryParser.h
	include/bufr/ResultSet.h
	include/bufr/Tokenizer.h
	include/bufr/SubsetTable.h
	include/bufr/Data.h
	include/bufr/IodaEncoder.h
	include/bufr/IodaDescription.h
)

list(APPEND BUFR_PRIVATE
	src/bufr/ObjectFactory.h
	src/bufr/DataContainer.cpp
	src/bufr/DataObject.cpp
	src/bufr/DataObjectBuilder.h
	src/bufr/Log.h
	src/bufr/BufrReader/BufrDescription.cpp
	src/bufr/BufrReader/BufrParser.cpp
	src/bufr/BufrReader/Exports/Export.cpp
	src/bufr/BufrReader/Exports/Filters/BoundingFilter.h
	src/bufr/BufrReader/Exports/Filters/BoundingFilter.cpp
	src/bufr/BufrReader/Exports/Splits/CategorySplit.h
	src/bufr/BufrReader/Exports/Splits/CategorySplit.cpp
	src/bufr/BufrReader/Exports/Variables/DatetimeVariable.h
	src/bufr/BufrReader/Exports/Variables/DatetimeVariable.cpp
	src/bufr/BufrReader/Exports/Variables/SpectralRadianceVariable.h
	src/bufr/BufrReader/Exports/Variables/SpectralRadianceVariable.cpp
	src/bufr/BufrReader/Exports/Variables/RemappedBrightnessTemperatureVariable.h
	src/bufr/BufrReader/Exports/Variables/RemappedBrightnessTemperatureVariable.cpp
	src/bufr/BufrReader/Exports/Variables/SensorScanAngleVariable.h
	src/bufr/BufrReader/Exports/Variables/SensorScanAngleVariable.cpp
	src/bufr/BufrReader/Exports/Variables/SensorScanPositionVariable.h
	src/bufr/BufrReader/Exports/Variables/SensorScanPositionVariable.cpp
	src/bufr/BufrReader/Exports/Variables/AircraftAltitudeVariable.h
	src/bufr/BufrReader/Exports/Variables/AircraftAltitudeVariable.cpp
	src/bufr/BufrReader/Exports/Variables/TimeoffsetVariable.h
	src/bufr/BufrReader/Exports/Variables/TimeoffsetVariable.cpp
	src/bufr/BufrReader/Exports/Variables/QueryVariable.h
	src/bufr/BufrReader/Exports/Variables/QueryVariable.cpp
	src/bufr/BufrReader/Exports/Variables/WigosidVariable.cpp
	src/bufr/BufrReader/Exports/Variables/WigosidVariable.cpp
	src/bufr/BufrReader/Exports/Variables/Transforms/Transform.h
	src/bufr/BufrReader/Exports/Variables/Transforms/OffsetTransform.h
	src/bufr/BufrReader/Exports/Variables/Transforms/OffsetTransform.cpp
	src/bufr/BufrReader/Exports/Variables/Transforms/ScalingTransform.h
	src/bufr/BufrReader/Exports/Variables/Transforms/ScalingTransform.cpp
	src/bufr/BufrReader/Exports/Variables/Transforms/TransformBuilder.h
	src/bufr/BufrReader/Exports/Variables/Transforms/TransformBuilder.cpp
	src/bufr/BufrReader/Query/DataProvider/DataProvider.cpp
	src/bufr/BufrReader/Query/DataProvider/NcepDataProvider.cpp
	src/bufr/BufrReader/Query/DataProvider/WmoDataProvider.cpp
	src/bufr/BufrReader/Query/File.cpp
	src/bufr/BufrReader/Query/VectorMath.h
	src/bufr/BufrReader/Query/QuerySet.cpp
	src/bufr/BufrReader/Query/QuerySetImpl.h
	src/bufr/BufrReader/Query/QuerySetImpl.cpp
	src/bufr/BufrReader/Query/QueryRunner.h
	src/bufr/BufrReader/Query/QueryRunner.cpp
	src/bufr/BufrReader/Query/QueryParser.cpp
	src/bufr/BufrReader/Query/ResultSetImpl.h
	src/bufr/BufrReader/Query/ResultSetImpl.cpp
	src/bufr/BufrReader/Query/ResultSet.cpp
	src/bufr/BufrReader/Query/Target.h
	src/bufr/BufrReader/Query/Tokenizer.cpp
	src/bufr/BufrReader/Query/SubsetTable.cpp
	src/bufr/BufrReader/Query/SubsetLookupTable.h
	src/bufr/BufrReader/Query/SubsetLookupTable.cpp
	src/bufr/IodaEncoder/IodaEncoder.cpp
	src/bufr/IodaEncoder/IodaDescription.cpp

	# atms
	src/bufr/BufrReader/Exports/Variables/Transforms/atms/atms_kinds.F90
	src/bufr/BufrReader/Exports/Variables/Transforms/atms/atms_spatial_average_mod.f90
	src/bufr/BufrReader/Exports/Variables/Transforms/atms/atms_spatial_average_interface.h
	src/bufr/BufrReader/Exports/Variables/Transforms/atms/atms_spatial_average_interface.f90
)

source_group("core//include" FILES ${BUFR_PUBLIC})
source_group("core//src" FILES ${BUFR_PRIVATE})


## Define which source files are compiled.
list(APPEND BUFR_SRCS
	${BUFR_PUBLIC}
	${BUFR_PRIVATE}
	)

## Create options files that specify compilation options, available libraries,
## and test file paths.

## Create the library
ecbuild_add_library( TARGET bufr_lib
                     SOURCES ${BUFR_SRCS}
                     INSTALL_HEADERS LISTED
                     HEADER_DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
                     LINKER_LANGUAGE CXX )

## Linking options
target_link_libraries(bufr_lib PUBLIC ioda)
target_link_libraries(bufr_lib PUBLIC gsl::gsl-lite)
target_link_libraries(bufr_lib PUBLIC Eigen3::Eigen)
target_link_libraries(bufr_lib PUBLIC udunits::udunits)
target_link_libraries(bufr_lib PUBLIC oops)
target_link_libraries(bufr_lib PUBLIC eckit)
target_link_libraries(bufr_lib PUBLIC bufr::bufr_4)

## Public include files

target_include_directories(bufr_lib PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
	)

install(DIRECTORY include/bufr
				DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
				COMPONENT Headers)
