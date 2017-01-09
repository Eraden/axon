#include <axon/db.h>

char axon_isDB(const char *str) {
  return strcmp(str, "db") == 0;
}

int axon_dbExec(int argc, char **argv) {
  if (argc < 2) return AXON_SUCCESS;
  const char *op = argv[2];

  if (strcmp(op, "init") != 0 && axon_isDatabaseInitExists() == 0) {
    fprintf(stderr, "DB init does not exists! Type `axon db init` to create it\n");
    return AXON_NOT_INITIALIZED;
  } else if (strcmp(op, "init") == 0) {
    return axon_runCommand("axon-creator init");
  } else if (strcmp(op, "new") == 0) {
    return axon_runCommandArgv("axon-creator new", 3, argc, argv);
  } else if (strcmp(op, "change") == 0) {
    return axon_runCommandArgv("axon-creator change", 3, argc, argv);
  } else if (strcmp(op, "migrate") == 0) {
    return axon_runCommand("axon-migrator migrate");
  } else if (strcmp(op, "create") == 0) {
    return axon_runCommand("axon-migrator create");
  } else if (strcmp(op, "drop") == 0) {
    return axon_runCommand("axon-migrator drop");
  } else if (strcmp(op, "setup") == 0) {
    return axon_runCommand("axon-migrator setup");
  } else {
    return AXON_UNKNOWN_COMMAND;
  }
}
