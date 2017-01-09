#include "./creator.h"

static int axon_newCommand(int argc, char **argv);

static int axon_dropCommand(int argc, char **argv);

static int axon_newCommand(int argc, char **argv) {
  if (argc < 3) return AXON_NOT_ENOUGH_ARGS;

  const char *type = argv[2];

  if (strcmp(type, "table") == 0) {
    return axon_newTable(argc, argv);
  } else if (strcmp(type, "enum") == 0) {
    return axon_newEnum(argc, argv);
  } else {
    axon_creator_info();
    return AXON_UNKNOWN_COMMAND;
  }
}

static int axon_dropCommand(int argc, char **argv) {
  if (argc < 3) return AXON_NOT_ENOUGH_ARGS;
  int result = axon_ensureStructure();
  if (result != AXON_SUCCESS) {
    AXON_NO_SRC_DIRECTORY_MSG
    return result;
  }

  const char *type = argv[2];

  if (strcmp(type, "table") == 0) {
    return axon_dropTable(argc, argv);
  } else if (strcmp(type, "enum") == 0) {
    return axon_dropEnum(argc, argv);
  } else {
    axon_creator_info();
    return AXON_UNKNOWN_COMMAND;
  }
}

static int axon_renameCommand(int argc, char **argv) {
  if (argc < 3) return AXON_NOT_ENOUGH_ARGS;
  int result = axon_ensureStructure();
  if (result != AXON_SUCCESS) {
    AXON_NO_SRC_DIRECTORY_MSG
    return result;
  }

  const char *type = argv[2];

  if (strcmp(type, "table") == 0) {
    return axon_renameTable(argc, argv);
  } else {
    axon_creator_info();
    return AXON_UNKNOWN_COMMAND;
  }
}

int axon_runCreator(int argc, char **argv) {
  if (argc < 2 || axon_creator_isInfo(argv[1])) {
    axon_creator_info();
    return AXON_OPERATION_REQUIRED;
  }
  const char *op = argv[1];

  if (strcmp(op, "init") == 0) {
    return axon_initCommand();
  } else if (strcmp(op, "new") == 0) {
    return axon_newCommand(argc, argv);
  } else if (strcmp(op, "change") == 0) {
    return axon_changeTable(argc, argv);
  } else if (strcmp(op, "drop") == 0) {
    return axon_dropCommand(argc, argv);
  } else if (strcmp(op, "rename") == 0) {
    return axon_renameCommand(argc, argv);
  } else {
    axon_creator_info();
    return AXON_UNKNOWN_COMMAND;
  }
  return AXON_SUCCESS;
}
