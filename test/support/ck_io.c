#include "./ck_io.h"
#include "koro/db/exec.h"

static int stderrFD;
static fpos_t stderrPos;
static int stdoutFD;
static fpos_t stdoutPos;

void ck_dropTestDb() {
  ck_redirectStderr(
      KoroExecContext context = koro_getContext("DROP DATABASE kore_test", "dbname = postgres", KORO_ONLY_QUERY);
      koro_psqlExecute(&context);
  );
}

void ck_createTestDb() {
  ck_redirectStderr(
      KoroExecContext context = koro_getContext("CREATE DATABASE kore_test", "dbname = postgres", KORO_ONLY_QUERY);
      koro_psqlExecute(&context);
  );
}

void ck_catchStderr(const char *newStream) {
  fflush(stderr);
  fgetpos(stderr, &stderrPos);
  stderrFD = dup(fileno(stderr));
  freopen(newStream, "w", stderr);
  setvbuf(stderr, NULL, _IONBF, 0);
}

void ck_releaseStderr() {
  fflush(stderr);
  dup2(stderrFD, fileno(stderr));
  close(stderrFD);
  clearerr(stderr);
  fsetpos(stderr, &stderrPos);
}

void ck_catchStdout(const char *newStream) {
  fflush(stdout);
  fgetpos(stdout, &stdoutPos);
  stdoutFD = dup(fileno(stdout));
  freopen(newStream, "w", stdout);
  setvbuf(stdout, NULL, _IONBF, 0);
}

void ck_releaseStdout() {
  fflush(stdout);
  dup2(stdoutFD, fileno(stdout));
  close(stdoutFD);
  clearerr(stdout);
  fsetpos(stdout, &stdoutPos);
}

static int ck_removeDirectory(const char *path);

static int ck_removeDirectory(const char *path) {
  DIR *d = opendir(path);
  size_t pathLen = strlen(path);
  int r = -1;

  if (d) {
    struct dirent *p;

    r = 0;

    while (!r && (p = readdir(d))) {
      int r2 = -1;
      char *buf;
      size_t len;

      /* Skip the names "." and ".." as we don't want to recurse on them. */
      if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
        continue;
      }

      len = pathLen + strlen(p->d_name) + 2;
      buf = malloc(len);

      if (buf) {
        struct stat statBuf;

        snprintf(buf, len, "%s/%s", path, p->d_name);

        if (!stat(buf, &statBuf)) {
          if (S_ISDIR(statBuf.st_mode)) {
            r2 = ck_removeDirectory(buf);
          } else {
            r2 = unlink(buf);
          }
        }

        free(buf);
      }

      r = r2;
    }

    closedir(d);
  }

  if (!r) {
    r = rmdir(path);
  }

  return r;
}

char *_ck_findFile(const char *dir, const char *pattern) {
  DIR *d = opendir(dir);
  size_t pathLen = strlen(dir);
  char *path = NULL;

  if (!d) return path;

  struct dirent *p;

  int r = 0;

  while (!r && (p = readdir(d))) {
    int r2 = -1;
    char *buf = NULL;
    size_t len;

    /* Skip the names "." and ".." as we don't want to recurse on them. */
    if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
      continue;
    }

    len = pathLen + strlen(p->d_name) + 2;
    buf = malloc(len);

    if (buf) {
      struct stat statBuf;

      snprintf(buf, len, "%s/%s", dir, p->d_name);

      if (!stat(buf, &statBuf) && S_ISREG(statBuf.st_mode) && strstr(p->d_name, pattern)) {
        path = buf;
      } else {
        free(buf);
      }
    }

    r = r2;
  }

  closedir(d);

  return path;
}

int _ck_unlink(const char *path) {
  int status;
  struct stat st_buf;
  status = stat(path, &st_buf);
  if (status != 0) return 0;
  if (S_ISREG (st_buf.st_mode)) {
    return unlink(path);
  } else if (S_ISDIR (st_buf.st_mode)) {
    return ck_removeDirectory(path);
  }
  return 0;
}

int _ck_io_check(const char *path) {
  FILE *f = fopen(path, "r");
  if (f == NULL)
    return CK_FS_MISSING;
  fclose(f);
  if (access(path, F_OK) == -1)
    return CK_FS_PERM_DENIED;
  return CK_FS_OK;
}

int _ck_io_contains(const char *path, const char *content) {
  const size_t len = strlen(content);
  int pos = 0;
  int result = _ck_io_check(path);
  if (result != CK_FS_OK) return result;
  FILE *f = fopen(path, "r");
  while (!feof(f)) {
    char expected = content[pos];
    char got = (char) fgetc(f);
    if (got == expected) pos += 1;
    else pos = 0;
    if (pos == len) break;
  }
  fclose(f);
  return pos == len;
}

void _ck_make_dummy_sql(const char *name, const char *sql, long long int timestamp) {
  ck_catchStdout("./log/info.log");
  FILE *f = NULL;
  char path[1024];
  memset(path, 0, 1024);
  sprintf(path, "./db/migrate/%lli_%s.sql", timestamp, name);
  f = fopen(path, "w+");
  fprintf(f, "%s", sql);
  fflush(f);
  fclose(f);
  ck_releaseStdout();
}