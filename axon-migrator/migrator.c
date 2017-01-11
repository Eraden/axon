#include "migrator.h"

int axon_runMigrator(int argc, char **argv) {
  if (argc < 2 || axon_migrator_isInfo(argv[1])) {
    axon_migrator_info();
    return AXON_OPERATION_REQUIRED;
  }

  if (axon_isMigrate(argv[1])) {
    return axon_migrate(argc, argv);
  } else if (axon_isDatabaseCreate(argv[1])) {
    return axon_createDatabase();
  } else if (axon_isDatabaseDrop(argv[1])) {
    return axon_dropDatabase();
  } else if (axon_isDatabaseSetup(argv[1])) {
    return axon_databaseSetup();
  } else if (axon_isDatabaseSeed(argv[1])) {
    return axon_databaseSeed();
  } else {
    axon_migrator_info();
    return AXON_UNKNOWN_COMMAND;
  }
}