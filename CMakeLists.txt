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
IF (CMAKE_C_COMPILER MATCHES "clang")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -fno-omit-frame-pointer")
ELSE ()
  MESSAGE(INFO " No memory tests will be performed!")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ldl -coverage")
ENDIF ()

SET(
    KORO_SOURCE_FILES
    cli/info.c
    cli/db.c
    cli/db/table.c
    cli/db/init.c
)

SET(
    KORO_MIGRATOR_SOURCE_FILES
    koro_migrator/db/exec.c
    koro_migrator/migrate.c
    koro_migrator/create.c
)

SET(
    TEST_SOURCE_FILES
    ${KORO_SOURCE_FILES}
    ${KORO_MIGRATOR_SOURCE_FILES}
    test/support/ck_io.c
    test/support/prepare.c
    cli/db/init.c
    test/test_cli.c
    test/test_cli.h
    test/test_migrator.c
    test/test_migrator.h
    test/test_config.c test/test_config.h test/support/variables.h test/support/reset_environment.c)

ADD_EXECUTABLE(koro ${KORO_SOURCE_FILES} cli/main.c)
ADD_EXECUTABLE(koro-migrator ${KORO_MIGRATOR_SOURCE_FILES} koro_migrator/main.c)
ADD_EXECUTABLE(koro_test ${TEST_SOURCE_FILES} test/test.c)

ADD_LIBRARY(koroutils shared/utils.c)
ADD_LIBRARY(koroconfig shared/utils.c shared/config.c)

SET(
    LIBS
    -pthread
    ${CMAKE_THREAD_LIBS_INIT}
    ${PostgreSQL_LIBRARIES}
    ${LIBYAML_LIBRARIES}
    koroconfig
    koroutils
)

if (CMAKE_C_COMPILER MATCHES "gcc")
  INCLUDE(CodeCoverage)
  SETUP_TARGET_FOR_COVERAGE(koro_coverage koro_test coverage)
endif ()

FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dummy DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

TARGET_LINK_LIBRARIES(
    koroutils
    ${LIBYAML_LIBRARIES}
)

TARGET_LINK_LIBRARIES(
    koroconfig
    koroutils
    ${LIBYAML_LIBRARIES}
)

TARGET_LINK_LIBRARIES(
    koro
    ${LIBS}
)

TARGET_LINK_LIBRARIES(
    koro-migrator
    ${LIBS}
)

TARGET_LINK_LIBRARIES(
    koro_test
    ${CHECK_LIBRARIES}
    ${LIBS} -ldl gcov
)
