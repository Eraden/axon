#include "./test_utils.h"

START_TEST(test_touchFile)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  ck_unlink("./db");
  int res = axon_touch("./db/setup.csv");
  ck_assert_int_eq(res, 1);
  res = axon_checkIO("./db/setup.csv");
  ck_assert_int_eq(res, 1);
END_TEST

START_TEST(test_getDatabaseName)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  char *name = NULL;

  ck_overrideFile(AXON_DATABASE_CONFIG_FILE, "test:\n  name: test\n");
  name = axon_getDatabaseName();
  ck_assert_ptr_ne(name, NULL);
  ck_assert_str_eq(name, "test");
  free(name);

  ck_overrideFile(AXON_DATABASE_CONFIG_FILE, "dev:\n  name: test\n");
  name = axon_getDatabaseName();
  ck_assert_ptr_eq(name, NULL);
END_TEST

void test_utils(Suite *s) {
  TCase *testCaseUtils = tcase_create("Utils");
  tcase_add_test(testCaseUtils, test_touchFile);
  tcase_add_test(testCaseUtils, test_getDatabaseName);
  suite_add_tcase(s, testCaseUtils);
}
