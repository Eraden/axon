#pragma once

#include "axon/utils.h"
#include "axon/codes.h"

#define KORO_MIGRATION_PATH_LEN 2049
#define KORO_MIGRATION_FILE_NAME_LEN 1025

typedef struct sAxonColumnData {
  char *name;
  char *type;
  char *defaultValue;
} AxonColumnData;

typedef struct sAxonTableData {
  AxonColumnData **columns;
  size_t columnsCount;
} AxonTableData;

char axon_dbNewTable(int argc, char **argv);

char axon_dbChange(int argc, char **argv);
