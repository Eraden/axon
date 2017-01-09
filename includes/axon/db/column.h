#pragma once

#include <axon/utils.h>
#include <axon/codes.h>
#include <axon/db/write.h>

typedef struct sAxonColumnData {
  char *name;
  char *type;
  char *defaultValue;
} AxonColumnData;

AxonColumnData *axon_getColumn(char *rawColumn);

int axon_changeTable(int argc, char **argv);

void axon_freeColumn(AxonColumnData *column);
