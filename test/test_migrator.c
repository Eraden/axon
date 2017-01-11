#include "test_migrator.h"

START_TEST(test_noArgs)
  GO_TO_DUMMY
  int result = 0;
  char *args[1] = {"inline"};
  ck_redirectStdout(result = axon_runMigrator(1, args);)
  ck_assert_int_eq(result, AXON_OPERATION_REQUIRED);
END_TEST

START_TEST(test_unknownOperation)
  GO_TO_DUMMY
  int result = 0;
  char *args[2] = {"inline", "foo"};
  ck_redirectStdout(result = axon_runMigrator(2, args);)
  ck_assert_int_eq(result, AXON_UNKNOWN_COMMAND);
END_TEST

START_TEST(test_migratorMigrate)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)

  ck_make_dummy_sql("create_examples", "CREATE TABLE examples ( id serial );", NOW() + 1);
  ck_make_dummy_sql("change_examples", "ALTER TABLE examples ADD COLUMN login varchar;", NOW() + 2);
  ck_make_dummy_sql("change_examples", "ALTER TABLE examples DROP COLUMN login;", NOW() + 3);
  ck_make_dummy_sql("change_examples", "ALTER TABLE examples ADD COLUMN created_at timestamp;", NOW() + 4);

  char *args[3] = {"inline", "migrate", "--skip-triggers"};
  ck_assert_int_eq(axon_migrate(3, args), AXON_SUCCESS);
END_TEST

START_TEST(test_migratorCreateDatabase)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  int result;
  ck_dropTestDb();
  ck_redirectStderr(result = axon_createDatabase();)
  ck_assert_int_eq(result, AXON_SUCCESS);
END_TEST

START_TEST(test_migratorCreateDatabaseWithoutConfig)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  ck_dropTestDb();
  ck_unlink(AXON_DATABASE_CONFIG_FILE);
  int result;
  ck_redirectStderr(
      result = axon_createDatabase();
  )
  ck_assert_int_eq(result, AXON_CONFIG_MISSING);
END_TEST

START_TEST(test_migratorDropDatabaseWithoutConfig)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  ck_dropTestDb();
  ck_unlink(AXON_DATABASE_CONFIG_FILE);
  int result;
  ck_redirectStderr(
      result = axon_dropDatabase();
  )
  ck_assert_int_eq(result, AXON_CONFIG_MISSING);
END_TEST

START_TEST(test_getContextNoConfig)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */);
  putenv("KORE_ENV=invalid-env");
  ck_unlink(AXON_DATABASE_CONFIG_FILE);
  char *info = NULL;
  ck_redirectStderr(axon_getConnectionInfo();)
  ck_assert_ptr_eq(info, NULL);
  putenv("KORE_ENV=test");
  ck_path_contains("./log/error.log", "No database config file for current env!");
END_TEST

START_TEST(test_getContextNoDbName)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */);
  ck_overrideFile(AXON_DATABASE_CONFIG_FILE, "test:\n  host: localhost\n");
  char *info = NULL;
  ck_redirectStderr(axon_getConnectionInfo();)
  ck_assert_ptr_eq(info, NULL);
  ck_unlink("./conf/database.yml");
  IN_CLEAR_STATE(/* */);
END_TEST

START_TEST(test_migrateWithCallbacks)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  ck_writeTestTriggersConfig();
  int result;
  long long unsigned now = 123456789;
  char *beforeCallback = "./db/migrate/123456789_before_callback.c";
  char *afterCallback = "./db/migrate/123456789_after_callback.c";

  ck_make_dummy_sql("create_posts", "CREATE TEMP TABLE posts (id serial);", now);

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

  char *args[2] = {"inline", "migrate"};
  result = axon_runMigrator(2, args);
  ck_assert_int_eq(result, AXON_SUCCESS);
END_TEST

START_TEST(test_psqlExecNoConnInfo)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */);
  AxonExecContext context = axon_getContext("", NULL, AXON_ONLY_QUERY);
  int result = axon_psqlExecute(&context);
  ck_assert_int_eq(result, AXON_CONFIG_MISSING);
  if (context.error) {
    free(context.error);
  }
END_TEST

START_TEST(test_psqlExecInvalidConnInfo)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */);
  AxonExecContext context = axon_getContext("", "dbname = 1234567890asdfghjkl", AXON_ONLY_QUERY);
  int result;
  ck_redirectStderr(result = axon_psqlExecute(&context);)
  ck_assert_int_eq(result, AXON_FAILURE);
  if (context.error) {
    free(context.error);
  }
END_TEST

START_TEST(test_queryInTransaction)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  AxonExecContext context;
  context = axon_getContext(
      "CREATE TABLE accounts(id serial, login text)",
      "dbname = kore_test",
      AXON_ONLY_QUERY
  );
  axon_psqlExecute(&context);
  if (context.error) free(context.error);

  context.sql = "INSERT INTO accounts(login) VALUES ('hello'), ('world')";
  axon_psqlExecute(&context);
  if (context.error) free(context.error);

  context.sql = "SELECT id FROM accounts";
  context.type = AXON_ONLY_QUERY | AXON_TRANSACTION_QUERY | AXON_USE_CURSOR_QUERY;
  int result = axon_psqlExecute(&context);
  if (context.error) free(context.error);

  ck_assert_int_eq(result, AXON_SUCCESS);
END_TEST

START_TEST(test_migrateWithPerformed)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  const long long now = NOW();
  ck_make_dummy_sql("first", "CREATE TABLE first(id serial)", now + 1);
  ck_make_dummy_sql("second", "CREATE TABLE second(id serial)", now + 2);
  ck_make_dummy_sql("third", "CREATE TABLE third(id serial)", now + 3);
  ck_make_dummy_sql("fourth", "CREATE TABLE fourth(id serial)", now + 4);
  char buffer[1024];
  memset(buffer, 0, 1024);
  sprintf(buffer, "%lli\n%lli\n", now + 1, now + 2);
  FILE *f = fopen(AXON_MIGRATIONS_FILE, "w+");
  ck_assert_ptr_ne(f, NULL);
  fprintf(f, "%s", buffer);
  fclose(f);

  char *args[3] = {"inline", "migrate", "--skip-triggers"};
  int result = axon_migrate(3, args);
  ck_assert_int_eq(result, AXON_SUCCESS);
END_TEST

START_TEST(test_invalidMigrate)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  const long long now = NOW();
  int result;
  char *args[3] = {"inline", "migrate", "--skip-triggers"};

  ck_make_dummy_sql("first", "CREATE TABLE first(id serial)", now + 1);
  ck_make_dummy_sql("second", "CREATE TABLE second(id serial)", now + 2);
  result = axon_migrate(3, args);
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_unlink(AXON_MIGRATIONS_FILE);
  ck_make_dummy_sql("third", "CREATE TABLE third(id serial)", now + 3);
  ck_make_dummy_sql("fourth", "CREATE TABLE fourth(id serial)", now + 4);
  ck_redirectStderr(result = axon_migrate(3, args);)
  ck_assert_int_eq(result, AXON_SEQ_INVALID_FILE);
  ck_path_contains("./log/error.log", "aborting (all changes will be reversed)...");
END_TEST

START_TEST(test_markedPerformed)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  const long long int now = NOW();
  ck_make_dummy_sql("one", "SELECT 1", now + 1);
  ck_make_dummy_sql("two", "SELECT 1", now + 2);
  ck_make_dummy_sql("tree", "SELECT 1", now + 3);
  ck_make_dummy_sql("four", "SELECT 1", now + 4);
  ck_make_dummy_sql("five", "SELECT 1", now + 5);
  ck_make_dummy_sql("six", "SELECT 1", now + 6);
  ck_make_dummy_sql("seven", "SELECT 1", now + 7);
  axon_touch("./db/migrate/hello_world");
  ck_unlink(AXON_MIGRATIONS_FILE);
  FILE *f = fopen(AXON_MIGRATIONS_FILE, "a+");
  for (short int i = 0; i < 5; i++) {
    fprintf(f, "%lli\n", now + i + 1);
  }
  fclose(f);

  AxonMigratorContext *context = axon_loadMigrations();
  ck_assert_int_eq(context->len, 7);
  for (unsigned int i = 0; i < 7; i++) {
    ck_assert(context->migrations[i]->perform == (i >= 5));
  }
  axon_freeMigrations(context, 0);
END_TEST

START_TEST(test_migrateWithoutDirectory)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  ck_unlink("./db/migrate");
  AxonMigratorContext *context = axon_loadMigrations();
  ck_assert_ptr_ne(context, NULL);
  ck_assert_int_eq(context->len, 0);
  ck_assert_ptr_eq(context->migrations, NULL);
  axon_freeMigrations(context, 0);
END_TEST

START_TEST(test_isFunctions)
  GO_TO_DUMMY
  ck_assert_int_eq(axon_migrator_isInfo("info"), 1);
  ck_assert_int_eq(axon_migrator_isInfo("--info"), 1);
  ck_assert_int_eq(axon_migrator_isInfo("-i"), 1);
  ck_assert_int_eq(axon_migrator_isInfo("help"), 1);
  ck_assert_int_eq(axon_migrator_isInfo("--help"), 1);
  ck_assert_int_eq(axon_migrator_isInfo("-h"), 1);
  ck_assert_int_eq(axon_migrator_isInfo("set"), 0);

  ck_assert_int_eq(axon_isDatabaseSetup("setup"), 1);
  ck_assert_int_eq(axon_isDatabaseSetup("set"), 0);

  ck_assert_int_eq(axon_isDatabaseSeed("seed"), 1);
  ck_assert_int_eq(axon_isDatabaseSeed("set"), 0);
END_TEST

START_TEST(test_axonSetup)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  axon_createConfig();

  int result;
  ck_unlink(AXON_ORDER_CONFIG_FILE);
  result = axon_databaseSetup();
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "seed:\n  - one.sql\n  - two.sql\n  tree.sql\n");
  result = axon_databaseSetup();
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "setup:\n  - one.sql\n  - two.sql\n  tree.sql\n");
  ck_overrideFile("db/setup/one.sql", "CREATE TABLE test1(id serial);");
  ck_overrideFile("db/setup/two.sql", "CREATE TABLE test2(id serial);");
  ck_overrideFile("db/setup/tree.sql", "CREATE TABLE test3(id serial);");
  result = axon_databaseSetup();
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "setup:\n  - four.sql\n  - five.sql\n");
  ck_overrideFile("db/setup/five.sql", "CREATE TABLE test5(id serial);");
  result = axon_databaseSetup();
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "setup:\n  - five.sql\n");
  result = axon_databaseSetup();
  ck_assert_int_eq(result, AXON_SEQ_INVALID_FILE);
END_TEST

START_TEST(test_axonSeed)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  axon_createConfig();

  char *args[2] = { "inline", "seed" };

  int result;
  ck_unlink(AXON_ORDER_CONFIG_FILE);
  result = axon_runMigrator(2, args);
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "setup:\n  - one.sql\n  - two.sql\n  tree.sql\n");
  result = axon_runMigrator(2, args);
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "seed:\n  - one.sql\n  - two.sql\n  tree.sql\n");
  ck_overrideFile("db/seed/one.sql", "CREATE TABLE test1(id serial);");
  ck_overrideFile("db/seed/two.sql", "CREATE TABLE test2(id serial);");
  ck_overrideFile("db/seed/tree.sql", "CREATE TABLE test3(id serial);");
  result = axon_runMigrator(2, args);
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "seed:\n  - four.sql\n  - five.sql\n");
  ck_overrideFile("db/seed/five.sql", "CREATE TABLE test5(id serial);");
  result = axon_runMigrator(2, args);
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "seed:\n  - five.sql\n");
  result = axon_runMigrator(2, args);
  ck_assert_int_eq(result, AXON_SEQ_INVALID_FILE);
END_TEST

START_TEST(test_sequence)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  char *connInfo = axon_getConnectionInfo();
  ck_assert_ptr_ne(connInfo, NULL);
  AxonSequence *axonSequence = NULL;
  char **files = NULL;
  size_t len = 0;
  int result;

  axonSequence = axon_getSequence(connInfo, files, len);
  ck_assert_ptr_ne(axonSequence, NULL);
  result = axon_execSequence(axonSequence);
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_assert_ptr_eq(axonSequence->errorMessage, NULL);
  axon_freeSequence(axonSequence);

  files = calloc(sizeof(char *), 4);
  ck_overrideFile(files[0] = "db/setup/one.sql", "CREATE TABLE test1(id serial);");
  ck_overrideFile(files[1] = "db/setup/two.sql", "CREATE TABLE test2(id serial);");
  ck_overrideFile(files[2] = "db/setup/tree.sql", "CREATE TABLE test3(id serial);");
  len = 4;
  axonSequence = axon_getSequence(connInfo, files, len);
  ck_assert_ptr_ne(axonSequence, NULL);
  result = axon_execSequence(axonSequence);
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_assert_ptr_eq(axonSequence->errorMessage, NULL);
  axon_freeSequence(axonSequence);
  free(files);

  free(connInfo);
END_TEST

void test_migrator(Suite *s) {
  TCase *testCaseDatabase = tcase_create("Migrator");
  tcase_add_test(testCaseDatabase, test_noArgs);
  tcase_add_test(testCaseDatabase, test_unknownOperation);
  tcase_add_test(testCaseDatabase, test_isFunctions);
  tcase_add_test(testCaseDatabase, test_migrateWithPerformed);
  tcase_add_test(testCaseDatabase, test_migrateWithoutDirectory);
  tcase_add_test(testCaseDatabase, test_migratorCreateDatabase);
  tcase_add_test(testCaseDatabase, test_migratorCreateDatabaseWithoutConfig);
  tcase_add_test(testCaseDatabase, test_migratorDropDatabaseWithoutConfig);
  tcase_add_test(testCaseDatabase, test_migratorMigrate);
  tcase_add_test(testCaseDatabase, test_invalidMigrate);
  tcase_add_test(testCaseDatabase, test_getContextNoConfig);
  tcase_add_test(testCaseDatabase, test_getContextNoDbName);
  tcase_add_test(testCaseDatabase, test_psqlExecNoConnInfo);
  tcase_add_test(testCaseDatabase, test_psqlExecInvalidConnInfo);
  tcase_add_test(testCaseDatabase, test_queryInTransaction);
  tcase_add_test(testCaseDatabase, test_markedPerformed);
  tcase_add_test(testCaseDatabase, test_axonSetup);
  tcase_add_test(testCaseDatabase, test_axonSeed);
  tcase_add_test(testCaseDatabase, test_sequence);
  tcase_add_test(testCaseDatabase, test_migrateWithCallbacks);
  suite_add_tcase(s, testCaseDatabase);
}
