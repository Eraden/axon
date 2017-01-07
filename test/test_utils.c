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

void test_utils(Suite *s) {
  TCase *testCaseUtils = tcase_create("Utils");
  tcase_add_test(testCaseUtils, test_touchFile);
  suite_add_tcase(s, testCaseUtils);
}
