#include "./test_creator.h"

START_TEST(test_noArgsGiven)
  GO_TO_DUMMY

  int result = 0;

  char *args[1] = {"inline"};
  ck_redirectStdout(result = axon_runCreator(1, args););
  ck_assert_int_eq(result, AXON_OPERATION_REQUIRED);
  char buffer[1024];
  sprintf(buffer, AXON_CREATOR_INFO, AXON_MAJOR_VERSION, AXON_MINOR_VERSION, AXON_PATCH_VERSION);
  ck_path_contains("./log/info.log", buffer);
END_TEST

START_TEST(test_unknownCommand)
  GO_TO_DUMMY

  int result = 0;
  char buffer[1024];

  char *args[2] = {"inline", "foo"};
  ck_redirectStdout(result = axon_runCreator(2, args););
  ck_assert_int_eq(result, AXON_UNKNOWN_COMMAND);
  sprintf(buffer, AXON_CREATOR_INFO, AXON_MAJOR_VERSION, AXON_MINOR_VERSION, AXON_PATCH_VERSION);
  ck_path_contains("./log/info.log", buffer);

  char *unknownNewArgs[3] = {"inline", "new", "foo"};
  ck_redirectStdout(result = axon_runCreator(3, unknownNewArgs););
  ck_assert_int_eq(result, AXON_UNKNOWN_COMMAND);
  sprintf(buffer, AXON_CREATOR_INFO, AXON_MAJOR_VERSION, AXON_MINOR_VERSION, AXON_PATCH_VERSION);
  ck_path_contains("./log/info.log", buffer);

  char *unknownRenameArgs[3] = {"inline", "rename", "foo"};
  ck_redirectStdout(result = axon_runCreator(3, unknownRenameArgs););
  ck_assert_int_eq(result, AXON_UNKNOWN_COMMAND);
  sprintf(buffer, AXON_CREATOR_INFO, AXON_MAJOR_VERSION, AXON_MINOR_VERSION, AXON_PATCH_VERSION);
  ck_path_contains("./log/info.log", buffer);

  char *unknownDropArgs[3] = {"inline", "drop", "foo"};
  ck_redirectStdout(result = axon_runCreator(3, unknownDropArgs););
  ck_assert_int_eq(result, AXON_UNKNOWN_COMMAND);
  sprintf(buffer, AXON_CREATOR_INFO, AXON_MAJOR_VERSION, AXON_MINOR_VERSION, AXON_PATCH_VERSION);
  ck_path_contains("./log/info.log", buffer);

  char *unknownChangeArgs[5] = {"inline", "change", "foo", "bar", "example"};
  ck_redirectStdout(result = axon_runCreator(5, unknownChangeArgs););
  ck_assert_int_eq(result, AXON_UNKNOWN_COMMAND);
  sprintf(buffer, AXON_CREATOR_INFO, AXON_MAJOR_VERSION, AXON_MINOR_VERSION, AXON_PATCH_VERSION);
  ck_path_contains("./log/info.log", buffer);
END_TEST

START_TEST(test_info)
  char *args[2] = {"inline", "info"};
  int result = 0;

  ck_redirectStdout(result = axon_runCreator(2, args););
  ck_assert_int_eq(result, AXON_OPERATION_REQUIRED);
  char buffer[1024];
  sprintf(buffer, AXON_CREATOR_INFO, AXON_MAJOR_VERSION, AXON_MINOR_VERSION, AXON_PATCH_VERSION);
  ck_path_contains("./log/info.log", buffer);
END_TEST

START_TEST(test_notEnaughNewArgs)
  GO_TO_DUMMY

  int result = 0;

  char *args[2] = {"inline", "new"};
  ck_redirectStdout(result = axon_runCreator(2, args););
  ck_assert_int_eq(result, AXON_NOT_ENOUGH_ARGS);
END_TEST

START_TEST(test_newTable)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  int result;
  char *path = NULL;

  ck_unlink("./db");
  char *args[8] = {"inline", "new", "table", "accounts", "id", "name", "age:int", "timestamps"};
  ck_redirectStdout(result = axon_runCreator(8, args);)
  ck_assert_int_eq(result, AXON_SUCCESS);
  path = ck_find_file_in("./db/migrate", "create_table_accounts.sql");
  ck_assert_ptr_ne(path, NULL);
  ck_path_contains(
      path,
      "CREATE TABLE accounts (\n  id serial,\n  name varchar,\n  age int,\n  updated_at timestamp,\n  created_at timestamp\n);\n"
  );
  free(path);

  ck_unlink("./db");
  char *newTableWithReference[6] = {"inline", "new", "table", "posts", "id", "account_id:accounts(id)"};
  ck_redirectStdout(result = axon_runCreator(6, newTableWithReference);)
  ck_assert_int_eq(result, AXON_SUCCESS);
  path = ck_find_file_in("./db/migrate", "create_table_posts.sql");
  ck_assert_ptr_ne(path, NULL);
  ck_path_contains(
      path,
      "CREATE TABLE posts (\n  id serial,\n  account_id int REFERENCES accounts(id)\n);\n"
  );
  free(path);

  char *notEnoughArgs[3] = {"inline", "new", "table"};
  ck_redirectStderr(result = axon_runCreator(3, notEnoughArgs);)
  ck_assert_int_eq(result, AXON_NOT_ENOUGH_ARGS);

  chdir("..");
  char *wrongDirArgs[4] = {"inline", "new", "table", "foo"};
  ck_redirectStderr(result = axon_runCreator(4, wrongDirArgs);)
  ck_assert_int_eq(result, AXON_INVALID_DIRECTORY);
END_TEST

START_TEST(test_dropTable)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  int result;

  ck_unlink("./db");
  char *args[4] = {"inline", "drop", "table", "accounts"};
  ck_redirectStderr(result = axon_runCreator(4, args);)
  ck_assert_int_eq(result, AXON_SUCCESS);
  char *path = ck_find_file_in("./db/migrate", "drop_table_accounts.sql");
  ck_assert_ptr_ne(path, NULL);
  ck_path_contains(path, "DROP TABLE accounts;\n");
  free(path);

  char *notEnoughArgs[3] = {"inline", "drop", "table"};
  ck_redirectStderr(result = axon_runCreator(3, notEnoughArgs);)
  ck_assert_int_eq(result, AXON_NOT_ENOUGH_ARGS);

  chdir("..");
  char *wrongDirArgs[4] = {"inline", "drop", "table", "foo"};
  ck_redirectStderr(result = axon_runCreator(4, wrongDirArgs);)
  ck_assert_int_eq(result, AXON_INVALID_DIRECTORY);
END_TEST

START_TEST(test_renameTable)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  int result;

  ck_unlink("./db");
  char *args[5] = {"inline", "rename", "table", "users", "accounts"};
  ck_redirectStderr(result = axon_runCreator(5, args);)
  ck_assert_int_eq(result, AXON_SUCCESS);
  char *path = ck_find_file_in("./db/migrate", "rename_table_users.sql");
  ck_assert_ptr_ne(path, NULL);
  ck_path_contains(path, "ALTER TABLE users RENAME TO accounts;\n");
  free(path);

  char *notEnoughArgs[3] = {"inline", "rename", "table"};
  ck_redirectStderr(result = axon_runCreator(3, notEnoughArgs);)
  ck_assert_int_eq(result, AXON_NOT_ENOUGH_ARGS);

  chdir("..");
  char *wrongDirArgs[5] = {"inline", "rename", "table", "foo", "bar"};
  ck_redirectStderr(result = axon_runCreator(5, wrongDirArgs);)
  ck_assert_int_eq(result, AXON_INVALID_DIRECTORY);
END_TEST

START_TEST(test_newEnum)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  int result;

  ck_unlink("./db");
  char *args[8] = {"inline", "new", "enum", "colors", "yellow", "blue", "red", "black"};
  ck_redirectStdout(result = axon_runCreator(8, args);)
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
  char *path = ck_find_file_in("./db/migrate", "create_enum_colors.sql");
  ck_assert_ptr_ne(path, NULL);
  ck_path_contains(path, "CREATE TYPE colors AS enum (\n  'yellow',\n  'blue',\n  'red',\n  'black'\n);\n");
  free(path);

  char *noTypesArgs[4] = {"inline", "new", "enum", "colors"};
  ck_redirectStdout(result = axon_runCreator(4, noTypesArgs);)
  ck_assert_int_eq(result, AXON_NOT_ENOUGH_ARGS);

  chdir("..");
  char *wrongDirArgs[5] = {"inline", "new", "enum", "colors", "blue"};
  ck_redirectStdout(result = axon_runCreator(5, wrongDirArgs);)
  ck_assert_int_eq(result, AXON_INVALID_DIRECTORY);
END_TEST

START_TEST(test_dropEnum)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  int result;

  ck_unlink("./db");
  char *args[4] = {"inline", "drop", "enum", "colors"};
  ck_redirectStdout(result = axon_runCreator(4, args);)
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
  char *path = ck_find_file_in("./db/migrate", "drop_enum_colors.sql");
  ck_assert_ptr_ne(path, NULL);
  ck_path_contains(path, "DROP TYPE colors;\n");
  free(path);

  char *noTypesArgs[3] = {"inline", "drop", "enum"};
  ck_redirectStdout(result = axon_runCreator(3, noTypesArgs);)
  ck_assert_int_eq(result, AXON_NOT_ENOUGH_ARGS);

  chdir("..");
  char *wrongDirArgs[4] = {"inline", "drop", "enum", "colors"};
  ck_redirectStdout(result = axon_runCreator(4, wrongDirArgs);)
  ck_assert_int_eq(result, AXON_INVALID_DIRECTORY);
END_TEST

START_TEST(test_changeTable)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  char *path = NULL;
  int result;

  ck_unlink("./db");
  char *dropArgs[5] = {"inline", "change", "accounts", "drop", "age:int"};
  ck_redirectStdout(result = axon_runCreator(5, dropArgs);)
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
  path = ck_find_file_in("./db/migrate", "drop_column_from_table_accounts.sql");
  ck_assert_ptr_ne(path, NULL);
  ck_path_contains(path, "ALTER TABLE accounts DROP COLUMN age;");
  free(path);

  ck_unlink("./db");
  char *addArgs[5] = {"inline", "change", "posts", "add", "age:int"};
  ck_redirectStdout(result = axon_runCreator(5, addArgs);)
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
  path = ck_find_file_in("./db/migrate", "add_column_to_table_posts.sql");
  ck_assert_ptr_ne(path, NULL);
  ck_path_contains(path, "ALTER TABLE posts ADD COLUMN age int;");
  free(path);

  ck_unlink("./db");
  char *retypeArgs[5] = {"inline", "change", "posts", "retype", "age:int"};
  ck_redirectStdout(result = axon_runCreator(5, retypeArgs))
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_path_exists("./db");
  ck_path_exists("./db/migrate");
  path = ck_find_file_in("./db/migrate", "change_column_type_posts.sql");
  ck_assert_ptr_ne(path, NULL);
  ck_path_contains(path, "ALTER TABLE posts ALTER COLUMN age TYPE int;");
  free(path);

  ck_unlink("./db");
  char *removeArgs[5] = {"inline", "change", "posts", "remove", "age:int"};
  ck_redirectStderr(ck_redirectStdout(result = axon_runCreator(5, removeArgs)))
  ck_assert_int_eq(result, AXON_UNKNOWN_COMMAND);
  ck_path_exists("./db");
  ck_path_exists("./db/migrate");

  chdir("..");
  char *wrongDirArgs[5] = {"inline", "change", "posts", "drop", "age:int"};
  ck_redirectStderr(ck_redirectStdout(result = axon_runCreator(5, wrongDirArgs);))
  ck_assert_int_eq(result, AXON_INVALID_DIRECTORY);
END_TEST

void test_creator(Suite *s) {
  TCase *testCaseCreator = tcase_create("Creator");
  tcase_add_test(testCaseCreator, test_noArgsGiven);
  tcase_add_test(testCaseCreator, test_notEnaughNewArgs);
  tcase_add_test(testCaseCreator, test_unknownCommand);
  tcase_add_test(testCaseCreator, test_info);
  tcase_add_test(testCaseCreator, test_newTable);
  tcase_add_test(testCaseCreator, test_dropTable);
  tcase_add_test(testCaseCreator, test_renameTable);
  tcase_add_test(testCaseCreator, test_newEnum);
  tcase_add_test(testCaseCreator, test_dropEnum);
  tcase_add_test(testCaseCreator, test_changeTable);
  suite_add_tcase(s, testCaseCreator);
}
