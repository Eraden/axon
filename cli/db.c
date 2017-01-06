#include "koro/db.h"

static char koro_isDbInitExists(void);

static char koro_dbInit();

static char koro_isDbInitExists(void) {
  char *fileName = "src/db/init.h";
  return koro_checkIO(fileName);
}

static char koro_dbInit() {
  if (koro_checkIO("./src") == 0) {
    fprintf(stderr, "No src directory in path!\n");
    return 0;
  }

  koro_ensureStructure();

  if (koro_checkIO("./src/db/init.c") == 0) {
    FILE *f = fopen("./src/db/init.c", "w+");
    fprintf(f, "%s", INIT_SOURCE_CONTENT);
    fclose(f);
  }
  if (koro_checkIO("./src/db/init.h") == 0) {
    FILE *f = fopen("./src/db/init.h", "w+");
    fprintf(f, "%s", INIT_HEADER_CONTENT);
    fclose(f);
  }

  printf(
      ""
          "Please modify your conf/project.conf\n"
          "  load    ./project.so db_init\n"
  );

  KoroGraph dbChildren[2] = {{.root="init.c",.len=0}, {.root="init.h",.len=0}};
  KoroGraph srcChildren[1] = {{.root="db", .leafs=dbChildren,.len=2}};
  KoroGraph koroGraph = {.root="src", .len=1, .leafs=srcChildren};
  koro_createInfo(&koroGraph);

  return 1;
}

static char koro_dbNew(int argc, char **argv) {
  if (argc < 4) return 0;
  const char *type = argv[3];

  if (strcmp(type, "table") == 0) {
    return koro_dbNewTable(argc, argv);
  } else {
    return KORO_FAILURE;
  }
}

char koro_isDB(const char *str) {
  return strcmp(str, "db") == 0;
}

char koro_dbExec(int argc, char **argv) {
  if (argc < 2) return 0;
  const char *op = argv[2];

  if (strcmp(op, "init") != 0 && koro_isDbInitExists() == 0) {
    fprintf(stderr, "DB init does not exists! Type `koro db init` to create it\n");
    return 0;
  } else if (strcmp(op, "init") == 0) {
    return koro_dbInit();
  } else if (strcmp(op, "new") == 0) {
    return koro_dbNew(argc, argv);
  } else if (strcmp(op, "change") == 0) {
    return koro_dbChange(argc, argv);
  } else if (strcmp(op, "migrate") == 0) {
    return (char) koro_runCommand("koro-migrator migrate");
  } else if (strcmp(op, "create") == 0) {
    return (char) koro_runCommand("koro-migrator create");
  } else if (strcmp(op, "drop") == 0) {
    return (char) koro_runCommand("koro-migrator drop");
  } else {
    return KORO_UNKNOWN_COMMAND;
  }
}
