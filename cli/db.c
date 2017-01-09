#include "axon/db.h"

static char axon_isDbInitExists(void);

static char axon_dbInit();

static char axon_isDbInitExists(void) {
  char *fileName = "src/db/init.h";
  return axon_checkIO(fileName);
}

static char axon_dbInit() {
  if (axon_checkIO("./src") == 0) {
    fprintf(stderr, "No src directory in path!\n");
    return AXON_FAILURE;
  }

  axon_ensureStructure();

  if (axon_checkIO("./src/db/init.c") == 0) {
    FILE *f = fopen("./src/db/init.c", "w+");
    fprintf(f, "%s", INIT_SOURCE_CONTENT);
    fclose(f);
  }
  if (axon_checkIO("./src/db/init.h") == 0) {
    FILE *f = fopen("./src/db/init.h", "w+");
    fprintf(f, "%s", INIT_HEADER_CONTENT);
    fclose(f);
  }

  printf(
      ""
          "Please modify your conf/project.conf\n"
          "  load    ./project.so db_init\n"
  );

  AxonGraph dbChildren[2] = {{.root="init.c",.len=0}, {.root="init.h",.len=0}};
  AxonGraph srcChildren[1] = {{.root="db", .leafs=dbChildren,.len=2}};
  AxonGraph axonGraph = {.root="src", .len=1, .leafs=srcChildren};
  axon_createInfo(&axonGraph);

  return AXON_SUCCESS;
}

static char axon_dbNew(int argc, char **argv) {
  if (argc < 4) return 0;
  const char *type = argv[3];

  if (strcmp(type, "table") == 0) {
    return axon_dbNewTable(argc, argv);
  } else {
    return AXON_FAILURE;
  }
}

char axon_isDB(const char *str) {
  return strcmp(str, "db") == 0;
}

char axon_dbExec(int argc, char **argv) {
  if (argc < 2) return 0;
  const char *op = argv[2];

  if (strcmp(op, "init") != 0 && axon_isDbInitExists() == 0) {
    fprintf(stderr, "DB init does not exists! Type `axon db init` to create it\n");
    return AXON_FAILURE;
  } else if (strcmp(op, "init") == 0) {
    return axon_dbInit();
  } else if (strcmp(op, "new") == 0) {
    return axon_dbNew(argc, argv);
  } else if (strcmp(op, "change") == 0) {
    return axon_dbChange(argc, argv);
  } else if (strcmp(op, "migrate") == 0) {
    return (char) axon_runCommand("axon-migrator migrate");
  } else if (strcmp(op, "create") == 0) {
    return (char) axon_runCommand("axon-migrator create");
  } else if (strcmp(op, "drop") == 0) {
    return (char) axon_runCommand("axon-migrator drop");
  } else if (strcmp(op, "setup") == 0) {
    return (char) axon_runCommand("axon-migrator setup");
  } else {
    return AXON_UNKNOWN_COMMAND;
  }
}
