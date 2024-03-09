# (C) Copyright 2024 NOAA/NCEP/EMC.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.


# Function definitions

# The list of all test input config files, which need to be symlink from the build folder.
# (These symlink are created at the end of this file, to give CMake functions setting up certain
# tests a chance to append items to this list).

list( APPEND test_input
  testinput/bufrtest_filtering_mapping.yaml
  testinput/bufrtest_split_mapping.yaml
  testinput/bufrtest_filter_split_mapping.yaml
  testinput/bufrtest_empty_fields_mapping.yaml
  testinput/bufrtest_simple_groupby_mapping.yaml
  testinput/bufrtest_read_2_dim_blocks_mapping.yaml
  testinput/bufrtest_mhs_basic_mapping.yaml
  testinput/bufrtest_hrs_basic_mapping.yaml
  testinput/bufrtest_adpupa_mapping.yaml
  testinput/bufrtest_amua_ta_mapping.yaml
  testinput/bufrtest_amua_n15_mapping.yaml
  testinput/bufrtest_esamua_mapping.yaml
  testinput/bufrtest_mhs_mapping.yaml
  testinput/bufrtest_esmhs_mapping.yaml
  testinput/bufrtest_adpsfc_mapping.yaml
  testinput/bufrtest_adpsfc_prepbufr_mapping.yaml
  testinput/bufrtest_adpsfc_snow_mapping.yaml
  testinput/bufrtest_aircar_mapping.yaml
  testinput/bufrtest_aircft_noamdar103_mapping.yaml
  testinput/bufrtest_aircft_amdar103_mapping.yaml
  testinput/bufrtest_aircft_prepbufr_mapping.yaml
  testinput/bufrtest_wmo_radiosonde_mapping.yaml
  testinput/bufrtest_satwnd_new_format_mapping.yaml
  testinput/bufrtest_satwnd_old_format_mapping.yaml
  testinput/bufrtest_satwnd_avhrr_mapping.yaml
  testinput/bufrtest_mtiasi_mapping.yaml
  testinput/bufrtest_atms_mapping.yaml
  testinput/bufrtest_sfcshp_mapping.yaml
  testinput/bufrtest_adpupa_prepbufr_mapping.yaml
  testinput/bufrtest_rtma_mesonet_mapping.yaml
  testinput/bufrtest_rtma_adpsfc_mapping.yaml
  testinput/bufrtest_sevcsr_mapping.yaml
  testinput/bufrtest_specific_subsets_by_query_mapping.yaml
  testinput/bufrtest_specifying_subsets_mapping.yaml
  testinput/bufrtest_lgycld_rrfs_mapping.yaml
  testinput/bufrtest_adpsfc_cloud_rrfs_prepbufr_mapping.yaml
  testinput/bufrtest_long_strs_mapping.yaml
  testinput/bufrtest_rtma_aircft_mapping.yaml
  testinput/bufrtest_rtma_aircft_NC004103_mapping.yaml
  testinput/bufrtest_wmo_amdar_multi_mapping.yaml
  testinput/bufrtest_fieldname_validation.py
  testinput/bufrtest_python_test.py
  testinput/bufrtest_python_to_ioda.py
)

list(APPEND test_output
  testoutput/bufrtest_adpsfc.nc
  testoutput/bufrtest_adpupa.nc
  testoutput/bufrtest_aircft_noamdar103.nc
  testoutput/bufrtest_atms.nc
  testoutput/bufrtest_filter_split.nc
  testoutput/bufrtest_long_strs.nc
  testoutput/bufrtest_mtiasi_metop_a.nc
  testoutput/bufrtest_read_2_dim_blocks.nc
  testoutput/bufrtest_rtma_mesonet.nc
  testoutput/bufrtest_sevcsr.nc
  testoutput/bufrtest_split.nc
  testoutput/bufrtest_adpsfc_cloud_rrfs_prepbufr.nc
  testoutput/bufrtest_adpupa_prepbufr.nc
  testoutput/bufrtest_aircft_prepbufr.nc
  testoutput/bufrtest_empty_fields.nc
  testoutput/bufrtest_filtering.nc
  testoutput/bufrtest_mhs.nc
  testoutput/bufrtest_mtiasi_metop_b.nc
  testoutput/bufrtest_rtma_adpsfc.nc
  testoutput/bufrtest_satwnd_avhrr.nc
  testoutput/bufrtest_sfcshp.nc
  testoutput/bufrtest_wmo_amdar_multi.nc
  testoutput/bufrtest_adpsfc_prepbufr.nc
  testoutput/bufrtest_aircar.nc
  testoutput/bufrtest_amua_n15.nc
  testoutput/bufrtest_esamua.nc
  testoutput/bufrtest_hrs_basic.nc
  testoutput/bufrtest_mhs_basic.nc
  testoutput/bufrtest_mtiasi_metop_c.nc
  testoutput/bufrtest_rtma_aircft.nc
  testoutput/bufrtest_satwnd_new_format.nc
  testoutput/bufrtest_simple_groupby.nc
  testoutput/bufrtest_wmo_radiosonde.nc
  testoutput/bufrtest_adpsfc_snow.nc
  testoutput/bufrtest_aircft_amdar103.nc
  testoutput/bufrtest_amua_ta.nc
  testoutput/bufrtest_esmhs.nc
  testoutput/bufrtest_lgycld_rrfs.nc
  testoutput/bufrtest_mtiasi.nc
  testoutput/bufrtest_python_to_ioda.nc
  testoutput/bufrtest_rtma_aircft_NC004103.nc
  testoutput/bufrtest_satwnd_old_format.nc
  testoutput/bufrtest_specifying_subsets.nc
)

list(APPEND test_data
  testinput/data/amdar_wmo_multi.bufr
  testinput/data/bufr_adpupa_prepbufr.bufr
  testinput/data/bufr_empty_fields.bufr_d
  testinput/data/bufr_read_2_dim_blocks.bufr
  testinput/data/bufr_read_wmo_radiosonde.bufr
  testinput/data/bufr_satwnd_new_format.bufr
  testinput/data/bufr_satwnd_old_format.bufr
  testinput/data/bufr_sfcshp.bufr
  testinput/data/bufr_simple_groupby.bufr
  testinput/data/gdas.t00z.1bhrs4.tm00.bufr_d
  testinput/data/gdas.t00z.atms.tm00.bufr_d
  testinput/data/gdas.t00z.sevcsr.tm00.bufr_d
  testinput/data/gdas.t06z.adpsfc.tm00.bufr_d
  testinput/data/gdas.t06z.snocvr.tm00.bufr_d
  testinput/data/gdas.t12z.1bamua.tm00.bufr_d
  testinput/data/gdas.t12z.adpsfc.tm00.bufr_d
  testinput/data/gdas.t12z.adpupa.tm00.bufr_d
  testinput/data/gdas.t12z.aircar.tm00.bufr_d
  testinput/data/gdas.t12z.aircft.tm00.bufr_d
  testinput/data/gdas.t12z.esamua.tm00.bufr_d
  testinput/data/gdas.t12z.esmhs.tm00.bufr_d
  testinput/data/gdas.t12z.mtiasi.tm00.bufr_d
  testinput/data/gdas.t18z.1bmhs.tm00.bufr_d
  testinput/data/gdas.t18z.satwnd_avhrr.tm00.bufr_d
  testinput/data/rap.t06z.adpsfc.prepbufr.tm00.bufr
  testinput/data/rap.t06z.lgycld.tm00.bufr_d
  testinput/data/rtma_ru.t0000z.adpsfc_nc000101.tm00.bufr_d
  testinput/data/rtma_ru.t0000z.aircft.tm00_nc004006.bufr_d
  testinput/data/rtma_ru.t0000z.aircft.tm00_nc004103.bufr_d
  testinput/data/rtma_ru.t00z.msonet.tm00.bufr_d
)

# create test directories and make links to the input files
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testinput)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testinput/data)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testoutput)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/testrun)
foreach(FILENAME ${test_input} ${test_output} ${test_data})
  execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}
    ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME} )
endforeach(FILENAME)

ecbuild_add_test( TARGET  test_bufr_filtering
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t18z.1bmhs.tm00.bufr_d
                                                               testinput/bufrtest_filtering_mapping.yaml
                                                               testrun/bufrtest_filtering.nc"
                          bufrtest_filtering.nc)

ecbuild_add_test( TARGET  test_bufr_split
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t18z.1bmhs.tm00.bufr_d
                                                               testinput/bufrtest_split_mapping.yaml
                                                               testrun/bufrtest_split.nc"
                          bufrtest_split.nc)

ecbuild_add_test( TARGET  test_bufr_filter_split
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t18z.1bmhs.tm00.bufr_d
                                                               testinput/bufrtest_filter_split_mapping.yaml
                                                               testrun/bufrtest_filter_split.nc"
                          bufrtest_filter_split.nc)

ecbuild_add_test( TARGET  test_bufr_empty_fields
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/bufr_empty_fields.bufr_d
                                                               testinput/bufrtest_empty_fields_mapping.yaml
                                                               testrun/bufrtest_empty_fields.nc"
                          bufrtest_empty_fields.nc)


ecbuild_add_test( TARGET  test_bufr_simple_groupby
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/bufr_simple_groupby.bufr
                                                               testinput/bufrtest_simple_groupby_mapping.yaml
                                                               testrun/bufrtest_simple_groupby.nc"
                          bufrtest_simple_groupby.nc)

ecbuild_add_test( TARGET  test_bufr_read_2_dim_blocks
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/bufr_read_2_dim_blocks.bufr
                                                               testinput/bufrtest_read_2_dim_blocks_mapping.yaml
                                                               testrun/bufrtest_read_2_dim_blocks.nc"
                          bufrtest_read_2_dim_blocks.nc)

ecbuild_add_test( TARGET  test_bufr_mhs_basic
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t18z.1bmhs.tm00.bufr_d
                                                               testinput/bufrtest_mhs_basic_mapping.yaml
                                                               testrun/bufrtest_mhs_basic.nc"
                          bufrtest_mhs_basic.nc)

ecbuild_add_test( TARGET  test_bufr_hrs_basic
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t00z.1bhrs4.tm00.bufr_d
                                                               testinput/bufrtest_hrs_basic_mapping.yaml
                                                               testrun/bufrtest_hrs_basic.nc"
                          bufrtest_hrs_basic.nc)

ecbuild_add_test( TARGET  test_bufr_adpupa
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.adpupa.tm00.bufr_d
                                                               testinput/bufrtest_adpupa_mapping.yaml
                                                               testrun/bufrtest_adpupa.nc"
                          bufrtest_adpupa.nc)

ecbuild_add_test( TARGET  test_bufr_amua_ta
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.1bamua.tm00.bufr_d
                                                               testinput/bufrtest_amua_ta_mapping.yaml
                                                               testrun/bufrtest_amua_ta.nc"
                          bufrtest_amua_ta.nc)

ecbuild_add_test( TARGET  test_bufr_amua_n15
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.1bamua.tm00.bufr_d
                                                               testinput/bufrtest_amua_n15_mapping.yaml
                                                               testrun/bufrtest_amua_n15.nc"
                          bufrtest_amua_n15.nc)

ecbuild_add_test( TARGET  test_bufr_esamua
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.esamua.tm00.bufr_d
                                                               testinput/bufrtest_esamua_mapping.yaml
                                                               testrun/bufrtest_esamua.nc"
                          bufrtest_esamua.nc)

ecbuild_add_test( TARGET  test_bufr_mhs
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t18z.1bmhs.tm00.bufr_d
                                                               testinput/bufrtest_mhs_mapping.yaml
                                                               testrun/bufrtest_mhs.nc"
                          bufrtest_mhs.nc)

ecbuild_add_test( TARGET  test_bufr_esmhs
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.esmhs.tm00.bufr_d
                                                               testinput/bufrtest_esmhs_mapping.yaml
                                                               testrun/bufrtest_esmhs.nc"
                          bufrtest_esmhs.nc)

ecbuild_add_test( TARGET  test_bufr_adpsfc
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/rtma_ru.t0000z.adpsfc_nc000101.tm00.bufr_d
                                                               testinput/bufrtest_adpsfc_mapping.yaml
                                                               testrun/bufrtest_adpsfc.nc"
                          bufrtest_adpsfc.nc)

ecbuild_add_test( TARGET  test_bufr_adpsfc_prepbufr
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.adpsfc.prepbufr
                                                               testinput/bufrtest_adpsfc_prepbufr_mapping.yaml
                                                               testrun/bufrtest_adpsfc_prepbufr.nc"
                          bufrtest_adpsfc_prepbufr.nc)

ecbuild_add_test( TARGET  test_bufr_adpsfc_snow
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t06z.adpsfc.tm00.bufr_d
                                                               testinput/bufrtest_adpsfc_snow_mapping.yaml
                                                               testrun/bufrtest_adpsfc_snow.nc"
                          bufrtest_adpsfc_snow.nc)

ecbuild_add_test( TARGET  test_bufr_aircar
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.aircar.tm00.bufr_d
                                                               testinput/bufrtest_aircar_mapping.yaml
                                                               testrun/bufrtest_aircar.nc"
                          bufrtest_aircar.nc)

ecbuild_add_test( TARGET  test_bufr_aircft_noamdar103
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.aircft.tm00.bufr_d
                                                               testinput/bufrtest_aircft_noamdar103_mapping.yaml
                                                               testrun/bufrtest_aircft_noamdar103.nc"
                          bufrtest_aircft_noamdar103.nc)

ecbuild_add_test( TARGET  test_bufr_aircft_amdar103
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.aircft.tm00.bufr_d
                                                               testinput/bufrtest_aircft_amdar103_mapping.yaml
                                                               testrun/bufrtest_aircft_amdar103.nc"
                          bufrtest_aircft_amdar103.nc)

ecbuild_add_test( TARGET  test_bufr_aircft_prepbufr
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.acft_profiles.prepbufr
                                                               testinput/bufrtest_aircft_prepbufr_mapping.yaml
                                                               testrun/bufrtest_aircft_prepbufr.nc"
                          bufrtest_aircft_prepbufr.nc)

ecbuild_add_test( TARGET  test_bufr_wmo_radiosonde
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/bufr_read_wmo_radiosonde.bufr
                                                               testinput/bufrtest_wmo_radiosonde_mapping.yaml
                                                               testrun/bufrtest_wmo_radiosonde.nc"
                          bufrtest_wmo_radiosonde.nc)

ecbuild_add_test( TARGET  test_bufr_satwnd_old_format
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/bufr_satwnd_old_format.bufr
                                                               testinput/bufrtest_satwnd_old_format_mapping.yaml
                                                               testrun/bufrtest_satwnd_old_format.nc"
                          bufrtest_satwnd_old_format.nc)

ecbuild_add_test( TARGET  test_bufr_satwnd_new_format
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/bufr_satwnd_new_format.bufr
                                                               testinput/bufrtest_satwnd_new_format_mapping.yaml
                                                               testrun/bufrtest_satwnd_new_format.nc"
                          bufrtest_satwnd_new_format.nc)

ecbuild_add_test( TARGET  test_bufr_satwnd_avhrr
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t18z.satwnd_avhrr.tm00.bufr_d
                                                               testinput/bufrtest_satwnd_avhrr_mapping.yaml
                                                               testrun/bufrtest_satwnd_avhrr.nc"
                          bufrtest_satwnd_avhrr.nc)

ecbuild_add_test( TARGET  test_bufr_mtiasi
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.mtiasi.tm00.bufr_d
                                                               testinput/bufrtest_mtiasi_mapping.yaml
                                                               testrun/bufrtest_mtiasi.nc"
                          bufrtest_mtiasi.nc)

ecbuild_add_test( TARGET  test_bufr_mtiasi_cached
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.mtiasi.tm00.bufr_d
                                                               testinput/bufrtest_mtiasi_cached_mapping.yaml
                                                               testrun/bufrtest_mtiasi_cached.nc"
                          bufrtest_mtiasi_metop_b.nc)

ecbuild_add_test( TARGET  test_bufr_atms
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t00z.atms.tm00.bufr_d
                                                               testinput/bufrtest_atms_mapping.yaml
                                                               testrun/bufrtest_atms.nc"
                          bufrtest_atms.nc)

ecbuild_add_test( TARGET  test_bufr_sfcshp
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/bufr_sfcshp.bufr
                                                               testinput/bufrtest_sfcshp_mapping.yaml
                                                               testrun/bufrtest_sfcshp.nc"
                          bufrtest_sfcshp.nc)

ecbuild_add_test( TARGET  test_bufr_adpupa_prepbufr
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/bufr_adpupa_prepbufr.bufr
                                                               testinput/bufrtest_adpupa_prepbufr_mapping.yaml
                                                               testrun/bufrtest_adpupa_prepbufr.nc"
                          bufrtest_adpupa_prepbufr.nc)

ecbuild_add_test( TARGET  test_bufr_rtma_mesonet
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/rtma_ru.t00z.msonet.tm00.bufr_d
                                                               testinput/bufrtest_rtma_mesonet_mapping.yaml
                                                               testrun/bufrtest_rtma_mesonet.nc"
                          bufrtest_rtma_mesonet.nc)

ecbuild_add_test( TARGET  test_bufr_rtma_adpsfc
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/rtma_ru.t0000z.adpsfc_nc000101.tm00.bufr_d
                                                               testinput/bufrtest_rtma_adpsfc_mapping.yaml
                                                               testrun/bufrtest_rtma_adpsfc.nc"
                          bufrtest_rtma_adpsfc.nc)

ecbuild_add_test( TARGET  test_bufr_sevcsr
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t00z.sevcsr.tm00.bufr_d
                                                               testinput/bufrtest_sevcsr_mapping.yaml
                                                               testrun/bufrtest_sevcsr.nc"
                          bufrtest_sevcsr.nc)

ecbuild_add_test( TARGET  test_bufr_specific_subsets_by_query
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.aircft.tm00.bufr_d
                                                               testinput/bufrtest_specific_subsets_by_query_mapping.yaml
                                                               testrun/bufrtest_specific_subsets_by_query.nc"
                          bufrtest_specifying_subsets.nc)

ecbuild_add_test( TARGET  test_bufr_specifying_subsets
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t12z.aircft.tm00.bufr_d
                                                               testinput/bufrtest_specifying_subsets_mapping.yaml
                                                               testrun/bufrtest_specifying_subsets.nc"
                          bufrtest_specifying_subsets.nc)

ecbuild_add_test( TARGET  test_bufr_lgycld_rrfs
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/rap.t06z.lgycld.tm00.bufr_d
                                                               testinput/bufrtest_lgycld_rrfs_mapping.yaml
                                                               testrun/bufrtest_lgycld_rrfs.nc"
                          bufrtest_lgycld_rrfs.nc)

ecbuild_add_test( TARGET  test_bufr_adpsfc_cloud_rrfs_prepbufr
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/rap.t06z.adpsfc.prepbufr.tm00.bufr
                                                               testinput/bufrtest_adpsfc_cloud__mapping.yaml
                                                               testrun/bufrtest_adpsfc_cloud_rrfs_prepbufr.nc"
                          bufrtest_adpsfc_cloud_rrfs_prepbufr.nc)

ecbuild_add_test( TARGET  test_bufr_long_strs
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/gdas.t06z.snocvr.tm00.bufr_d
                                                               testinput/bufrtest_long_strs_mapping.yaml
                                                               testrun/bufrtest_long_strs.nc"
                          bufrtest_long_strs.nc)

ecbuild_add_test( TARGET  test_bufr_rtma_aircft
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/rtma_ru.t0000z.aircft.tm00_nc004006.bufr_d
                                                               testinput/bufrtest_rtma_aircft_mapping.yaml
                                                               testrun/bufrtest_rtma_aircft.nc"
                          bufrtest_rtma_aircft.nc)

ecbuild_add_test( TARGET  test_bufr_rtma_aircft_NC004103
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/rtma_ru.t0000z.aircft.tm00_nc004103.bufr_d
                                                               testinput/bufrtest_rtma_aircft_NC004103_mapping.yaml
                                                               testrun/bufrtest_rtma_aircft_NC004103.nc"
                          bufrtest_rtma_aircft_NC004103.nc)

ecbuild_add_test( TARGET  test_bufr_wmo_amdar_multi
                  TYPE    SCRIPT
                  COMMAND bash
                  ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                          netcdf
                          "${CMAKE_BINARY_DIR}/bin/bufr2ioda.x testinput/data/amdar_wmo_multi.bufr
                                                               testinput/bufrtest_wmo_amdar_multi_mapping.yaml
                                                               testrun/bufrtest_wmo_amdar_multi.nc"
                          bufrtest_wmo_amdar_multi.nc)

ecbuild_add_test( TARGET  test_bufr_show_queries_ncep
                  TYPE    SCRIPT
                  COMMAND ${CMAKE_BINARY_DIR}/bin/show_queries.x
                  ARGS "Data/testinput_tier_1/gdas.t18z.1bmhs.tm00.bufr_d")

ecbuild_add_test( TARGET  test_bufr_show_queries_wmo
                  TYPE    SCRIPT
                  COMMAND ${CMAKE_BINARY_DIR}/bin/show_queries.x
                  ARGS -t "Data/testinput_tier_1/bufr_tables"
                          "Data/testinput_tier_1/amdar_wmo_multi.bufr")

ecbuild_add_test( TARGET  test_bufr_show_queries_help
                  TYPE    SCRIPT
                  COMMAND ${CMAKE_BINARY_DIR}/bin/show_queries.x
                  ARGS    -h)


if (${BUILD_PYTHON_BINDINGS})

  ecbuild_add_test( TARGET  test_bufr_fieldname_validation
                    TYPE    SCRIPT
                    COMMAND python3
                    ARGS    testinput/bufrtest_fieldname_validation.py
                    ENVIRONMENT PYTHONPATH=${CMAKE_BINARY_DIR}/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}:$ENV{PYTHONPATH})

  ecbuild_add_test( TARGET  test_bufr_python_test
                    TYPE    SCRIPT
                    COMMAND python3
                    ARGS    testinput/bufrtest_python_test.py
                    ENVIRONMENT PYTHONPATH=${CMAKE_BINARY_DIR}/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}:$ENV{PYTHONPATH})

  ecbuild_add_test( TARGET  test_bufr_python_to_ioda
                    TYPE    SCRIPT
                    COMMAND bash
                    ARGS    ${CMAKE_BINARY_DIR}/bin/bufr_comp.sh
                            netcdf
                            "python3 testinput/bufrtest_python_to_ioda.py"
                            bufrtest_python_to_ioda.nc
                    ENVIRONMENT PYTHONPATH=${CMAKE_BINARY_DIR}/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}:$ENV{PYTHONPATH})

endif()