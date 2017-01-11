#pragma once

#include <axon/utils.h>
#include <axon/codes.h>
#include <axon/db/write.h>

typedef enum eAxonColumnConstraintType {
  AXON_COLUMN_CONSTRAINT_NONE,
  AXON_COLUMN_CONSTRAINT_REFERENCE,
} AxonColumnConstraintType;

typedef struct sAxonColumnConstraint {
  char *name;
  union {
    char *check;
    char *reference;
    char uniq;
  };
  AxonColumnConstraintType type;
} AxonColumnConstraint;

typedef struct sAxonColumnData {
  char *name;
  char *type;
  AxonColumnConstraint **constraints;
  size_t constraintsLen;
} AxonColumnData;

AxonColumnData *axon_getColumn(char *rawColumn);

int axon_changeTable(int argc, char **argv);

void axon_freeColumn(AxonColumnData *column);
