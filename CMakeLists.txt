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
    axon-creator/info.c
    axon-creator/write.c
    axon-creator/init.c
    axon-creator/column.c
    axon-creator/table.c
    axon-creator/enum.c
    axon-creator/creator.c
)

SET(
    AXON_MIGRATOR_SOURCE_FILES
    axon-migrator/info.c
    axon-migrator/migrate.c
    axon-migrator/create.c
    axon-migrator/drop.c
    axon-migrator/setup.c
    axon-migrator/seed.c
    axon-migrator/migrator.c
)

SET(
    AXON_COMPILER_SOURCE_FILES
    axon-compiler/compiler.c
)

SET(
    AXON_CONFIG_SOURCE_FILES
    lib/utils.c
    lib/database_config.c
    lib/order_config.c
    lib/triggers_config.c
    lib/config.c
)

SET(
    AXON_REQUESTER_SOURCE_FILES
    axon-requester/requester.c
)

SET(
    TEST_SOURCE_FILES
    ${AXON_SOURCE_FILES}
    ${AXON_CREATOR_SOURCE_FILES}
    ${AXON_MIGRATOR_SOURCE_FILES}
    ${AXON_COMPILER_SOURCE_FILES}
    ${AXON_CONFIG_SOURCE_FILES}
    ${AXON_REQUESTER_SOURCE_FILES}
    test/support/ck_io.c
    test/support/prepare.c
    test/support/build_dummy_triggers.c
    axon-creator/init.c
    test/test_cli.c
    test/test_migrator.c
    test/test_creator.c
    test/test_config.c
    test/test_utils.c
    test/test_compiler.c
    test/test_requester.c
    test/support/reset_environment.c
)

ADD_EXECUTABLE(axon ${AXON_SOURCE_FILES} cli/main.c)
ADD_EXECUTABLE(axon-migrator ${AXON_MIGRATOR_SOURCE_FILES} axon-migrator/main.c)
ADD_EXECUTABLE(axon-creator ${AXON_CREATOR_SOURCE_FILES} axon-creator/main.c)
ADD_EXECUTABLE(axon-compiler ${AXON_COMPILER_SOURCE_FILES} axon-compiler/main.c)
ADD_EXECUTABLE(axon-requester ${AXON_REQUESTER_SOURCE_FILES} axon-requester/main.c)

IF (CMAKE_BUILD_TYPE MATCHES "Debug")
  ADD_EXECUTABLE(axon_test ${TEST_SOURCE_FILES} test/test.c)
ENDIF ()

ADD_LIBRARY(axonutils SHARED lib/utils.c)
ADD_LIBRARY(axonconfig SHARED ${AXON_CONFIG_SOURCE_FILES})
ADD_LIBRARY(axonexec SHARED lib/exec/exec.c lib/exec/exec_sequence.c)

SET(
    LIBS
    -pthread
    ${CMAKE_THREAD_LIBS_INIT}
    ${PostgreSQL_LIBRARIES}
    ${LIBYAML_LIBRARIES}
    axonconfig
    axonutils
)

FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/dummy DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/includes DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

ADD_CUSTOM_COMMAND(
    TARGET axonutils
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:axonutils> ${CMAKE_CURRENT_BINARY_DIR}/lib
)

ADD_CUSTOM_COMMAND(
    TARGET axonconfig
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:axonconfig> ${CMAKE_CURRENT_BINARY_DIR}/lib
)

ADD_CUSTOM_COMMAND(
    TARGET axonexec
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:axonexec> ${CMAKE_CURRENT_BINARY_DIR}/lib
)

TARGET_LINK_LIBRARIES(axonutils ${LIBYAML_LIBRARIES})

TARGET_LINK_LIBRARIES(axonconfig ${LIBYAML_LIBRARIES})

TARGET_LINK_LIBRARIES(axonexec ${PostgreSQL_LIBRARIES})

TARGET_LINK_LIBRARIES(axon ${LIBS})

TARGET_LINK_LIBRARIES(axon-migrator axonexec ${LIBS})

TARGET_LINK_LIBRARIES(axon-creator ${LIBS})

TARGET_LINK_LIBRARIES(axon-compiler ${LIBS})

TARGET_LINK_LIBRARIES(axon-requester ${LIBS} axonexec dl)

SET_TARGET_PROPERTIES(axon-requester PROPERTIES COMPILE_OPTIONS -O0)
SET_TARGET_PROPERTIES(axon-requester PROPERTIES COMPILE_OPTIONS -g)
SET_TARGET_PROPERTIES(axon-requester PROPERTIES COMPILE_OPTIONS -fsanitize=address)
SET_TARGET_PROPERTIES(axon-requester PROPERTIES COMPILE_OPTIONS -fno-omit-frame-pointer)

INSTALL(
    TARGETS axon axon-migrator axon-creator axon-compiler axon-requester axonconfig axonutils axonexec
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)
INSTALL(DIRECTORY includes DESTINATION include FILES_MATCHING PATTERN "*.h")

IF (CMAKE_BUILD_TYPE MATCHES "Debug")
  IF (CMAKE_C_COMPILER MATCHES "gcc")
    INCLUDE(CodeCoverage)
    SETUP_TARGET_FOR_COVERAGE(axon_coverage axon_test coverage)
  ENDIF ()
  TARGET_LINK_LIBRARIES(axon_test ${CHECK_LIBRARIES} axonexec ${LIBS} dl gcov)
  ADD_TEST(
      NAME axon_test
      COMMAND ./axon_test && rm -Rf ./dummy/db
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
ENDIF ()
ADD_TEST(
    NAME shell_script
    COMMAND
    bash --login ${CMAKE_CURRENT_SOURCE_DIR}/cmake/test.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dummy
)
