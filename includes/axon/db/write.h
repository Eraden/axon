#pragma once

#include <axon/utils.h>

#define AXON_CALLBACK "" \
  "#include <axon/triggers.h>\n" \
  "#include <axon/codes.h>\n\n" \
  "int %s_%llu_callback(AxonCallbackData *data);\n\n" \
  "int %s_%llu_callback(AxonCallbackData *data) {\n" \
  "  return AXON_SUCCESS;\n" \
  "}\n"

#define AXON_MIGRATIONS_FILE "./.migrations"
#define AXON_MIGRATION_PATH_LEN 2049
#define AXON_MIGRATION_FILE_NAME_LEN 1025

FILE *axon_createMigration(char **fileName, const char *pattern, const char *tableName);

void axon_createMigrationInfo(char *fileName);
