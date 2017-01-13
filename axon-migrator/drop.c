#include <axon/db/drop.h>

char axon_isDatabaseDrop(char *str) {
  return str && strcmp(str, "drop") == 0;
}

int axon_dropDatabase() {
  if (axon_configExists() == 0)
    return AXON_CONFIG_MISSING;

  char *name = axon_getDatabaseName();
  if (name == NULL) return AXON_FAILURE; /* LCOV_EXCL_LINE */

  char *buffer = calloc(sizeof(char), strlen("DROP DATABASE ") + strlen(name) + 1);
  strcat(buffer, "DROP DATABASE ");
  strcat(buffer, name);

  AxonExecContext context = axon_getContext(buffer, "dbname = postgres", AXON_ONLY_QUERY);

  int result = axon_psqlExecute(&context);
  /* LCOV_EXCL_START */
  if (context.error) {
    fprintf(stderr, "%s\n", context.error);
    free(context.error);
  }
  /* LCOV_EXCL_STOP */
  free(name);
  free(buffer);
  return result;
}
