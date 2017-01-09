#include "./test_requester.h"
#include "./support/build_dummy_triggers.h"

START_TEST(test_noArgs)
  GO_TO_DUMMY
  int result;

  char *noArgs[1] = {"inline"};
  result = axon_runRequester(1, noArgs);
  ck_assert_int_eq(result, AXON_NOT_ENOUGH_ARGS);

  char *onlyTimestampArgs[2] = {"inline", "123"};
  result = axon_runRequester(2, onlyTimestampArgs);
  ck_assert_int_eq(result, AXON_NOT_ENOUGH_ARGS);

  char *wrongOrderArgs[3] = {"inline", "--dry", "1234"};
  result = axon_runRequester(3, wrongOrderArgs);
  ck_assert_int_eq(result, AXON_INVALID_ARG_TYPE);
END_TEST

START_TEST(test_validRun)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  int result;
  long long unsigned now = 123456789;
  char *timestamp = "123456789";
  char *fileName = "./db/migrate/123456789_create_posts.sql";
  char *beforeCallback = "./db/migrate/123456789_before_callback.c";
  char *afterCallback = "./db/migrate/123456789_after_callback.c";

  ck_unlink(beforeCallback);
  ck_unlink(afterCallback);

  ck_make_dummy_sql("create_posts", "CREATE TEMP TABLE posts (id serial);", now);

  char *noCallbacksArgs[3] = {"inline", timestamp, "--dry"};
  result = axon_runRequester(3, noCallbacksArgs);
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_axon_dummy_triggers(
      beforeCallback, NULL,
      afterCallback, NULL
  );

  char *execFileArgs[3] = {"inline", timestamp, fileName};
  result = axon_runRequester(3, execFileArgs);
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_axon_dummy_triggers(
      beforeCallback, "#include <axon/triggers.h>\n"
          "#include <axon/codes.h>\n"
          "#include <axon/utils.h>\n"
          "int before_123456789_callback(AxonCallbackData *data) {\n"
          "  printf(\"Hello before trigger\\n\");\n"
          "  return AXON_SUCCESS;\n"
          "}",
      afterCallback, NULL
  );

  char *beforeCallbackArgs[3] = {"inline", timestamp, fileName};
  ck_redirectStdout(result = axon_runRequester(3, beforeCallbackArgs));
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_path_contains(infoLogPath, "Hello before trigger\n");

  ck_axon_dummy_triggers(
      beforeCallback, NULL,
      afterCallback, "#include <axon/triggers.h>\n"
          "#include <axon/codes.h>\n"
          "#include <axon/utils.h>\n"
          "int after_123456789_callback(AxonCallbackData *data) {\n"
          "  printf(\"Hello after trigger\\n\");\n"
          "  return AXON_SUCCESS;\n"
          "}"
  );

  char *afterCallbackArgs[3] = {"inline", timestamp, fileName};
  ck_redirectStdout(result = axon_runRequester(3, afterCallbackArgs));
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_path_contains(infoLogPath, "Hello after trigger\n");

  ck_axon_dummy_triggers(
      beforeCallback, "#include <axon/triggers.h>\n"
          "#include <axon/codes.h>\n"
          "#include <axon/utils.h>\n"
          "int before_123456789_callback(AxonCallbackData *data) {\n"
          "  printf(\"Hello before trigger\\n\");\n"
          "  return AXON_SUCCESS;\n"
          "}",
      afterCallback, "#include <axon/triggers.h>\n"
          "#include <axon/codes.h>\n"
          "#include <axon/utils.h>\n"
          "int after_123456789_callback(AxonCallbackData *data) {\n"
          "  printf(\"Hello after trigger\\n\");\n"
          "  return AXON_SUCCESS;\n"
          "}"
  );

  char *bothCallbackArgs[3] = {"inline", timestamp, fileName};
  ck_redirectStdout(result = axon_runRequester(3, bothCallbackArgs));
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_path_contains(infoLogPath, "Hello before trigger\n");
  ck_path_contains(infoLogPath, "Hello after trigger\n");

  ck_axon_dummy_triggers(
      beforeCallback, "#include <axon/triggers.h>\n"
          "#include <axon/codes.h>\n"
          "#include <axon/utils.h>\n"
          "int before_123456789_callback(AxonCallbackData *data) {\n"
          "  data->sql = calloc(sizeof(char), strlen(\"CREATE TEMP TABLE replace(id serial);\") + 1);\n"
          "  strcpy(data->sql, \"CREATE TEMP TABLE replace(id serial);\");\n"
          "  return AXON_SUCCESS;\n"
          "}",
      afterCallback, NULL
  );

  char *replaceSqlArgs[3] = {"inline", timestamp, fileName};
  ck_redirectStdout(result = axon_runRequester(3, replaceSqlArgs));
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_axon_dummy_triggers(
      beforeCallback, "#include <axon/triggers.h>\n"
          "void freePayload(void *ptr) { printf(\"Hello free payload\\n\"); free(ptr); }\n"
          "#include <axon/codes.h>\n"
          "#include <axon/utils.h>\n"
          "int before_123456789_callback(AxonCallbackData *data) {\n"
          "  data->freePayload = freePayload;\n"
          "  data->payload = calloc(sizeof(char), strlen(\"CREATE TEMP TABLE replace(id serial);\") + 1);\n"
          "  strcpy(data->payload, \"CREATE TEMP TABLE replace(id serial);\");\n"
          "  return AXON_SUCCESS;\n"
          "}",
      afterCallback, NULL
  );

  char *payloadArgs[3] = {"inline", timestamp, fileName};
  ck_redirectStdout(result = axon_runRequester(3, payloadArgs));
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_path_contains(infoLogPath, "Hello free payload\n");
END_TEST

void test_requester(Suite *s) {
  TCase *testCaseRequester = tcase_create("Requester");
  tcase_add_test(testCaseRequester, test_noArgs);
  tcase_add_test(testCaseRequester, test_validRun);
  suite_add_tcase(s, testCaseRequester);
}