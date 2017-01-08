#include "axon/db/table.h"

static void axon_freeColumn(AxonColumnData *column) {
  if (column == NULL) return;
  if (column->name) free(column->name);
  if (column->type) free(column->type);
  if (column->defaultValue) free(column->defaultValue);
  free(column);
}

static FILE *axon_createMigration(char **fileName, const char *pattern, const char *tableName) {
  char path[AXON_MIGRATION_PATH_LEN];
  memset(path, 0, AXON_MIGRATION_PATH_LEN);
  memset(*fileName, 0, AXON_MIGRATION_FILE_NAME_LEN);
  sprintf(*fileName, pattern, (unsigned long long) time(NULL), tableName);
  sprintf(path, "./db/migrate/%s", *fileName);
  FILE *f = fopen(path, "w+");
  return f;
}

static AxonColumnData *axon_getColumn(char *rawColumn) {
  const size_t columnLen = strlen(rawColumn) + 1;

  char *columnName = calloc(sizeof(char), columnLen);
  char *columnType = calloc(sizeof(char), columnLen);
  char *columnDefault = calloc(sizeof(char), columnLen);

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
        else if (current == columnType) current = fill = columnDefault; /* LCOV_EXCL_LINE */
        else if (current == columnDefault) current = fill = NULL; /* LCOV_EXCL_LINE */
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
  column->name = columnName;
  column->type = columnType;
  column->defaultValue = columnDefault;
  return column;
}

static void axon_appendColumn(AxonTableData *table, AxonColumnData *column) {
  table->columnsCount += 1;
  table->columns = table->columns ?
                   realloc(table->columns, sizeof(AxonColumnData *) * table->columnsCount) :
                   calloc(sizeof(AxonColumnData *), table->columnsCount);
  table->columns[table->columnsCount - 1] = column;
}

static void axon_createMigrationInfo(char *fileName) {
  AxonGraph createdFiles[1] = {{.root=fileName, .len=0}};
  AxonGraph dbChildren[1] = {{.root="migrate", .len=1, .leafs=createdFiles}};
  AxonGraph db = {.root="db", .len=1, .leafs=dbChildren};
  axon_createInfo(&db);
}

char axon_dbNewTable(int argc, char **argv) {
  if (argc < 5) return 0;
  axon_ensureStructure();

  const char *name = argv[4];
  AxonTableData table;
  table.columns = NULL;
  table.columnsCount = 0;

  for (size_t i = 5; i < argc; i++) {
    char *rawColumn = argv[i];
    if (rawColumn == NULL) break;
    else if (strcmp(rawColumn, "id") == 0) {
      axon_appendColumn(&table, axon_getColumn("id:serial"));
    } else if (strcmp(rawColumn, "timestamps") == 0) {
      axon_appendColumn(&table, axon_getColumn("created_at:timestamp"));
      axon_appendColumn(&table, axon_getColumn("updated_at:timestamp"));
    } else {
      axon_appendColumn(&table, axon_getColumn(rawColumn));
    }
  }
  if (table.columns == NULL) axon_appendColumn(&table, axon_getColumn("id"));

  char *fileName = calloc(sizeof(char), AXON_MIGRATION_FILE_NAME_LEN);
  FILE *f = axon_createMigration(&fileName, "%llu_create_table_%s.sql", name);
  if (f == NULL)
    return AXON_FAILURE; /* LCOV_EXCL_LINE */

  fprintf(f, "CREATE TABLE %s (\n", name);
  for (size_t i = 0; i < table.columnsCount; i++) {
    AxonColumnData *column = table.columns[i];
    if (i > 0) fprintf(f, ",\n");
    fprintf(f, "  %s %s", column->name, column->type);
    axon_freeColumn(column);
  }
  fprintf(f, "\n);\n");
  if (table.columns) free(table.columns);
  fclose(f);

  axon_createMigrationInfo(fileName);
  free(fileName);

  return AXON_SUCCESS;
}

char axon_dbChange(int argc, char **argv) {
  if (argc < 6) return 0;
  axon_ensureStructure();

  const char *name = argv[3];
  const char *op = argv[4];
  char result;

  char *fileName = calloc(sizeof(char), AXON_MIGRATION_FILE_NAME_LEN);
  FILE *f = axon_createMigration(&fileName, "%llu_change_table_%s.sql", name);
  if (f == NULL)
    return AXON_FAILURE; /* LCOV_EXCL_LINE */
  AxonColumnData *column = axon_getColumn(argv[5]);

  fprintf(f, "ALTER TABLE %s", name);

  if (strcmp(op, "add") == 0) {
    fprintf(f, " ADD COLUMN %s %s;\n", column->name, column->type);
    result = AXON_SUCCESS;
  } else if (strcmp(op, "drop") == 0) {
    fprintf(f, " DROP COLUMN %s;", column->name);
    result = AXON_SUCCESS;
  } else {
    fprintf(stderr, "Unknown change operation '%s'\n", op);
    result = AXON_FAILURE;
  }

  fclose(f);
  axon_createMigrationInfo(fileName);
  free(fileName);

  axon_freeColumn(column);
  return result;
}
