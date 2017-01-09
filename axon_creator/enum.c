#include <axon/db/enum.h>

static void axon_freeAxonEnum(AxonEnum *axonEnum) {
  if (axonEnum->name) free(axonEnum->name);
  char **values = axonEnum->values;
  while (values && *values) {
    if (*values) free(*values);
    values += 1;
  }
  if (axonEnum->values) free(axonEnum->values);
}

int axon_newEnum(int argc, char **argv) {
  if (argc < 5) return AXON_NOT_ENOUGH_ARGS;
  int result = axon_ensureStructure();
  if (result != AXON_SUCCESS) {
    AXON_NO_SRC_DIRECTORY_MSG
    return result;
  }
  const char *name = argv[3];
  AxonEnum axonEnum;
  axonEnum.values = NULL;
  axonEnum.len = 0;
  axonEnum.name = calloc(sizeof(char), strlen(name) + 1);
  strcpy(axonEnum.name, name);

  for (int i = 4; i < argc; i++) {
    if (argv[i] == NULL) break;
    char *arg = calloc(sizeof(char), strlen(argv[i]) + 1);
    strcpy(arg, argv[i]);
    axonEnum.len += 1;
    axonEnum.values = axonEnum.values ?
                      realloc(axonEnum.values, sizeof(char *) * (axonEnum.len + 1)) :
                      calloc(sizeof(char *), axonEnum.len + 1);
    axonEnum.values[axonEnum.len - 1] = arg;
    axonEnum.values[axonEnum.len] = NULL;
  }

  char *fileName = calloc(sizeof(char), AXON_MIGRATION_FILE_NAME_LEN);
  FILE *f = axon_createMigration(&fileName, "%llu_create_enum_%s.sql", axonEnum.name);
  if (f == NULL) return AXON_FAILURE; /* LCOV_EXCL_LINE */
  fprintf(f, "CREATE TYPE %s AS enum (", axonEnum.name);
  char **values = axonEnum.values;
  int i = 0;
  while (values && *values) {
    fprintf(f, i++ > 0 ? ",\n" : "\n");
    fprintf(f, "  '%s'", *values);
    values += 1;
  }
  fprintf(f, "\n);\n");
  fclose(f);
  free(fileName);

  axon_freeAxonEnum(&axonEnum);
  return AXON_SUCCESS;
}

int axon_dropEnum(int argc, char **argv) {
  if (argc < 4) return AXON_NOT_ENOUGH_ARGS;

  const char *name = argv[3];

  char *fileName = calloc(sizeof(char), AXON_MIGRATION_FILE_NAME_LEN);
  FILE *f = axon_createMigration(&fileName, "%llu_drop_enum_%s.sql", name);
  if (f == NULL) return AXON_FAILURE; /* LCOV_EXCL_LINE */
  fprintf(f, "DROP TYPE %s;\n", name);
  free(fileName);
  fclose(f);

  return AXON_SUCCESS;
}
