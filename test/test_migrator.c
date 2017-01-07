#include "test_migrator.h"

START_TEST(test_migratorMigrate)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)

  ck_make_dummy_sql("create_examples", "CREATE TABLE examples ( id serial );", NOW() + 1);
  ck_make_dummy_sql("change_examples", "ALTER TABLE examples ADD COLUMN login varchar;", NOW() + 2);
  ck_make_dummy_sql("change_examples", "ALTER TABLE examples DROP COLUMN login;", NOW() + 3);
  ck_make_dummy_sql("change_examples", "ALTER TABLE examples ADD COLUMN created_at timestamp;", NOW() + 4);

  ck_assert_int_eq(koro_migrate(), KORO_SUCCESS);
END_TEST

START_TEST(test_migratorCreateDatabase)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  int result;
  ck_dropTestDb();
  ck_redirectStderr(
      result = koro_createDatabase();
  )
  ck_assert_int_eq(result, KORO_SUCCESS);
END_TEST

START_TEST(test_migratorCreateDatabaseWithoutConfig)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  ck_dropTestDb();
  ck_unlink("./conf/database.yml");
  int result;
  ck_redirectStderr(
      result = koro_createDatabase();
  )
  ck_assert_int_eq(result, KORO_CONFIG_MISSING);
END_TEST

START_TEST(test_migratorDropDatabaseWithoutConfig)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  ck_dropTestDb();
  ck_unlink("./conf/database.yml");
  int result;
  ck_redirectStderr(
      result = koro_dropDatabase();
  )
  ck_assert_int_eq(result, KORO_CONFIG_MISSING);
END_TEST

START_TEST(test_getContextNoConfig)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */);
  putenv("KORE_ENV=invalid-env");
  ck_unlink("./conf/database.yml");
  char *info = NULL;
  ck_redirectStderr(koro_getConnectionInfo();)
  ck_assert_ptr_eq(info, NULL);
  putenv("KORE_ENV=test");
  ck_path_contains("./log/error.log", "No database config file for current env!");
END_TEST

START_TEST(test_getContextNoDbName)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */);
  ck_overrideFile("./conf/database.yml", "test:\n  host: localhost\n");
  char *info = NULL;
  ck_redirectStderr(koro_getConnectionInfo();)
  ck_assert_ptr_eq(info, NULL);
  ck_unlink("./conf/database.yml");
  IN_CLEAR_STATE(/* */);
END_TEST

START_TEST(test_psqlExecNoConnInfo)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */);
  KoroExecContext context = koro_getContext("", NULL, KORO_ONLY_QUERY);
  int result = koro_psqlExecute(&context);
  ck_assert_int_eq(result, KORO_CONFIG_MISSING);
END_TEST

START_TEST(test_psqlExecInvalidConnInfo)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */);
  KoroExecContext context = koro_getContext("", "dbname = 1234567890asdfghjkl", KORO_ONLY_QUERY);
  int result;
  ck_redirectStderr(result = koro_psqlExecute(&context);)
  ck_assert_int_eq(result, KORO_FAILURE);
END_TEST

START_TEST(test_queryInTransaction)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  KoroExecContext context;
  context = koro_getContext(
      "CREATE TABLE accounts(id serial, login text)",
      "dbname = kore_test",
      KORO_ONLY_QUERY
  );
  koro_psqlExecute(&context);
  context.sql = "INSERT INTO accounts(login) VALUES ('hello'), ('world')";
  koro_psqlExecute(&context);
  context.sql = "SELECT id FROM accounts";
  context.type = KORO_ONLY_QUERY | KORO_TRANSACTION_QUERY | KORO_USE_CURSOR_QUERY;
  int result = koro_psqlExecute(&context);
  ck_assert_int_eq(result, KORO_SUCCESS);
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
  ck_unlink("./.migrations");
  FILE *f = fopen("./.migrations", "a+");
  for (short int i = 0; i < 5; i++) {
    fprintf(f, "%lli\n", now + i + 1);
  }
  fclose(f);

  KoroMigratorContext *context = koro_loadMigrations();
  for (unsigned int i = 0; i < 7; i++) {
    ck_assert(context->migrations[i]->perform == (i >= 5));
  }
  koro_freeMigrations(context, 0);
END_TEST

void test_migrator(Suite *s) {
  TCase *testCaseDatabase = tcase_create("Migrator");
  tcase_add_test(testCaseDatabase, test_migratorCreateDatabase);
  tcase_add_test(testCaseDatabase, test_migratorCreateDatabaseWithoutConfig);
  tcase_add_test(testCaseDatabase, test_migratorDropDatabaseWithoutConfig);
  tcase_add_test(testCaseDatabase, test_migratorMigrate);
  tcase_add_test(testCaseDatabase, test_getContextNoConfig);
  tcase_add_test(testCaseDatabase, test_getContextNoDbName);
  tcase_add_test(testCaseDatabase, test_psqlExecNoConnInfo);
  tcase_add_test(testCaseDatabase, test_psqlExecInvalidConnInfo);
  tcase_add_test(testCaseDatabase, test_queryInTransaction);
  tcase_add_test(testCaseDatabase, test_markedPerformed);
  suite_add_tcase(s, testCaseDatabase);
}
