#pragma once

#include <axon/utils.h>
#include <axon/codes.h>
#include <axon/db/write.h>
#include <axon/db/column.h>

typedef struct sAxonTableData {
  AxonColumnData **columns;
  size_t columnsCount;
} AxonTableData;

int axon_newTable(int argc, char **argv);

int axon_dropTable(int argc, char **argv);

int axon_renameTable(int argc, char **argv);
