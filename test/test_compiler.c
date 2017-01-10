#include "./test_compiler.h"

START_TEST(test_compiler_triggers)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  FILE *f;
  long long unsigned t;

  f = fopen("./db/migrate/1_before_callback.c", "w+");
  ck_assert_ptr_ne(f, NULL);
  t = NOW() + 1;
  fprintf(f, AXON_CALLBACK, "before", t, "before", t);
  fclose(f);

  f = fopen("./db/migrate/1_after_callback.c", "w+");
  ck_assert_ptr_ne(f, NULL);
  t = NOW() + 1;
  fprintf(f, AXON_CALLBACK, "after", t, "after", t);
  fclose(f);

  char *args[2] = {"inline", "triggers"};
  axon_runCompiler(2, args);
END_TEST

void test_compiler(Suite *s) {
  TCase *testCaseCompiler = tcase_create("Compiler");
  tcase_add_test(testCaseCompiler, test_compiler_triggers);
  suite_add_tcase(s, testCaseCompiler);
}
