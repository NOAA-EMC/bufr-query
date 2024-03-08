

list(APPEND _scripts
  bufr_comp.sh
  bufr_cpp_lint.py
  bufr_py_lint.sh)

foreach(_f IN LISTS _scripts)
  execute_process( COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/${_f} ${CMAKE_BINARY_DIR}/bin/${_f} )
endforeach()
