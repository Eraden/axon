#include "./test.h"
#include "koro/db/create.h"
#include "koro/db/migrate.h"
#include "koro/db.h"
#include "koro/info.h"

START_TEST(test_migratorMigrate)
  ck_unlink("./db");
  ck_unlink("./.migrations");
  koro_ensureStructure();
  ck_dropTestDb();
  ck_createTestDb();

  ck_make_dummy_sql("create_examples", "CREATE TABLE examples ( id serial );", NOW() + 1);
  ck_make_dummy_sql("change_examples", "ALTER TABLE examples ADD COLUMN login varchar;", NOW() + 2);
  ck_make_dummy_sql("change_examples", "ALTER TABLE examples DROP COLUMN login;", NOW() + 3);
  ck_make_dummy_sql("change_examples", "ALTER TABLE examples ADD COLUMN created_at timestamp;", NOW() + 4);

  ck_assert_int_eq(koro_migrate(), KORO_SUCCESS);
END_TEST

START_TEST(test_migratorCreateDatabase)
  ck_catchStderr("./log/error.log");
  ck_dropTestDb();
  ck_assert_int_eq(koro_createDatabase(), KORO_SUCCESS);
  ck_releaseStderr();
END_TEST

START_TEST(test_readConfig)
  ck_unlink("./conf/database.yml");

  KoroConfig *koroConfig = koro_readConfig();
  ck_assert_int_eq(koroConfig->len, 3);

  KoroEnvironmentConfig **configs = koroConfig->configs;
  char **keys = koroConfig->environments;

  ck_assert_ptr_ne(keys, NULL);
  ck_assert_ptr_ne(configs, NULL);

  KoroEnvironmentConfig *c = NULL;
  const char *key = NULL;

  c = *configs;
  key = *keys;
  ck_assert_ptr_ne(key, NULL);
  ck_assert_str_eq(key, "dev");
  ck_assert_ptr_ne(c, NULL);

  ck_assert_ptr_ne(c->host, NULL);
  ck_assert_str_eq(c->host, "localhost");
  ck_assert_ptr_ne(c->name, NULL);
  ck_assert_str_eq(c->name, "kore_dev");
  ck_assert_int_eq(c->port, 5432);

  keys += 1;
  configs += 1;

  c = *configs;
  key = *keys;
  ck_assert_ptr_ne(key, NULL);
  ck_assert_str_eq(key, "prod");
  ck_assert_ptr_ne(c, NULL);

  ck_assert_ptr_ne(c->host, NULL);
  ck_assert_str_eq(c->host, "localhost");
  ck_assert_ptr_ne(c->name, NULL);
  ck_assert_str_eq(c->name, "kore_prod");
  ck_assert_int_eq(c->port, 5432);

  keys += 1;
  configs += 1;

  c = *configs;
  key = *keys;
  ck_assert_ptr_ne(key, NULL);
  ck_assert_str_eq(key, "test");
  ck_assert_ptr_ne(c, NULL);

  ck_assert_ptr_ne(c->host, NULL);
  ck_assert_str_eq(c->host, "localhost");
  ck_assert_ptr_ne(c->name, NULL);
  ck_assert_str_eq(c->name, "kore_test");
  ck_assert_int_eq(c->port, 5432);

  koro_freeConfig(koroConfig);
END_TEST

START_TEST(test_dbInit)
  ck_unlink("./db");
  ck_unlink("./src/db");
  ck_unlink("./.migrations");

  char *args[4] = {"inline", "db", "init", NULL};
  ck_catchStdout("./log/info.log");
  koro_dbExec(4, args);
  ck_releaseStdout();

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

START_TEST(test_newTable)
  ck_unlink("./db");
  char *args[8] = {"inline", "db", "new", "table", "accounts", "id", "name", "age:int"};
  int result;

  ck_redirectStdout(
      result = koro_dbExec(8, args);
  )
  ck_assert_int_eq(result, KORO_SUCCESS);
  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
  ck_assert_file_in("./db/migrate", "create_table_accounts.sql");
END_TEST

START_TEST(test_dbChange)
  ck_unlink("./db");
  char *path = NULL;
  int result;

  char *dropArgs[6] = {"inline", "db", "change", "accounts", "drop", "age:int"};

  ck_redirectStdout(
      result = koro_dbExec(6, dropArgs);
  )
  ck_assert_int_eq(result, KORO_SUCCESS);

  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
  ck_assert_file_in("./db/migrate", "change_table_accounts.sql");
  path = ck_find_file_in("./db/migrate", "change_table_posts.sql");
  ck_path_contains(path, "ALTER TABLE posts DROP COLUMN age int;");
  free(path);

  ck_unlink("./db");
  char *addArgs[6] = {"inline", "db", "change", "posts", "add", "age:int"};

  ck_redirectStdout(
      result = koro_dbExec(6, addArgs);
  )
  ck_assert_int_eq(result, KORO_SUCCESS);

  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
  ck_assert_file_in("./db/migrate", "change_table_posts.sql");
  path = ck_find_file_in("./db/migrate", "change_table_posts.sql");
  ck_path_contains(path, "ALTER TABLE posts ADD COLUMN age int;");
  free(path);

  ck_unlink("./db");
  char *removeArgs[6] = {"inline", "db", "change", "posts", "remove", "age:int"};

  ck_redirectStderr(ck_redirectStdout(result = koro_dbExec(6, removeArgs)))
  ck_assert_int_eq(result, KORO_FAILURE);
  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
  ck_path_contains("./log/error.log", "Unknown change operation 'remove'");
END_TEST

START_TEST(test_info)
  ck_redirectStdout(koro_info());
  ck_assert_int_eq(koro_isInfo("info"), 1);
  ck_assert_int_eq(koro_isInfo("--info"), 1);
  ck_assert_int_eq(koro_isInfo("-i"), 1);
  ck_assert_int_eq(koro_isInfo("help"), 1);
  ck_assert_int_eq(koro_isInfo("--help"), 1);
  ck_assert_int_eq(koro_isInfo("-h"), 1);
  ck_assert_int_eq(koro_isInfo("db"), 0);
  ck_path_contains("./log/info.log", "koro");
END_TEST

void test_cli(Suite *s) {
  TCase *testCaseDatabase = tcase_create("CLI");
  tcase_add_test(testCaseDatabase, test_info);
  tcase_add_test(testCaseDatabase, test_dbInit);
  tcase_add_test(testCaseDatabase, test_newTable);
  tcase_add_test(testCaseDatabase, test_dbChange);
  suite_add_tcase(s, testCaseDatabase);
}

void test_migrator(Suite *s) {
  TCase *testCaseDatabase = tcase_create("Migrator");
  tcase_add_test(testCaseDatabase, test_migratorCreateDatabase);
  tcase_add_test(testCaseDatabase, test_migratorMigrate);
  suite_add_tcase(s, testCaseDatabase);
}

void test_config(Suite *s) {
  TCase *testCaseConfig = tcase_create("Config");
  tcase_add_test(testCaseConfig, test_readConfig);
  suite_add_tcase(s, testCaseConfig);
}

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");
  setenv("KORE_ENV", "test", 0);
  ck_unlink("./dummy/log");
  ck_unlink("./.migrations");
  ck_dropTestDb();

  if (_ck_io_check("./dummy") != CK_FS_OK) {
    fprintf(stderr, "No `dummy` found!\n");
    exit(EXIT_FAILURE);
  }
  chdir("./dummy");
  koro_mkdir("./log");

  Suite *s = suite_create("koro");

  SRunner *sr;
  int number_failed = 0;

  sr = srunner_create(s);
  srunner_set_fork_status(sr, CK_NOFORK);

  test_cli(s);
  test_config(s);
  test_migrator(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed += srunner_ntests_failed(sr);
  srunner_free(sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}