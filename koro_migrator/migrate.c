#include "koro/db/migrate.h"

static void koro_markPerformed(KoroMigration **migrations) {
  KoroMigration **ptr = migrations;
  FILE *save = fopen("./.migrations", "r");
  if (save == NULL) return;
  char *buffer = NULL;

  while (!feof(save)) {
    char c = (char) fgetc(save);

    switch (c) {
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '0': {
        const size_t len = buffer ? strlen(buffer) + 1 : 1;
        buffer = buffer ?
                 realloc(buffer, sizeof(char) * (len + 1)) :
                 calloc(sizeof(char), len + 1);
        buffer[len - 1] = c;
        buffer[len] = 0;
        break;
      }
      default: {
        if (buffer) {
          int val = atoi(buffer);
          KoroMigration **coll = ptr;
          while (coll && *coll) {
            if ((*coll)->timestamp == val) {
              (*coll)->perform = 0;
              break;
            }
            coll += 1;
          }
          free(buffer);
          buffer = NULL;
        }
      }
    }
  }

  fclose(save);
}

static void swap(KoroMigration **x, KoroMigration **y) {
  KoroMigration *temp = *x;
  *x = *y;
  *y = temp;
  return;
}

static __attribute__((__malloc__)) KoroMigration *
median3(KoroMigration **a, size_t left, size_t right) {
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

  if (d == NULL)
    return KORO_FAILURE;

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
      return KORO_FAILURE;
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
  koro_markPerformed(context->migrations);
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
  if (connInfo == NULL)
    return KORO_CONFIG_MISSING;

  KoroMigratorContext *migratorContext = koro_loadMigrations();
  KoroMigration **migrations = NULL;
  int result;
  KoroExecContext context;

  migrations = migratorContext->migrations;
  context = koro_getContext("BEGIN", connInfo, KORO_ONLY_QUERY | KORO_KEEP_CONNECTION);
  result = koro_psqlExecute(&context);

  while (result == KORO_SUCCESS && migrations && *migrations) {
    KoroMigration *migration = *migrations;
    if (!migration->perform) {
      migrations += 1;
      continue;
    }
    char *sql = kore_loadSQL(migration->path);
    if (sql == NULL) {
      result = KORO_FAILURE;
      break;
    }

    context.sql = sql;
    result = koro_psqlExecute(&context);
    if (result != KORO_SUCCESS) {
      fprintf(stderr, "\n");
      fprintf(stderr, "%-90s %s[FAILED]%s\n", migration->path, KORO_COLOR_RED, KORO_COLOR_NRM);
      fprintf(stderr, "  aborting (all changes will be reversed)...\n\n%s[SQL]%s %s\n",
              KORO_COLOR_CYN, KORO_COLOR_NRM, sql);
      break;
    } else {
      fprintf(stdout, "%-90s %s[OK]%s\n", migration->path, KORO_COLOR_GRN, KORO_COLOR_NRM);
    }
    free(sql);
    migrations += 1;
  }

  if (result == KORO_SUCCESS) {
    context.sql = "END";
    context.type = KORO_ONLY_QUERY;
    koro_psqlExecute(&context);
  } else {
    context.sql = "ROLLBACK";
    context.type = KORO_ONLY_QUERY;
    koro_psqlExecute(&context);
  }

  migrations = migratorContext->migrations;
  while (migrations && *migrations) {
    if (result != KORO_SUCCESS) break;
    if ((*migrations)->perform) {
      FILE *save = fopen("./.migrations", "a+");
      if (save) {
        fprintf(save, "%i\n", (*migrations)->timestamp);
        fclose(save);
      }
    }
    if ((*migrations)->path) free((*migrations)->path);
    free(*migrations);
    migrations += 1;
  }
  free(migratorContext->migrations);
  free(migratorContext);
  free(connInfo);

  return result;
}
