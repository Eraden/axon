#include "./reset_environment.h"

void createInitSource() {
  FILE *f;

  f = fopen("./src/db/init.c", "w+");
  if (f == NULL) {
    fprintf(stderr, "Can't write to \"./db/src/init.c\"");
    exit(EXIT_FAILURE);
  }
  fprintf(f, "%s", INIT_SOURCE_CONTENT);
  fclose(f);
}

void createInitHeader() {
  WITHIN(dummyRoot,
    FILE *f = fopen("./src/db/init.c", "w+");
    if (f == NULL) {
      fprintf(stderr, "Can't write to \"./db/src/init.c\"");
      exit(EXIT_FAILURE);
    }
    fprintf(f, "%s", INIT_HEADER_CONTENT);
    fclose(f);
  )
}

void __attribute__((__used__))
_prepare_clear_state(void) {
  GO_TO_DUMMY
  ck_unlink("./.migrations");
  ck_unlink("./log");
  ck_unlink("./db");
  ck_unlink("./src/db");

  ck_redirectStdout(
      ck_redirectStderr(
      koro_ensureStructure();
      createInitSource();
      createInitHeader();
      koro_createConfig();
      ck_dropTestDb();
      ck_createTestDb();
  )
  )
}
