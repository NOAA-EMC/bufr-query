# (C) Copyright 2024 NOAA/NCEP/EMC.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.


# Function definitions

# The list of all test input config files, which need to be symlink from the build folder.
# (These symlink are created at the end of this file, to give CMake functions setting up certain
# tests a chance to append items to this list).
list( APPEND input_files
  testinput/bufrtest_filtering.yaml
  testinput/bufrtest_filtering_mapping.yaml
  testinput/bufrtest_split.yaml
  testinput/bufrtest_split_mapping.yaml
  testinput/bufrtest_filter_split.yaml
  testinput/bufrtest_filter_split_mapping.yaml
  testinput/bufrtest_empty_fields.yaml
  testinput/bufrtest_empty_fields_mapping.yaml
  testinput/bufrtest_simple_groupby.yaml
  testinput/bufrtest_simple_groupby_mapping.yaml
  testinput/bufrtest_read_2_dim_blocks.yaml
  testinput/bufrtest_read_2_dim_blocks_mapping.yaml
  testinput/bufrtest_mhs_basic.yaml
  testinput/bufrtest_mhs_basic_mapping.yaml
  testinput/bufrtest_hrs_basic.yaml
  testinput/bufrtest_hrs_basic_mapping.yaml
  testinput/bufrtest_adpupa.yaml
  testinput/bufrtest_adpupa_mapping.yaml
  testinput/bufrtest_amua_ta.yaml
  testinput/bufrtest_amua_ta_mapping.yaml
  testinput/bufrtest_amua_n15.yaml
  testinput/bufrtest_amua_n15_mapping.yaml
  testinput/bufrtest_esamua.yaml
  testinput/bufrtest_esamua_mapping.yaml
  testinput/bufrtest_mhs.yaml
  testinput/bufrtest_mhs_mapping.yaml
  testinput/bufrtest_esmhs.yaml
  testinput/bufrtest_esmhs_mapping.yaml
  testinput/bufrtest_adpsfc.yaml
  testinput/bufrtest_adpsfc_mapping.yaml
  testinput/bufrtest_adpsfc_prepbufr.yaml
  testinput/bufrtest_adpsfc_prepbufr_mapping.yaml
  testinput/bufrtest_adpsfc_snow.yaml
  testinput/bufrtest_adpsfc_snow_mapping.yaml
  testinput/bufrtest_aircar.yaml
  testinput/bufrtest_aircar_mapping.yaml
  testinput/bufrtest_aircft_noamdar103.yaml
  testinput/bufrtest_aircft_noamdar103_mapping.yaml
  testinput/bufrtest_aircft_amdar103.yaml
  testinput/bufrtest_aircft_amdar103_mapping.yaml
  testinput/bufrtest_aircft_prepbufr.yaml
  testinput/bufrtest_aircft_prepbufr_mapping.yaml
  testinput/bufrtest_wmo_radiosonde.yaml
  testinput/bufrtest_wmo_radiosonde_mapping.yaml
  testinput/bufrtest_satwnd_new_format.yaml
  testinput/bufrtest_satwnd_new_format_mapping.yaml
  testinput/bufrtest_satwnd_old_format.yaml
  testinput/bufrtest_satwnd_old_format_mapping.yaml
  testinput/bufrtest_satwnd_avhrr.yaml
  testinput/bufrtest_satwnd_avhrr_mapping.yaml
  testinput/bufrtest_mtiasi.yaml
  testinput/bufrtest_mtiasi_mapping.yaml
  testinput/bufrtest_mtiasi_cached.yaml
  testinput/bufrtest_atms.yaml
  testinput/bufrtest_atms_mapping.yaml
  testinput/bufrtest_sfcshp.yaml
  testinput/bufrtest_sfcshp_mapping.yaml
  testinput/bufrtest_adpupa_prepbufr.yaml
  testinput/bufrtest_adpupa_prepbufr_mapping.yaml
  testinput/bufrtest_rtma_mesonet.yaml
  testinput/bufrtest_rtma_mesonet_mapping.yaml
  testinput/bufrtest_rtma_adpsfc.yaml
  testinput/bufrtest_rtma_adpsfc_mapping.yaml
  testinput/bufrtest_sevcsr.yaml
  testinput/bufrtest_sevcsr_mapping.yaml
  testinput/bufrtest_specific_subsets_by_query.yaml
  testinput/bufrtest_specific_subsets_by_query_mapping.yaml
  testinput/bufrtest_specifying_subsets.yaml
  testinput/bufrtest_specifying_subsets_mapping.yaml
  testinput/bufrtest_lgycld_rrfs.yaml
  testinput/bufrtest_lgycld_rrfs_mapping.yaml
  testinput/bufrtest_adpsfc_cloud_rrfs_prepbufr.yaml
  testinput/bufrtest_adpsfc_cloud_rrfs_prepbufr_mapping.yaml
  testinput/bufrtest_long_strs.yaml
  testinput/bufrtest_long_strs_mapping.yaml
  testinput/bufrtest_rtma_aircft.yaml
  testinput/bufrtest_rtma_aircft_mapping.yaml
  testinput/bufrtest_rtma_aircft_NC004103.yaml
  testinput/bufrtest_rtma_aircft_NC004103_mapping.yaml
  testinput/bufrtest_wmo_amdar_multi.yaml
  testinput/bufrtest_wmo_amdar_multi_mapping.yaml
  testinput/bufrtest_fieldname_validation.py
  testinput/bufrtest_python_test.py
  testinput/bufrtest_python_to_ioda.yaml
  testinput/bufrtest_python_to_ioda.py
)


add_library(ioda_test INTERFACE)
set(BUILD_DIR_TEST_INCLUDE_PATH ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/test/include)
add_custom_target(ioda_test_headers ALL COMMAND ${CMAKE_COMMAND} -E make_directory "${BUILD_DIR_TEST_INCLUDE_PATH}/${PROJECT_NAME}"
                                    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR} ${BUILD_DIR_TEST_INCLUDE_PATH}/${PROJECT_NAME}/test)
add_dependencies(ioda_test ioda_test_headers)
target_include_directories(ioda_test INTERFACE $<BUILD_INTERFACE:${BUILD_DIR_TEST_INCLUDE_PATH}>)
target_link_libraries(ioda_test INTERFACE ioda)


ecbuild_add_test( TARGET  test_ioda_bufr_filtering
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_filtering.yaml"
                          bufrtest_filtering.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_split
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_split.yaml"
                          bufrtest_split.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_filter_split
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_filter_split.yaml"
                          bufrtest_filter_split.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_empty_fields
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_empty_fields.yaml"
                          bufrtest_empty_fields.nc)


ecbuild_add_test( TARGET  test_ioda_bufr_simple_groupby
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_simple_groupby.yaml"
                          bufrtest_simple_groupby.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_read_2_dim_blocks
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_read_2_dim_blocks.yaml"
                          bufrtest_read_2_dim_blocks.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_mhs_basic
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_mhs_basic.yaml"
                          bufrtest_mhs_basic.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_hrs_basic
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_hrs_basic.yaml"
                          bufrtest_hrs_basic.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_adpupa
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_adpupa.yaml"
                          bufrtest_adpupa.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_amua_ta
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_amua_ta.yaml"
                          bufrtest_amua_ta.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_amua_n15
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_amua_n15.yaml"
                          bufrtest_amua_n15.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_esamua
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_esamua.yaml"
                          bufrtest_esamua.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_mhs
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_mhs.yaml"
                          bufrtest_mhs.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_esmhs
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_esmhs.yaml"
                          bufrtest_esmhs.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_adpsfc
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_adpsfc.yaml"
                          bufrtest_adpsfc.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_adpsfc_prepbufr
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_adpsfc_prepbufr.yaml"
                          bufrtest_adpsfc_prepbufr.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_adpsfc_snow
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_adpsfc_snow.yaml"
                          bufrtest_adpsfc_snow.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_aircar
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_aircar.yaml"
                          bufrtest_aircar.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_aircft_noamdar103
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_aircft_noamdar103.yaml"
                          bufrtest_aircft_noamdar103.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_aircft_amdar103
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_aircft_amdar103.yaml"
                          bufrtest_aircft_amdar103.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_aircft_prepbufr
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_aircft_prepbufr.yaml"
                          bufrtest_aircft_prepbufr.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_wmo_radiosonde
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_wmo_radiosonde.yaml"
                          bufrtest_wmo_radiosonde.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_satwnd_old_format
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_satwnd_old_format.yaml"
                          bufrtest_satwnd_old_format.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_satwnd_new_format
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_satwnd_new_format.yaml"
                          bufrtest_satwnd_new_format.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_satwnd_avhrr
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_satwnd_avhrr.yaml"
                          bufrtest_satwnd_avhrr.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_mtiasi
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_mtiasi.yaml"
                          bufrtest_mtiasi.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_mtiasi_cached
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_mtiasi_cached.yaml"
                          bufrtest_mtiasi_metop_b.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_atms
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_atms.yaml"
                          bufrtest_atms.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_sfcshp
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_sfcshp.yaml"
                          bufrtest_sfcshp.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_adpupa_prepbufr
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_adpupa_prepbufr.yaml"
                          bufrtest_adpupa_prepbufr.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_rtma_mesonet
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_rtma_mesonet.yaml"
                          bufrtest_rtma_mesonet.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_rtma_adpsfc
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_rtma_adpsfc.yaml"
                          bufrtest_rtma_adpsfc.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_sevcsr
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_sevcsr.yaml"
                          bufrtest_sevcsr.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_specific_subsets_by_query
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_specific_subsets_by_query.yaml"
                          bufrtest_specifying_subsets.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_specifying_subsets
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_specifying_subsets.yaml"
                          bufrtest_specifying_subsets.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_lgycld_rrfs
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_lgycld_rrfs.yaml"
                          bufrtest_lgycld_rrfs.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_adpsfc_cloud_rrfs_prepbufr
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_adpsfc_cloud_rrfs_prepbufr.yaml"
                          bufrtest_adpsfc_cloud_rrfs_prepbufr.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_long_strs
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_long_strs.yaml"
                          bufrtest_long_strs.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_rtma_aircft
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_rtma_aircft.yaml"
                          bufrtest_rtma_aircft.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_rtma_aircft_NC004103
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_rtma_aircft_NC004103.yaml"
                          bufrtest_rtma_aircft_NC004103.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_wmo_amdar_multi
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_wmo_amdar_multi.yaml"
                          bufrtest_wmo_amdar_multi.nc)

ecbuild_add_test( TARGET  test_ioda_bufr_show_queries_ncep
                  TYPE    SCRIPT
                  COMMAND ${CMAKE_BINARY_DIR}/bin/show_queries.x
                  ARGS "Data/testinput_tier_1/gdas.t18z.1bmhs.tm00.bufr_d")

ecbuild_add_test( TARGET  test_ioda_bufr_show_queries_wmo
                  TYPE    SCRIPT
                  COMMAND ${CMAKE_BINARY_DIR}/bin/show_queries.x
                  ARGS -t "Data/testinput_tier_1/bufr_tables"
                          "Data/testinput_tier_1/amdar_wmo_multi.bufr")

ecbuild_add_test( TARGET  test_ioda_bufr_show_queries_help
                  TYPE    SCRIPT
                  COMMAND ${CMAKE_BINARY_DIR}/bin/show_queries.x
                  ARGS    -h)


if (${Python3_FOUND})

  ecbuild_add_test( TARGET  test_ioda_bufr_fieldname_validation
                    TYPE    SCRIPT
                    COMMAND python3
                    ARGS    testinput/bufrtest_fieldname_validation.py
                    ENVIRONMENT PYTHONPATH=${CMAKE_BINARY_DIR}/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}:$ENV{PYTHONPATH})

  ecbuild_add_test( TARGET  test_ioda_bufr_python_test
                    TYPE    SCRIPT
                    COMMAND python3
                    ARGS    testinput/bufrtest_python_test.py
                    ENVIRONMENT PYTHONPATH=${CMAKE_BINARY_DIR}/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}:$ENV{PYTHONPATH})

  ecbuild_add_test( TARGET  test_ioda_bufr_python_to_ioda
                    TYPE    SCRIPT
                    COMMAND bash
                    ARGS    ${CMAKE_BINARY_DIR}/bin/ioda_compare.sh
                            netcdf
                            "${CMAKE_BINARY_DIR}/bin/time_IodaIO.x testinput/bufrtest_python_to_ioda.yaml"
                            bufrtest_python_to_ioda.nc
                    ENVIRONMENT PYTHONPATH=${CMAKE_BINARY_DIR}/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}:$ENV{PYTHONPATH})

endif()