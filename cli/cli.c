#include "./cli.h"

static char axon_isTriggersCommand(const char *str) {
  return strcmp(str, "triggers") == 0;
}

static int axon_execTriggersCommand() {
  return axon_runCommand("axon-compiler triggers");
}

int axon_runCli(int argc, char **argv) {
  if (argc <= 1) {
    axon_info();
    return AXON_OPERATION_REQUIRED;
  }
  const char *op = argv[1];
  if (axon_isDatabaseCommand(op)) {
    return axon_execDatabaseCommand(argc, argv);
  } else if (axon_isTriggersCommand(op)) {
    return axon_execTriggersCommand();
  }
  axon_info();
  return AXON_UNKNOWN_COMMAND;
}
