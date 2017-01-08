#include <axon/info.h>
#include <axon/db/migrate.h>
#include <axon/db/create.h>
#include <axon/db/drop.h>
#include <axon/db/setup.h>

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");

  /* LCOV_EXCL_START */
  if (argc < 2 || axon_migrator_isInfo(argv[1])) {
    axon_migrator_info();
    return AXON_OPERATION_REQUIRED;
  }
  /* LCOV_EXCL_STOP */

  if (axon_isMigrate(argv[1])) {
    return axon_migrate();
  } else if (axon_isDatabaseCreate(argv[1])) {
    return axon_createDatabase();
  } else if (axon_isDatabaseDrop(argv[1])) {
    return axon_dropDatabase();
  } else if (axon_isSetup(argv[1])) {
    return axon_setup();
  } else {
    /* LCOV_EXCL_START */
    fprintf(stderr, "Unknown operation give, exiting...\n");
    return AXON_UNKNOWN_COMMAND;
    /* LCOV_EXCL_STOP */
  }
}
