#include "./test_compiler.h"

START_TEST(test_compiler_triggers)
  GO_TO_DUMMY
  ck_unlink(AXON_TRIGGERS_FILE);
  IN_CLEAR_STATE(/* */)
  FILE *f;
  int result;
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

  ck_overrideFile(
      AXON_TRIGGERS_FILE,
      ""
          "libs: axonconfig axonutils\n"
          "flags: -I../includes -L../lib\n"
  );

  char *args[2] = {"inline", "triggers"};
  result = axon_runCompiler(2, args);
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_assert_file_in("./.axon", "1_before_callback.o");
  ck_assert_file_in("./.axon", "triggers.so");

  ck_overrideFile(
      AXON_TRIGGERS_FILE,
      ""
          "libs:axonconfig axonutils\n"
          "flags:-I../includes -L../lib\n"
  );

  result = axon_runCompiler(2, args);
  ck_assert_int_eq(result, AXON_SUCCESS);
  ck_assert_file_in("./.axon", "1_before_callback.o");
  ck_assert_file_in("./.axon", "triggers.so");

  GO_TO_DUMMY
  ck_unlink("./db");
  result = axon_runCompiler(2, args);
  ck_assert_int_eq(result, AXON_SUCCESS);
END_TEST

START_TEST(test_unknownType)
  GO_TO_DUMMY
  char *args[2] = {"inline", "foo"};
  ck_assert_int_eq(axon_runCompiler(2, args), AXON_UNKNOWN_COMMAND);
END_TEST

void test_compiler(Suite *s) {
  TCase *testCaseCompiler = tcase_create("Compiler");
  tcase_add_test(testCaseCompiler, test_compiler_triggers);
  tcase_add_test(testCaseCompiler, test_unknownType);
  suite_add_tcase(s, testCaseCompiler);
}
