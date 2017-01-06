CMAKE_MINIMUM_REQUIRED(VERSION 3.6)

SET(CMAKE_C_STANDARD 11)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

FIND_PACKAGE(Check REQUIRED)
FIND_PACKAGE(Threads)
FIND_PACKAGE(YAJL REQUIRED)
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
INCLUDE_DIRECTORIES(${YAJL_INCLUDE_DIR})
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
    cli/db/init.c
)

SET(
    LIBS
    -pthread
    ${CMAKE_THREAD_LIBS_INIT}
    ${${YAJL_LIBRARY}}
    ${PostgreSQL_LIBRARIES}
    ${LIBYAML_LIBRARIES}
)

ADD_EXECUTABLE(koro ${KORO_SOURCE_FILES} cli/main.c)
ADD_EXECUTABLE(koro-migrator ${KORO_MIGRATOR_SOURCE_FILES} koro_migrator/main.c)
ADD_EXECUTABLE(koro_test ${TEST_SOURCE_FILES} test/test.c)

ADD_LIBRARY(koroconfig shared/config.c)
ADD_LIBRARY(koroutils shared/utils.c)

if (CMAKE_C_COMPILER MATCHES "gcc")
  INCLUDE(CodeCoverage)
  SETUP_TARGET_FOR_COVERAGE(koro_coverage koro_test coverage)
endif ()

FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dummy DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

TARGET_LINK_LIBRARIES(
    koro
    koroconfig
    koroutils
    ${LIBS}
)

TARGET_LINK_LIBRARIES(
    koro-migrator
    koroconfig
    koroutils
    ${LIBS}
)

TARGET_LINK_LIBRARIES(
    koro_test
    koroconfig
    koroutils
    ${CHECK_LIBRARIES}
    ${LIBS} -ldl gcov
)

ADD_TEST(
    NAME koro_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dummy
    COMMAND koro_test
)

ADD_TEST(
    NAME koro_info
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dummy
    COMMAND koro info
)

ADD_TEST(
    NAME koro_db_init
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dummy
    COMMAND koro cli/db init
)

ADD_TEST(
    NAME koro_db_create
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dummy
    COMMAND koro cli/db create
)

ADD_TEST(
    NAME koro_db_new_table_accounts
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dummy
    COMMAND koro cli/db new table accounts id:serial login timestamps
)
