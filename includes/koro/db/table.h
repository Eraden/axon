#pragma once

#include "koro/utils.h"

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
