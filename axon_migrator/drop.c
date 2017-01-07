#include "axon/db/drop.h"

char axon_isDatabaseDrop(char *str) {
  return str && strcmp(str, "drop") == 0;
}

int axon_dropDatabase() {
  if (axon_configExists() == 0)
    return KORO_CONFIG_MISSING;

  char *name = axon_getDatabaseName();

  char *buffer = calloc(sizeof(char), strlen("DROP DATABASE ") + strlen(name) + 1);
  strcat(buffer, "DROP DATABASE ");
  strcat(buffer, name);

  AxonExecContext context = axon_getContext(buffer, "dbname = postgres", KORO_ONLY_QUERY);

  int result = axon_psqlExecute(&context);
  free(name);
  free(buffer);
  return result;
}
