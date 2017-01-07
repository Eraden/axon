#include "./test.h"
#include "test_config.h"
#include "test_cli.h"
#include "test_migrator.h"

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");

  if (argc == 2 && strcmp(argv[1], "verbose") == 0)
    allMessages = 1;
  else
    allMessages = 0;

  prepare();

  Suite *s = suite_create("koro");
  SRunner *sr;
  int number_failed = 0;

  sr = srunner_create(s);
  srunner_set_fork_status(sr, CK_NOFORK);

  test_config(s);
  test_cli(s);
  test_migrator(s);

  srunner_run_all(sr, CK_SILENT);
  number_failed += srunner_ntests_failed(sr);

  ck_releaseAll();
  srunner_print(sr, CK_VERBOSE);
  srunner_free(sr);

  cleanup();

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
