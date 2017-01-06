#include "koro/db/table.h"

static void koro_freeColumn(KoroColumnData *column) {
  if (column == NULL) return;
  if (column->name) free(column->name);
  if (column->type) free(column->type);
  if (column->defaultValue) free(column->defaultValue);
  free(column);
}

static FILE *koro_createMigration(char **fileName, const char *pattern, const char *tableName) {
  char path[KORO_MIGRATION_PATH_LEN];
  memset(path, 0, KORO_MIGRATION_PATH_LEN);
  memset(*fileName, 0, KORO_MIGRATION_FILE_NAME_LEN);
  sprintf(*fileName, pattern, (unsigned long long) time(NULL), tableName);
  sprintf(path, "./db/migrate/%s", *fileName);
  FILE *f = fopen(path, "w+");
  return f;
}

static KoroColumnData *koro_getColumn(char *rawColumn) {
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
        else if (current == columnType) current = fill = columnDefault;
        else if (current == columnDefault) current = fill = NULL;
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
  KoroColumnData *column = (KoroColumnData *) calloc(sizeof(KoroColumnData), 1);
  column->name = columnName;
  column->type = columnType;
  column->defaultValue = columnDefault;
  return column;
}

static void koro_appendColumn(KoroTableData *table, KoroColumnData *column) {
  table->columnsCount += 1;
  table->columns = table->columns ?
                   realloc(table->columns, sizeof(KoroColumnData *) * table->columnsCount) :
                   calloc(sizeof(KoroColumnData *), table->columnsCount);
  table->columns[table->columnsCount - 1] = column;
}

static void koro_createMigrationInfo(char *fileName) {
  KoroGraph createdFiles[1] = {{.root=fileName, .len=0}};
  KoroGraph dbChildren[1] = {{.root="migrate", .len=1, .leafs=createdFiles}};
  KoroGraph db = {.root="db", .len=1, .leafs=dbChildren};
  koro_createInfo(&db);
}

char koro_dbNewTable(int argc, char **argv) {
  if (argc < 5) return 0;
  koro_ensureStructure();

  const char *name = argv[4];
  KoroTableData table;
  table.columns = NULL;
  table.columnsCount = 0;

  for (size_t i = 5; i < argc; i++) {
    char *rawColumn = argv[i];
    if (rawColumn == NULL) break;
    else if (strcmp(rawColumn, "id") == 0) {
      koro_appendColumn(&table, koro_getColumn("id:serial"));
    } else if (strcmp(rawColumn, "timestamps") == 0) {
      koro_appendColumn(&table, koro_getColumn("created_at:timestamp"));
      koro_appendColumn(&table, koro_getColumn("updated_at:timestamp"));
    } else {
      koro_appendColumn(&table, koro_getColumn(rawColumn));
    }
  }
  if (table.columns == NULL) koro_appendColumn(&table, koro_getColumn("id"));

  char *fileName = calloc(sizeof(char), KORO_MIGRATION_FILE_NAME_LEN);
  FILE *f = koro_createMigration(&fileName, "%llu_create_table_%s.sql", name);
  if (f == NULL)
    return KORO_FAILURE;

  fprintf(f, "CREATE TABLE %s (\n", name);
  for (size_t i = 0; i < table.columnsCount; i++) {
    KoroColumnData *column = table.columns[i];
    if (i > 0) fprintf(f, ",\n");
    fprintf(f, "  %s %s", column->name, column->type);
    koro_freeColumn(column);
  }
  fprintf(f, "\n);\n");
  if (table.columns) free(table.columns);
  fclose(f);

  koro_createMigrationInfo(fileName);
  free(fileName);

  return KORO_SUCCESS;
}

char koro_dbChange(int argc, char **argv) {
  if (argc < 6) return 0;
  koro_ensureStructure();

  const char *name = argv[3];
  const char *op = argv[4];
  char result;

  char *fileName = calloc(sizeof(char), KORO_MIGRATION_FILE_NAME_LEN);
  FILE *f = koro_createMigration(&fileName, "%llu_change_table_%s.sql", name);
  if (f == NULL)
    return KORO_FAILURE;
  KoroColumnData *column = koro_getColumn(argv[5]);

  fprintf(f, "ALTER TABLE %s", name);

  if (strcmp(op, "add") == 0) {
    fprintf(f, " ADD COLUMN %s %s;\n", column->name, column->type);
    result = KORO_SUCCESS;
  } else if (strcmp(op, "drop") == 0) {
    fprintf(f, " DROP COLUMN %s;", column->name);
    result = KORO_SUCCESS;
  } else {
    fprintf(stderr, "Unknown change operation '%s'\n", op);
    result = KORO_FAILURE;
  }

  fclose(f);
  koro_createMigrationInfo(fileName);
  free(fileName);

  koro_freeColumn(column);
  return result;
}
