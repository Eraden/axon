#include <axon/db/write.h>

FILE *axon_createMigration(char **fileName, const char *pattern, const char *tableName) {
  char path[AXON_MIGRATION_PATH_LEN];
  memset(path, 0, AXON_MIGRATION_PATH_LEN);
  memset(*fileName, 0, AXON_MIGRATION_FILE_NAME_LEN);
  sprintf(*fileName, pattern, (unsigned long long) time(NULL), tableName);
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
