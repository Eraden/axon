#include "./prepare.h"

static char *clone_and_append(char *a, char *b) {
  const size_t len = strlen(a) + strlen(b) + 1;
  char *c = calloc(sizeof(char), len);
  strcat(c, a);
  strcat(c, b);
  return c;
}

void prepare() {
  stderrReplaced = 0;
  stdoutReplaced = 0;

  char *tmp = calloc(sizeof(char), 1024);
  getcwd(tmp, 1024);
  appRoot = calloc(sizeof(char), strlen(tmp) + 1);
  strcpy(appRoot, tmp);
  free(tmp);

  dummyRoot = clone_and_append(appRoot, "/dummy");
  logRoot = clone_and_append(dummyRoot, "/log");
  infoLogPath = clone_and_append(logRoot, "/info.log");
  errorLogPath = clone_and_append(logRoot, "/error.log");

  putenv("KORE_ENV=test");
  const size_t len = strlen(appRoot) + strlen(getenv("PATH")) + 2;
  chdir(appRoot);
  path = calloc(sizeof(char), len);
  strcat(path, getenv("PATH"));
  strcat(path, ":");
  strcat(path, appRoot);
  setenv("PATH", path, 1);

  if (_ck_io_check("./dummy") != CK_FS_OK) {
    fprintf(stderr, "No `dummy` found!\n");
    exit(EXIT_FAILURE);
  }
  GO_TO_DUMMY
  ck_unlink(logRoot);
  ck_unlink("./.migrations");
  axon_mkdir(logRoot);
  ck_dropTestDb();
}

void cleanup() {
  free(appRoot);
  free(dummyRoot);
  free(logRoot);
  free(infoLogPath);
  free(errorLogPath);
  free(path);
}
