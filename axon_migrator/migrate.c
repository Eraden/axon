#include "axon/db/migrate.h"

static void axon_markPerformed(AxonMigration **migrations) {
  AxonMigration **ptr = migrations;
  FILE *save = fopen(AXON_MIGRATIONS_FILE, "r");
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
          AxonMigration **coll = ptr;
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

static void swap(AxonMigration **x, AxonMigration **y) {
  AxonMigration *temp = *x;
  *x = *y;
  *y = temp;
  return;
}

static __attribute__((__malloc__)) AxonMigration *
median3(AxonMigration **a, size_t left, size_t right) {
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

static int quicksort(AxonMigration **a, size_t left, size_t right) {
  if (left < right) {
    AxonMigration *pivot = median3(a, left, right);
    if (left == right - 1)
      return AXON_SUCCESS;
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

  return AXON_SUCCESS;
}

static int axon_fetchTimestamp(char *buf) {
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

static int kore_loadMigrationFiles(AxonMigratorContext *context) {
  const char *path = "./db/migrate";
  DIR *d = opendir(path);
  size_t pathLen = strlen(path);

  if (d == NULL) {
    return AXON_FAILURE;
  }

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
      /* LCOV_EXCL_START */
      fprintf(stderr, "%s\n", strerror(errno));
      return AXON_FAILURE;
      /* LCOV_EXCL_STOP */
    }

    struct stat statBuf;
    snprintf(buf, len, "%s/%s", path, p->d_name);

    char isFile = !stat(buf, &statBuf) && S_ISREG(statBuf.st_mode);
    if (isFile && strstr(p->d_name, ".sql") != NULL) {
      AxonMigration *migration = calloc(sizeof(AxonMigration), 1);
      migration->path = buf;
      migration->perform = 1;
      migration->timestamp = axon_fetchTimestamp(buf);

      context->len += 1;
      context->migrations = context->migrations ?
                            realloc(context->migrations, sizeof(AxonMigration *) * (context->len + 1)) :
                            calloc(sizeof(AxonMigration *), context->len + 1);
      context->migrations[context->len - 1] = migration;
      context->migrations[context->len] = 0;
      p = readdir(d);
    } else if (isFile) {
      free(buf);
      p = readdir(d);
    } else {
      /* LCOV_EXCL_START */
      free(buf);
      p = NULL;
      /* LCOV_EXCL_STOP */
    }
  }

  closedir(d);

  quicksort(context->migrations, 0, context->len - 1);

  return AXON_SUCCESS;
}

AxonMigratorContext *axon_loadMigrations(void) {
  AxonMigratorContext *context = calloc(sizeof(AxonMigratorContext), 1);
  if (kore_loadMigrationFiles(context) == AXON_SUCCESS) {
    axon_markPerformed(context->migrations);
  }
  return context;
}

void axon_freeMigrations(AxonMigratorContext *migratorContext, const char writeToSave) {
  AxonMigration **migrations = migratorContext->migrations;
  while (migrations && *migrations) {
    if (writeToSave && (*migrations)->perform) {
      FILE *save = fopen(AXON_MIGRATIONS_FILE, "a+");
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
}

char axon_isMigrate(const char *arg) {
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

int axon_migrate() {
  char *connInfo = axon_getConnectionInfo();
  if (connInfo == NULL)
    return AXON_CONFIG_MISSING;

  AxonMigratorContext *migratorContext = axon_loadMigrations();
  AxonMigration **migrations = NULL;
  int result;
  AxonExecContext context;

  migrations = migratorContext->migrations;
  context = axon_getContext("BEGIN", connInfo, AXON_ONLY_QUERY | AXON_KEEP_CONNECTION);
  result = axon_psqlExecute(&context);

  while (result == AXON_SUCCESS && migrations && *migrations) {
    AxonMigration *migration = *migrations;
    if (!migration->perform) {
      migrations += 1;
      continue;
    }
    char *sql = kore_loadSQL(migration->path);
    if (sql == NULL) {
      /* LCOV_EXCL_START */
      result = AXON_FAILURE;
      break;
      /* LCOV_EXCL_STOP */
    }

    context.sql = sql;
    result = axon_psqlExecute(&context);
    if (result != AXON_SUCCESS) {
      fprintf(stderr, "\n");
      fprintf(stderr, "%-90s %s[FAILED]%s\n", migration->path, AXON_COLOR_RED, AXON_COLOR_NRM);
      fprintf(stderr, "  aborting (all changes will be reversed)...\n\n%s[SQL]%s %s\n",
              AXON_COLOR_CYN, AXON_COLOR_NRM, sql);
      free(sql);
      break;
    } else {
      fprintf(stdout, "%-90s %s[OK]%s\n", migration->path, AXON_COLOR_GRN, AXON_COLOR_NRM);
    }
    free(sql);
    migrations += 1;
  }

  if (result == AXON_SUCCESS) {
    context.sql = "END";
    context.type = AXON_ONLY_QUERY;
    axon_psqlExecute(&context);
  } else {
    context.sql = "ROLLBACK";
    context.type = AXON_ONLY_QUERY;
    axon_psqlExecute(&context);
  }

  axon_freeMigrations(migratorContext, result == AXON_SUCCESS);
  free(connInfo);

  return result;
}
