  
INCLUDE("${CTEST_SCRIPT_DIRECTORY}/TrilinosCTestDriverCore.cmake")

#
# Platform/compiler specific options for godel using gcc
#

MACRO(TRILINOS_SYSTEM_SPECIFIC_CTEST_DRIVER)

  # Base of Trilinos/cmake/ctest then BUILD_DIR_NAME

##need to ask Ross about this
  SET( CTEST_DASHBOARD_ROOT "${CTEST_SCRIPT_DIRECTORY}/../../../${BUILD_DIR_NAME}" )

  SET( CTEST_NOTES_FILES "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}" )
  
  SET( CTEST_CVS_COMMAND_ARGS "cvs -q -z3" )
  
  SET( CTEST_BUILD_FLAGS "-j2 -i" )

  SET( CTEST_MEMORYCHECK_COMMAND /usr/bin/valgrind )
  #SET( CTEST_MEMORYCHECK_COMMAND_OPTIONS )
  
  SET( EXTRA_SYSTEM_CONFIGURE_OPTIONS
    "-DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}"
    "-DTrilinos_ENABLE_DEPENCENCY_UNIT_TESTS:BOOL=OFF"
#    "-DMEMORYCHECK_COMMAND:FILEPATH=/usr/bin/valgrind"
    )
  
  IF (COMM_TYPE STREQUAL MPI)
  
    SET( EXTRA_SYSTEM_CONFIGURE_OPTIONS
      ${EXTRA_SYSTEM_CONFIGURE_OPTIONS}
      "-DTPL_ENABLE_MPI:BOOL=ON"
      "-DMPI_BASE_DIR:PATH=/Users/bmpersc/bin/mpich2-1.0.8"
      )
  
  ELSE()
  
    SET( EXTRA_SYSTEM_CONFIGURE_OPTIONS
      ${EXTRA_SYSTEM_CONFIGURE_OPTIONS}
      "-DCMAKE_CXX_COMPILER:FILEPATH=/Users/bmpersc/bin/gcc-4.4/bin/g++"
      "-DCMAKE_C_COMPILER:FILEPATH=/Users/bmpersc/bin/gcc-4.4/bin/gcc"
      "-DCMAKE_Fortran_COMPILER:FILEPATH=/Users/bmpersc/bin/gcc-4.4/bin/gfortran"
      )
  
  ENDIF()

  TRILINOS_CTEST_DRIVER()

ENDMACRO()
