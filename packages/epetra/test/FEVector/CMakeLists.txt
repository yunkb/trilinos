INCLUDE(TribitsAddExecutableAndTest)

IF (WIN32 AND BUILD_SHARED_LIBS)
ELSE()
  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    FEVector_test
    SOURCES cxx_main.cpp ExecuteTestProblems.cpp
    DEPLIBS epetratest
    ARGS -v
    COMM serial mpi
    NUM_MPI_PROCS 1
    FAIL_REGULAR_EXPRESSION "tests FAILED"
    DEFINES -DEPETRA_LIB_EXPORTS_MODE
    )
  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    FEvector
    SOURCES fevector.cpp
    DEPLIBS epetratest
    ARGS -v
    COMM serial mpi
    NUM_MPI_PROCS 1
    FAIL_REGULAR_EXPRESSION "tests FAILED"
    DEFINES -DEPETRA_LIB_EXPORTS_MODE
    )
ENDIF()
