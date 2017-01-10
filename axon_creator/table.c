#include <axon/db/table.h>

static void axon_appendColumn(AxonTableData *table, AxonColumnData *column) {
  table->columnsCount += 1;
  table->columns = table->columns ?
                   realloc(table->columns, sizeof(AxonColumnData *) * table->columnsCount) :
                   calloc(sizeof(AxonColumnData *), table->columnsCount);
  table->columns[table->columnsCount - 1] = column;
}

int axon_newTable(int argc, char **argv) {
  if (argc < 4) return AXON_NOT_ENOUGH_ARGS;

  const char *name = argv[3];
  AxonTableData table;
  table.columns = NULL;
  table.columnsCount = 0;

  for (size_t i = 4; i < argc; i++) {
    char *rawColumn = argv[i];
    if (rawColumn == NULL) break;
    else if (strcmp(rawColumn, "id") == 0) {
      axon_appendColumn(&table, axon_getColumn("id:serial"));
    } else if (strcmp(rawColumn, "timestamps") == 0) {
      axon_appendColumn(&table, axon_getColumn("updated_at:timestamp"));
      axon_appendColumn(&table, axon_getColumn("created_at:timestamp"));
    } else {
      axon_appendColumn(&table, axon_getColumn(rawColumn));
    }
  }
  if (table.columns == NULL) axon_appendColumn(&table, axon_getColumn("id"));

  char *fileName = calloc(sizeof(char), AXON_MIGRATION_FILE_NAME_LEN);
  FILE *f = axon_createMigration(&fileName, "%llu_create_table_%s.sql", name);
  if (f == NULL) return AXON_FAILURE; /* LCOV_EXCL_LINE */

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

int axon_dropTable(int argc, char **argv) {
  if (argc < 4) return AXON_NOT_ENOUGH_ARGS;

  const char *name = argv[3];

  char *fileName = calloc(sizeof(char), AXON_MIGRATION_FILE_NAME_LEN);
  FILE *f = axon_createMigration(&fileName, "%llu_drop_table_%s.sql", name);
  if (f == NULL) return AXON_FAILURE; /* LCOV_EXCL_LINE */
  fprintf(f, "DROP TABLE %s;\n", name);
  free(fileName);
  fclose(f);

  return AXON_SUCCESS;
}

int axon_renameTable(int argc, char **argv) {
  if (argc < 5) return AXON_NOT_ENOUGH_ARGS;

  const char *oldName = argv[3];
  const char *newName = argv[4];

  char *fileName = calloc(sizeof(char), AXON_MIGRATION_FILE_NAME_LEN);
  FILE *f = axon_createMigration(&fileName, "%llu_rename_table_%s.sql", oldName);
  if (f == NULL) return AXON_FAILURE; /* LCOV_EXCL_LINE */
  fprintf(f, "ALTER TABLE %s RENAME TO %s;\n", oldName, newName);
  free(fileName);
  fclose(f);

  return AXON_SUCCESS;
}
