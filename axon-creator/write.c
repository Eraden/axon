#include <axon/db/write.h>

static void axon_createCallbackFile(long long unsigned t, char *type) {
  char path[AXON_MIGRATION_PATH_LEN];
  sprintf(path, "./db/migrate/%llu_%s_callback.c", t, type);
  FILE *f = fopen(path, "w+");
  if (f == NULL) return;
  fprintf(f, AXON_CALLBACK, type, t, type, t);
  fflush(f);
  fclose(f);
}

FILE *axon_createMigration(char **fileName, const char *pattern, const char *tableName) {
  long long unsigned t = (unsigned long long) time(NULL);
  axon_createCallbackFile(t, "before");
  axon_createCallbackFile(t, "after");

  char path[AXON_MIGRATION_PATH_LEN];
  memset(path, 0, AXON_MIGRATION_PATH_LEN);
  memset(*fileName, 0, AXON_MIGRATION_FILE_NAME_LEN);
  sprintf(*fileName, pattern, t, tableName);
  sprintf(path, "./db/migrate/%s", *fileName);
  FILE *f = fopen(path, "w+");
  return f;
}

void axon_createMigrationInfo(char *fileName) {
  AxonGraph createdFiles[1] = {{.root=fileName, .len=0}};
  AxonGraph dbChildren[1] = {{.root="migrate", .len=1, .leafs=createdFiles}};
  AxonGraph db = {.root="db", .len=1, .leafs=dbChildren};
  axon_createInfo(&db);
}
