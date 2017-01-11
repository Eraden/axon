#include "test_cli.h"
#include "./support/reset_environment.h"

START_TEST(test_invalidCliOperation)
  GO_TO_DUMMY
  int result;

  char *noArgs[2] = {"inline", NULL};
  ck_redirectStdout(result = axon_runCli(1, noArgs);)
  ck_assert_int_eq(result, AXON_OPERATION_REQUIRED);

  char *invalidCommand[2] = {"inline", "hello"};
  ck_redirectStdout(result = axon_runCli(2, invalidCommand);)
  ck_assert_int_eq(result, AXON_UNKNOWN_COMMAND);
END_TEST

START_TEST(test_initFilesExists)
  GO_TO_DUMMY
  ck_unlink("./src/db");
  ck_unlink("./log");
  int result;

  char *args[5] = {"inline", "db", "new", "enum", "states"};
  ck_redirectStderr(result = axon_runCli(5, args);)
  ck_assert_int_eq(result, AXON_NOT_INITIALIZED);
  ck_path_contains("./log/error.info", "DB init does not exists! Type `axon db init` to create it");
END_TEST

START_TEST(test_invalidDirectory)
  GO_TO_DUMMY
  chdir("..");

  int result;

  char *args[4] = {"inline", "db", "init", NULL};
  ck_redirectStderr(result = axon_runCli(4, args);)
  ck_assert_int_eq(result, AXON_INVALID_DIRECTORY);
  ck_path_contains(errorLogPath, "No src directory in path!");
  GO_TO_DUMMY
END_TEST

START_TEST(test_initDatabaseConnectionFiles)
  GO_TO_DUMMY
  ck_unlink("./db");
  ck_unlink("./src/db");
  ck_unlink(AXON_MIGRATIONS_FILE);

  char *args[4] = {"inline", "db", "init", NULL};
  ck_redirectStdout(axon_runCli(4, args);)
  ck_path_exists("./db");
  ck_path_exists("./db/setup");
  ck_path_exists("./db/migrate");
  ck_path_exists("./db/seed");
  ck_path_exists("./src/db");
  ck_path_exists("./src/db/init.h");
  ck_path_exists("./src/db/init.c");
  ck_path_contains("./src/db/init.h", "db_init");
  ck_path_contains("./src/db/init.c", "db_init");
END_TEST

START_TEST(test_databaseCommandNew)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)

  int result;

  ck_unlink("./db");
  char *newTableArgs[9] = {"inline", "db", "new", "table", "accounts", "id", "name", "age:int", "timestamps"};
  ck_redirectStdout(result = axon_runCli(9, newTableArgs);)
  ck_assert_int_eq(result, AXON_SUCCESS);
  char *path = ck_find_file_in("./db/migrate", "create_table_accounts.sql");
  ck_assert_ptr_ne(path, NULL);
  free(path);

  ck_unlink("./db");
  char *newEnumArgs[6] = {"inline", "db", "new", "enum", "states", "active"};
  ck_redirectStdout(result = axon_runCli(6, newEnumArgs);)
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_unlink("./db");
  char *newUnknownType[5] = {"inline", "db", "new", "foo", "states"};
  ck_redirectStdout(result = axon_runCli(5, newUnknownType);)
  ck_assert_int_eq(result, AXON_UNKNOWN_COMMAND);
END_TEST

START_TEST(test_changeTable)
  GO_TO_DUMMY
  char *path = NULL;
  int result;

  ck_unlink("./db");
  char *dropArgs[6] = {"inline", "db", "change", "accounts", "drop", "age:int"};
  ck_redirectStdout(result = axon_runCli(6, dropArgs);)
  ck_assert_int_eq(result, AXON_SUCCESS);
  path = ck_find_file_in("./db/migrate", "drop_column_from_table_accounts.sql");
  ck_assert_ptr_ne(path, NULL);
  ck_path_contains(path, "ALTER TABLE accounts DROP COLUMN age;");
  free(path);

  ck_unlink("./db");
  char *addArgs[6] = {"inline", "db", "change", "posts", "add", "age:int"};
  ck_redirectStdout(result = axon_runCli(6, addArgs);)
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_assert_file_in("./db/migrate", "add_column_to_table_posts.sql");
  path = ck_find_file_in("./db/migrate", "change_table_posts.sql");
  ck_path_contains(path, "ALTER TABLE posts ADD COLUMN age int;");
  free(path);

  ck_unlink("./db");
  char *removeArgs[6] = {"inline", "db", "change", "posts", "remove", "age:int"};
  ck_redirectStderr(ck_redirectStdout(result = axon_execDatabaseCommand(6, removeArgs)))
  ck_assert_int_eq(result, AXON_UNKNOWN_COMMAND);
  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
END_TEST

START_TEST(test_info)
  GO_TO_DUMMY

  ck_redirectStdout(axon_info());
  ck_assert_int_eq(axon_isInfo("info"), 1);
  ck_assert_int_eq(axon_isInfo("--info"), 1);
  ck_assert_int_eq(axon_isInfo("-i"), 1);
  ck_assert_int_eq(axon_isInfo("help"), 1);
  ck_assert_int_eq(axon_isInfo("--help"), 1);
  ck_assert_int_eq(axon_isInfo("-h"), 1);
  ck_assert_int_eq(axon_isInfo("db"), 0);
  ck_path_contains("./log/info.log", "axon");
END_TEST

START_TEST(test_migrateDatabase)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  ck_make_dummy_sql("simple_create", "CREATE TABLE accounts (id serial)", NOW() + 1);
  ck_make_dummy_sql("simple_alter", "ALTER TABLE accounts ADD COLUMN login text", NOW() + 2);

  char *args[3] = {"inline", "db", "migrate"};
  int result = 0;

  ck_redirectStdout(result = axon_runCli(3, args);)
  ck_assert_int_eq(result, AXON_SUCCESS);
END_TEST

START_TEST(test_triggers)
  GO_TO_DUMMY
  int result;

  char *triggersArgs[2] = {"inline", "triggers"};
  ck_redirectStdout(result = axon_runCli(2, triggersArgs);)
  ck_assert_int_eq(result, AXON_SUCCESS);
END_TEST

START_TEST(test_databaseCommands)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  int result = 0;

  ck_assert_int_eq(axon_isDatabaseCommand("db"), 1);
  ck_assert_int_eq(axon_isDatabaseCommand("database"), 0);
  ck_assert_int_eq(axon_isDatabaseCommand("other"), 0);

  ck_dropTestDb();
  char *createDatabaseArgs[3] = {"inline", "db", "create"};
  ck_redirectStdout(result = axon_runCli(3, createDatabaseArgs);)
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_createTestDb();
  char *dropDatabaseArgs[3] = {"inline", "db", "drop"};
  ck_redirectStdout(result = axon_runCli(3, dropDatabaseArgs);)
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_createTestDb();
  char *setupDatabaseArgs[3] = {"inline", "db", "setup"};
  ck_redirectStdout(result = axon_runCli(3, setupDatabaseArgs);)
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_createTestDb();
  char *seedDatabaseArgs[3] = {"inline", "db", "seed"};
  ck_redirectStdout(result = axon_runCli(3, seedDatabaseArgs);)
  ck_assert_int_eq(result, AXON_SUCCESS);

  char *invalidDatabaseArgs[3] = {"inline", "db", "hello"};
  ck_redirectStdout(result = axon_runCli(3, invalidDatabaseArgs);)
  ck_assert_int_eq(result, AXON_UNKNOWN_COMMAND);
END_TEST

void test_cli(Suite *s) {
  TCase *testCaseDatabase = tcase_create("CLI");
  tcase_add_test(testCaseDatabase, test_initFilesExists);
  tcase_add_test(testCaseDatabase, test_invalidDirectory);
  tcase_add_test(testCaseDatabase, test_databaseCommandNew);
  tcase_add_test(testCaseDatabase, test_info);
  tcase_add_test(testCaseDatabase, test_invalidCliOperation);
  tcase_add_test(testCaseDatabase, test_initDatabaseConnectionFiles);
  tcase_add_test(testCaseDatabase, test_changeTable);
  tcase_add_test(testCaseDatabase, test_databaseCommands);
  tcase_add_test(testCaseDatabase, test_migrateDatabase);
  tcase_add_test(testCaseDatabase, test_triggers);
  suite_add_tcase(s, testCaseDatabase);
}