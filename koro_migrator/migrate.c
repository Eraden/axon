#include "koro/db/migrate.h"

static void swap(KoroMigration **x, KoroMigration **y) {
  KoroMigration *temp = *x;
  *x = *y;
  *y = temp;
  return;
}

static KoroMigration *median3(KoroMigration **a, size_t left, size_t right) {
  size_t center = (left + right) / 2;
  if (a[center]->timestamp < a[left]->timestamp)
    swap(&a[left], &a[center]);
  if (a[right]->timestamp < a[left]->timestamp)
    swap(&a[left], &a[right]);
  if (a[right]->timestamp < a[center]->timestamp)
    swap(&a[center], &a[right]);
  swap(&a[center], &a[right - 1]);
  return a[right - 1];
}

static int quicksort(KoroMigration **a, size_t left, size_t right) {
  if (left < right) {
    KoroMigration *pivot = median3(a, left, right);
    if (left == right - 1)
      return KORO_SUCCESS;
    size_t i = left;
    size_t j = right - 1;
    for (;;) {
      while (a[++i]->timestamp < pivot->timestamp) {}
      while (pivot->timestamp < a[--j]->timestamp) {}
      if (i < j)
        swap(&a[i], &a[j]);
      else
        break;
    }
    swap(&a[i], &a[right - 1]);
    quicksort(a, left, i - 1);
    quicksort(a, i + 1, right);
  }
  return KORO_SUCCESS;
}

static int koro_fetchTimestamp(char *buf) {
  buf += strlen("./db/migrate/");
  char *num = calloc(sizeof(char), strlen(buf) + 1);
  char *ptr = num;
  while (buf && *buf) {
    switch (*buf) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        *ptr = *buf;
        ptr += 1;
        buf += 1;
        break;
      }
      default:
        buf = NULL;
    }
  }
  int i = num ? atoi(num) : 0;
  if (num) free(num);
  return i;
}

static int kore_loadMigrationFiles(KoroMigratorContext *context) {
  const char *path = "./db/migrate";
  DIR *d = opendir(path);
  size_t pathLen = strlen(path);

  if (d == NULL) return KORO_FAILURE;

  struct dirent *p = readdir(d);

  while (p) {
    char *buf = 0;
    size_t len = 0;
    if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
      p = readdir(d);
      continue;
    }

    len = pathLen + strlen(p->d_name) + 2;
    buf = malloc(len);

    if (buf == NULL) {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(KORO_FAILURE);
    }

    struct stat statBuf;
    snprintf(buf, len, "%s/%s", path, p->d_name);

    if (!stat(buf, &statBuf) && S_ISREG(statBuf.st_mode) && strstr(p->d_name, ".sql") != NULL) {
      KoroMigration *migration = calloc(sizeof(KoroMigration), 1);
      migration->path = buf;
      migration->perform = 1;
      migration->timestamp = koro_fetchTimestamp(buf);

      context->len += 1;
      context->migrations = context->migrations ?
                            realloc(context->migrations, sizeof(KoroMigration *) * (context->len + 1)) :
                            calloc(sizeof(KoroMigration *), context->len + 1);
      context->migrations[context->len - 1] = migration;
      context->migrations[context->len] = 0;
      p = readdir(d);
    } else {
      free(buf);
      p = NULL;
    }
  }

  closedir(d);

  quicksort(context->migrations, 0, context->len - 1);

  return KORO_SUCCESS;
}

static KoroMigratorContext *koro_loadMigrations() {
  KoroMigratorContext *context = calloc(sizeof(KoroMigratorContext), 1);
  kore_loadMigrationFiles(context);
  return context;
}

char koro_isMigrate(const char *arg) {
  return strcmp(arg, "migrate") == 0;
}

static char *kore_loadSQL(char *path) {
  FILE *f = fopen(path, "r");
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  char *buffer = calloc(sizeof(char), (size_t) (len + 1));
  rewind(f);
  fread(buffer, sizeof(char), (size_t) len, f);
  fclose(f);
  return buffer;
}

int koro_migrate() {
  char *connInfo = koro_getConnectionInfo();
  KoroMigratorContext *migratorContext = koro_loadMigrations();
  KoroMigration **migrations = NULL;
  int result;
  KoroExecContext context;

  migrations = migratorContext->migrations;
  context = koro_getContext("BEGIN", connInfo, KORO_ONLY_QUERY | KORO_KEEP_CONNECTION);
  result = koro_psqlExecute(&context);

  while (result == KORO_SUCCESS && migrations && *migrations) {
    char *sql = kore_loadSQL((*migrations)->path);
    if (sql) {
      context.sql = sql;
      result = koro_psqlExecute(&context);
      free(sql);
    }
    migrations += 1;
  }
  free(connInfo);

  context.sql = "END";
  context.type = KORO_ONLY_QUERY;
  koro_psqlExecute(&context);

  migrations = migratorContext->migrations;
  while (migrations && *migrations) {
    if ((*migrations)->path) free((*migrations)->path);
    free(*migrations);
    migrations += 1;
  }
  free(migratorContext->migrations);
  free(migratorContext);

  return result;
}
