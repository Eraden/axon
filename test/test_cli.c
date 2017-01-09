#include "test_cli.h"
#include "./support/reset_environment.h"

START_TEST(test_invalidDirectory)
  GO_TO_DUMMY
  chdir("..");
  int result;
  char *args[4] = {"inline", "db", "init", NULL};
  ck_redirectStderr(
      result = axon_dbExec(4, args);
  )

  ck_assert_int_eq(result, AXON_FAILURE);
  ck_path_contains(errorLogPath, "No src directory in path!");
  GO_TO_DUMMY
END_TEST

START_TEST(test_dbInit)
  GO_TO_DUMMY
  ck_unlink("./db");
  ck_unlink("./src/db");
  ck_unlink(AXON_MIGRATIONS_FILE);

  char *args[4] = {"inline", "db", "init", NULL};
  ck_redirectStdout(
      axon_dbExec(4, args);
  )

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

START_TEST(test_unknownNew)
  GO_TO_DUMMY
  ck_unlink("./db");
  ck_ensureDbEnv();
  char *args[5] = {"inline", "db", "new", "enum", "states"};
  int result;

  ck_redirectStdout(
      result = axon_dbExec(5, args);
  )
  ck_assert_int_eq(result, AXON_FAILURE);
END_TEST

START_TEST(test_newTable)
  GO_TO_DUMMY
  ck_unlink("./db");
  char *args[9] = {"inline", "db", "new", "table", "accounts", "id", "name", "age:int", "timestamps"};
  int result;

  ck_redirectStdout(
      result = axon_dbExec(9, args);
  )
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
  ck_assert_file_in("./db/migrate", "create_table_accounts.sql");
END_TEST

START_TEST(test_dbChange)
  GO_TO_DUMMY
  ck_unlink("./db");
  char *path = NULL;
  int result;

  char *dropArgs[6] = {"inline", "db", "change", "accounts", "drop", "age:int"};

  ck_redirectStdout(
      result = axon_dbExec(6, dropArgs);
  )
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
  ck_assert_file_in("./db/migrate", "change_table_accounts.sql");
  path = ck_find_file_in("./db/migrate", "change_table_posts.sql");
  ck_path_contains(path, "ALTER TABLE posts DROP COLUMN age int;");
  free(path);

  ck_unlink("./db");
  char *addArgs[6] = {"inline", "db", "change", "posts", "add", "age:int"};

  ck_redirectStdout(
      result = axon_dbExec(6, addArgs);
  )
  ck_assert_int_eq(result, AXON_SUCCESS);

  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
  ck_assert_file_in("./db/migrate", "change_table_posts.sql");
  path = ck_find_file_in("./db/migrate", "change_table_posts.sql");
  ck_path_contains(path, "ALTER TABLE posts ADD COLUMN age int;");
  free(path);

  ck_unlink("./db");
  char *removeArgs[6] = {"inline", "db", "change", "posts", "remove", "age:int"};

  ck_redirectStderr(ck_redirectStdout(result = axon_dbExec(6, removeArgs)))
  ck_assert_int_eq(result, AXON_FAILURE);
  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
  ck_path_contains("./log/error.log", "Unknown change operation 'remove'");
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

START_TEST(test_isDB)
  ck_assert_int_eq(axon_isDB("db"), 1);
  ck_assert_int_eq(axon_isDB("database"), 0);
  ck_assert_int_eq(axon_isDB("other"), 0);
END_TEST

START_TEST(test_dbInitExists)
  GO_TO_DUMMY
  ck_unlink("./src/db");
  ck_unlink("./log");
  char *args[5] = {"inline", "db", "new", "enum", "states"};
  int result;

  ck_redirectStderr(
      result = axon_dbExec(5, args);
  )
  ck_assert_int_eq(result, AXON_FAILURE);
  ck_path_contains("./log/error.info", "DB init does not exists! Type `axon db init` to create it");
END_TEST

START_TEST(test_migrateDatabase)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  ck_make_dummy_sql("simple_create", "CREATE TABLE accounts (id serial)", NOW() + 1);
  ck_make_dummy_sql("simple_alter", "ALTER TABLE accounts ADD COLUMN login text", NOW() + 2);

  char *args[3] = {"inline", "db", "migrate"};
  int result = 0;

  ck_redirectStdout(
      result = axon_dbExec(3, args);
  )
  ck_assert_int_eq(result, AXON_SUCCESS);
END_TEST

START_TEST(test_createDB)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  ck_dropTestDb();
  char *args[3] = {"inline", "db", "create"};
  int result = 0;

  ck_redirectStdout(
      result = axon_dbExec(3, args);
  )
  ck_assert_int_eq(result, AXON_SUCCESS);
END_TEST

START_TEST(test_dropDB)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)

  char *args[3] = {"inline", "db", "drop"};
  int result = 0;

  ck_redirectStdout(
      result = axon_dbExec(3, args);
  )
  ck_assert_int_eq(result, AXON_SUCCESS);
END_TEST

START_TEST(test_setupDatabase)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  char *args[3] = {"inline", "db", "setup"};
  int result = 0;

  ck_redirectStdout(
      result = axon_dbExec(3, args);
  )
  ck_assert_int_eq(result, AXON_SUCCESS);
END_TEST

START_TEST(test_unknownCmd)
  GO_TO_DUMMY
  ck_unlink("./db");
  ck_ensureDbEnv();

  char *args[3] = {"inline", "db", "hello"};
  int result = 0;

  ck_redirectStdout(
      result = axon_dbExec(3, args);
  )
  ck_assert_int_eq(result, AXON_UNKNOWN_COMMAND);
END_TEST

void test_cli(Suite *s) {
  TCase *testCaseDatabase = tcase_create("CLI");
  tcase_add_test(testCaseDatabase, test_isDB);
  tcase_add_test(testCaseDatabase, test_dbInitExists);
  tcase_add_test(testCaseDatabase, test_invalidDirectory);
  tcase_add_test(testCaseDatabase, test_unknownNew);
  tcase_add_test(testCaseDatabase, test_info);
  tcase_add_test(testCaseDatabase, test_dbInit);
  tcase_add_test(testCaseDatabase, test_newTable);
  tcase_add_test(testCaseDatabase, test_dbChange);
  tcase_add_test(testCaseDatabase, test_migrateDatabase);
  tcase_add_test(testCaseDatabase, test_createDB);
  tcase_add_test(testCaseDatabase, test_dropDB);
  tcase_add_test(testCaseDatabase, test_setupDatabase);
  tcase_add_test(testCaseDatabase, test_unknownCmd);
  suite_add_tcase(s, testCaseDatabase);
}