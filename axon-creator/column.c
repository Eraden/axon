#include <axon/db/column.h>
#include <axon/info.h>

static int axon_addColumnToTable(char **argv);

static int axon_dropColumnFromTable(char **argv);

static int axon_changeColumnType(char **argv);

AxonColumnData *axon_getColumn(char *rawColumn) {
  const size_t columnLen = strlen(rawColumn) + 1;

  char *columnName = calloc(sizeof(char), columnLen);
  char *columnType = calloc(sizeof(char), columnLen);

  if (strstr(rawColumn, ":") != NULL) {
    char *ptr = rawColumn;
    char *fill = columnName;
    char *current = columnName;
    while (ptr && *ptr && current) {
      if (*ptr != ':') {
        *fill = *ptr;
        fill += 1;
      } else {
        if (current == columnName) current = fill = columnType;
      }
      ptr += 1;
    }
  } else {
    strcpy(columnName, rawColumn);
    free(columnType);
    char isId = strcmp(rawColumn, "id") != 0;
    columnType = calloc(sizeof(char), strlen(isId ? "varchar" : "serial") + 1);
    strcpy(columnType, isId ? "varchar" : "serial");
  }
  AxonColumnData *column = (AxonColumnData *) calloc(sizeof(AxonColumnData), 1);
  column->constraints = NULL;
  column->constraintsLen = 0;
  column->name = columnName;
  if (strstr(columnType, "(") == NULL) {
    column->type = columnType;
  } else {
    column->type = calloc(sizeof(char), strlen("int") + 1);
    strcat(column->type, "int");
    AxonColumnConstraint *constraint = calloc(sizeof(AxonColumnConstraint), 1);
    constraint->type = AXON_COLUMN_CONSTRAINT_REFERENCE;
    constraint->name = NULL;
    constraint->reference = columnType;
    column->constraintsLen += 1;
    column->constraints = column->constraints ?
                          realloc(column->constraints, sizeof(AxonColumnConstraint *) * (column->constraintsLen + 1))
                                              : /* LCOV_EXCL_LINE */
                          calloc(sizeof(AxonColumnConstraint *), column->constraintsLen + 1);
    column->constraints[column->constraintsLen - 1] = constraint;
    column->constraints[column->constraintsLen] = 0;
  }
  return column;
}

int axon_changeTable(int argc, char **argv) {
  if (argc < 5) return AXON_NOT_ENOUGH_ARGS;
  int result = axon_ensureStructure();
  if (result != AXON_SUCCESS) {
    AXON_NO_SRC_DIRECTORY_MSG
    return result;
  }

  const char *op = argv[3];

  if (strcmp(op, "add") == 0) {
    return axon_addColumnToTable(argv);
  } else if (strcmp(op, "drop") == 0) {
    return axon_dropColumnFromTable(argv);
  } else if (strcmp(op, "retype") == 0) {
    return axon_changeColumnType(argv);
  } else {
    axon_creator_info();
    result = AXON_UNKNOWN_COMMAND;
  }
  return result;
}

static int axon_addColumnToTable(char **argv) {
  const char *name = argv[2];

  char *fileName = calloc(sizeof(char), AXON_MIGRATION_FILE_NAME_LEN);
  FILE *f = axon_createMigration(&fileName, "%llu_add_column_to_table_%s.sql", name);
  /* LCOV_EXCL_START */
  if (f == NULL) {
    free(fileName);
    return AXON_FAILURE;
  }
  /* LCOV_EXCL_STOP */
  AxonColumnData *column = axon_getColumn(argv[4]);

  fprintf(f, "ALTER TABLE %s", name);
  fprintf(f, " ADD COLUMN %s %s;\n", column->name, column->type);

  fclose(f);
  axon_createMigrationInfo(fileName);
  free(fileName);

  axon_freeColumn(column);
  return AXON_SUCCESS;
}

static int axon_dropColumnFromTable(char **argv) {
  const char *name = argv[2];

  char *fileName = calloc(sizeof(char), AXON_MIGRATION_FILE_NAME_LEN);
  FILE *f = axon_createMigration(&fileName, "%llu_drop_column_from_table_%s.sql", name);
  /* LCOV_EXCL_START */
  if (f == NULL) {
    free(fileName);
    return AXON_FAILURE;
  }
  /* LCOV_EXCL_STOP */
  AxonColumnData *column = axon_getColumn(argv[4]);

  fprintf(f, "ALTER TABLE %s", name);
  fprintf(f, " DROP COLUMN %s;", column->name);

  fclose(f);
  axon_createMigrationInfo(fileName);
  free(fileName);

  axon_freeColumn(column);
  return AXON_SUCCESS;
}

static int axon_changeColumnType(char **argv) {
  const char *name = argv[2];

  char *fileName = calloc(sizeof(char), AXON_MIGRATION_FILE_NAME_LEN);
  FILE *f = axon_createMigration(&fileName, "%llu_change_column_type_%s.sql", name);
  /* LCOV_EXCL_START */
  if (f == NULL) {
    free(fileName);
    return AXON_FAILURE;
  }
  /* LCOV_EXCL_STOP */
  AxonColumnData *column = axon_getColumn(argv[4]);

  fprintf(f, "ALTER TABLE %s", name);
  fprintf(f, " ALTER COLUMN %s TYPE %s;\n", column->name, column->type);

  fclose(f);
  axon_createMigrationInfo(fileName);
  free(fileName);

  axon_freeColumn(column);
  return AXON_SUCCESS;
}

static void axon_freeColumnConstraint(AxonColumnConstraint *constraint) {
  if (constraint == NULL) return;
  if (constraint->name) free(constraint->name);
  switch (constraint->type) {
    case AXON_COLUMN_CONSTRAINT_REFERENCE:
      if (constraint->reference) free(constraint->reference);
      break;
    case AXON_COLUMN_CONSTRAINT_NONE:
      break; /* LCOV_EXCL_LINE */
  }
  free(constraint);
}

void axon_freeColumn(AxonColumnData *column) {
  if (column == NULL) return;
  if (column->name) free(column->name);
  if (column->type) free(column->type);
  AxonColumnConstraint **constraints = column->constraints;
  while (constraints && *constraints) {
    axon_freeColumnConstraint(*constraints);
    constraints += 1;
  }
  if (column->constraints) free(column->constraints);
  free(column);
}
