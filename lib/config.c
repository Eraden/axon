#include <yaml.h>
#include <axon/config.h>

static char axon_databaseConfigExists() {
  return axon_checkIO(AXON_DATABASE_CONFIG_FILE);
}

static char axon_orderExists(void) {
  return axon_checkIO(AXON_ORDER_CONFIG_FILE);
}

static char axon_triggersExists(void) {
  return axon_checkIO(AXON_TRIGGERS_FILE);
}

char axon_configExists(void) {
  if (!axon_databaseConfigExists()) return 0;
  if (!axon_orderExists()) return 0;
  if (!axon_triggersExists()) return 0;
  return 1;
}

static int axon_createConfigurationFile(char exists, char *path, char *content) {
  if (exists) return AXON_SUCCESS;
  FILE *f = fopen(path, "w+");
  if (f == NULL) return AXON_FAILURE; /* LCOV_EXCL_LINE */

  fprintf(f, "%s", content);
  fclose(f);
  return AXON_SUCCESS;
}

static int axon_createDatabaseConfigFile(void) {
  return axon_createConfigurationFile(
      axon_databaseConfigExists(),
      AXON_DATABASE_CONFIG_FILE,
      ""
          "dev:\n"
          "  name: \"kore_dev\"\n"
          "  port: 5432\n"
          "  host: \"localhost\"\n"
          "prod:\n"
          "  name: \"kore_prod\"\n"
          "  port: 5432\n"
          "  host: \"localhost\"\n"
          "test:\n"
          "  name: \"kore_test\"\n"
          "  port: 5432\n"
          "  host: \"localhost\"\n"
  );
}

static int axon_createOrderFile(void) {
  return axon_createConfigurationFile(
      axon_orderExists(),
      AXON_ORDER_CONFIG_FILE,
      ""
          "setup:\n"
          "  - first.sql\n"
          "seed:\n"
          "  - first.sql\n"
  );
}

static int axon_createTriggersFile(void) {
  return axon_createConfigurationFile(
      axon_triggersExists(),
      AXON_TRIGGERS_FILE,
      "flags: -I./includes -L./\nlibs: axonconfig axonutils\n"
  );
}

int axon_createConfig(void) {
  int result = axon_ensureStructure();
  if (result != AXON_SUCCESS) return result;
  if (axon_configExists()) return AXON_SUCCESS;
  result = axon_createDatabaseConfigFile();
  if (result != AXON_SUCCESS) return result;
  result = axon_createOrderFile();
  if (result != AXON_SUCCESS) return result;
  result = axon_createTriggersFile();
  return result;
}

char *axon_getFlavor(void) {
  char *flavor = NULL;
  if (getenv("KORE_ENV")) {
    const char *env = getenv("KORE_ENV");
    flavor = calloc(sizeof(char), strlen(env) + 1);
    strcpy(flavor, env);
    return flavor;
  }
  // This is kore .flavor file
  FILE *f = fopen(".flavor", "r");
  if (f == NULL) {
    flavor = calloc(sizeof(char), strlen("dev") + 1);
    strcpy(flavor, "dev");
    return flavor;
  }
  while (!feof(f)) {
    char c = (char) fgetc(f);
    switch (c) {
      case ' ':
      case '\r':
      case '\n':
        fclose(f);
        return flavor;
      default: {
        if (isascii(c)) {
          const size_t len = flavor ? strlen(flavor) + 1 : 1;
          flavor = flavor ?
                   realloc(flavor, sizeof(char) * (len + 1)) :
                   calloc(sizeof(char), len + 1);
          flavor[len - 1] = c;
          flavor[len] = 0;
        }
        break;
      }
    }
  }
  setenv("KORE_ENV", flavor, 0);
  fclose(f);
  return flavor;
}

char axon_isDatabaseInitExists(void) {
  if (!axon_checkIO("src/db/init.h")) return 0;
  if (!axon_checkIO("src/db/init.c")) return 0;
  return 1;
}
