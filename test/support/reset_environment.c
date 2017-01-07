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

void _prepare_clear_state(void) {
  ck_redirectStdout(
      ck_redirectStderr(
          ck_unlink(logRoot);
      koro_ensureStructure();
      createInitSource();
      createInitHeader();
      koro_createConfig();
      ck_dropTestDb();
      ck_createTestDb();
  )
  )
}
