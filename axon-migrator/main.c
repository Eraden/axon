#include "./migrator.h"

/* LCOV_EXCL_START */
int main(int argc, char **argv) {
  setlocale(LC_ALL, "");
  return axon_runMigrator(argc, argv);
}
/* LCOV_EXCL_STOP */
