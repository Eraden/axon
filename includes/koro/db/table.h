#pragma once

#include "koro/utils.h"
#include "koro/codes.h"

#define KORO_MIGRATION_PATH_LEN 2049
#define KORO_MIGRATION_FILE_NAME_LEN 1025

typedef struct sKoroColumnData {
  char *name;
  char *type;
  char *defaultValue;
} KoroColumnData;

typedef struct sKoroTableData {
  KoroColumnData **columns;
  size_t columnsCount;
} KoroTableData;

char koro_dbNewTable(int argc, char **argv);

char koro_dbChange(int argc, char **argv);
