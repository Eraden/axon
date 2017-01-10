CMAKE_MINIMUM_REQUIRED(VERSION 3.6)

SET(CMAKE_C_STANDARD 11)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

FIND_PACKAGE(Check REQUIRED)
FIND_PACKAGE(Threads)
FIND_PACKAGE(LibYAML REQUIRED)
FIND_PACKAGE(PostgreSQL REQUIRED)

ENABLE_TESTING()

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
    /usr/include
    /usr/local/include
    /usr/include/postgresql
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/kore/includes
)

INCLUDE_DIRECTORIES(/usr/include /usr/local/include /usr/include/postgresql)
INCLUDE_DIRECTORIES(${CHECK_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${PostgreSQL_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${LIBYAML_INCLUDE_DIR})

ADD_DEFINITIONS(-Wall)
IF (CMAKE_BUILD_TYPE MATCHES "Debug")
  ADD_DEFINITIONS(-Wall -DAXON_LOCAL_TEST)
  IF (CMAKE_C_COMPILER MATCHES "clang")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -fno-omit-frame-pointer")
  ELSE ()
    MESSAGE(INFO " No memory tests will be performed!")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ldl -coverage")
  ENDIF ()
ENDIF ()

SET(
    AXON_SOURCE_FILES
    cli/info.c
    cli/db.c
    cli/cli.c
)

SET(
    AXON_CREATOR_SOURCE_FILES
    axon_creator/info.c
    axon_creator/write.c
    axon_creator/init.c
    axon_creator/column.c
    axon_creator/table.c
    axon_creator/enum.c
    axon_creator/creator.c
)

SET(
    AXON_MIGRATOR_SOURCE_FILES
    axon_migrator/db/exec.c
    axon_migrator/db/exec_sequence.c
    axon_migrator/info.c
    axon_migrator/migrate.c
    axon_migrator/create.c
    axon_migrator/drop.c
    axon_migrator/setup.c
    axon_migrator/migrator.c
)

SET(
    AXON_COMPILER_SOURCE_FILES
    axon_compiler/compiler.c
)

SET(
    TEST_SOURCE_FILES
    ${AXON_SOURCE_FILES}
    ${AXON_CREATOR_SOURCE_FILES}
    ${AXON_MIGRATOR_SOURCE_FILES}
    ${AXON_COMPILER_SOURCE_FILES}
    test/support/ck_io.c
    test/support/prepare.c
    axon_creator/init.c
    test/test_cli.c
    test/test_migrator.c
    test/test_creator.c
    test/test_config.c
    test/test_utils.c
    test/test_compiler.c
    test/support/reset_environment.c
)

ADD_EXECUTABLE(axon ${AXON_SOURCE_FILES} cli/main.c)
ADD_EXECUTABLE(axon-migrator ${AXON_MIGRATOR_SOURCE_FILES} axon_migrator/main.c)
ADD_EXECUTABLE(axon-creator ${AXON_CREATOR_SOURCE_FILES} axon_creator/main.c)
ADD_EXECUTABLE(axon-compiler ${AXON_COMPILER_SOURCE_FILES} axon_compiler/main.c)
ADD_EXECUTABLE(axon_test ${TEST_SOURCE_FILES} test/test.c)

ADD_LIBRARY(axonutils lib/utils.c)
ADD_LIBRARY(axonconfig lib/utils.c lib/config.c)

SET(
    LIBS
    -pthread
    ${CMAKE_THREAD_LIBS_INIT}
    ${PostgreSQL_LIBRARIES}
    ${LIBYAML_LIBRARIES}
    axonconfig
    axonutils
)

if (CMAKE_C_COMPILER MATCHES "gcc")
  INCLUDE(CodeCoverage)
  SETUP_TARGET_FOR_COVERAGE(axon_coverage axon_test coverage)
endif ()

FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dummy DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

TARGET_LINK_LIBRARIES(
    axonutils
    ${LIBYAML_LIBRARIES}
)

TARGET_LINK_LIBRARIES(
    axonconfig
    axonutils
    ${LIBYAML_LIBRARIES}
)

TARGET_LINK_LIBRARIES(
    axon
    ${LIBS}
)

TARGET_LINK_LIBRARIES(
    axon-migrator
    ${LIBS}
)

TARGET_LINK_LIBRARIES(
    axon-creator
    ${LIBS}
)

TARGET_LINK_LIBRARIES(
    axon-compiler
    ${LIBS}
)

TARGET_LINK_LIBRARIES(
    axon_test
    ${CHECK_LIBRARIES}
    ${LIBS} -ldl gcov
)

INSTALL(
    TARGETS axon axon-migrator axon-creator axonconfig axonutils
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)
INSTALL(DIRECTORY includes DESTINATION include FILES_MATCHING PATTERN "*.h")
