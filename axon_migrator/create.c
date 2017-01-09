#include <axon/db/create.h>

char axon_isDatabaseCreate(char *str) {
  return str && strcmp(str, "create") == 0;
}

int axon_createDatabase() {
  if (axon_configExists() == 0)
    return AXON_CONFIG_MISSING;
  char *name = axon_getDatabaseName();
  char *buffer = calloc(sizeof(char), strlen("CREATE DATABASE ") + strlen(name) + 1);
  strcat(buffer, "CREATE DATABASE ");
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
