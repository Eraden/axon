#include "koro/db/create.h"

static char *koro_getDatabaseName() {
  char *env = koro_getFlavor();
  KoroConfig *config = koro_readConfig();
  KoroEnvironmentConfig *envConfig = koro_findEnvConfig(config, env);
  free(env);
  if (envConfig == NULL) {
    koro_freeConfig(config);
    NO_DB_CONFIG_FOR_ENV_MSG
    return NULL;
  }

  char *name = calloc(sizeof(char), strlen(envConfig->name) + 1);
  strcat(name, envConfig->name);
  koro_freeConfig(config);
  return name;
}

char koro_isDatabaseCreate(char *str) {
  return str && strcmp(str, "create") == 0;
}

char koro_isDatabaseDrop(char *str) {
  return str && strcmp(str, "drop") == 0;
}

int koro_createDatabase() {
  if (koro_configExists() == 0)
    return KORO_CONFIG_MISSING;
  char *name = koro_getDatabaseName();
  char *buffer = calloc(sizeof(char), strlen("CREATE DATABASE ") + strlen(name) + 1);
  strcat(buffer, "CREATE DATABASE ");
  strcat(buffer, name);

  KoroExecContext context = koro_getContext(buffer, "dbname = postgres", KORO_ONLY_QUERY);

  int result = koro_psqlExecute(&context);
  free(name);
  free(buffer);
  return result;
}

int koro_dropDatabase() {
  if (koro_configExists() == 0)
    return KORO_CONFIG_MISSING;

  char *name = koro_getDatabaseName();

  char *buffer = calloc(sizeof(char), strlen("DROP DATABASE ") + strlen(name) + 1);
  strcat(buffer, "DROP DATABASE ");
  strcat(buffer, name);

  KoroExecContext context = koro_getContext(buffer, "dbname = postgres", KORO_ONLY_QUERY);

  int result = koro_psqlExecute(&context);
  free(name);
  free(buffer);
  return result;
}
