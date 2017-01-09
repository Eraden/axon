#include "./cli.h"

int axon_runCli(int argc, char **argv) {
  if (argc <= 1) {
    axon_info();
    return AXON_OPERATION_REQUIRED;
  }
  const char *op = argv[1];
  if (axon_isDB(op)) {
    return axon_dbExec(argc, argv);
  }
  axon_info();
  return AXON_UNKNOWN_COMMAND;
}
