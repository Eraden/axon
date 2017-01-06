#include "koro/utils.h"

char koro_checkIO(const char *path) {
  FILE *f = fopen(path, "r");
  if (f == NULL) return 0;
  char r = 0;
  if (access(path, F_OK) != -1) r = 1;
  fclose(f);
  return r;
}

char koro_mkdir(const char *path) {
  if (koro_checkIO(path) == 0 && mkdir(path, S_IRWXU | S_IRWXG) != 0) {
    fprintf(stderr, "No '%s' directory in path!\n", path);
    return 0;
  }
  return 1;
}

void koro_ensureStructure(void) {
  koro_mkdir("./conf");
  koro_mkdir("./src/db");
  koro_mkdir("./db");
  koro_mkdir("./db/migrate");
  koro_mkdir("./db/setup");
  koro_mkdir("./db/seed");
}

int koro_runCommand(const char *command) {
  FILE *cmd = popen(command, "r");
  while (!feof(cmd)) {
    char c = (char) fgetc(cmd);
    if (c && isascii(c)) printf("%c", c);
  }
  int result = pclose(cmd);
  result = __WEXITSTATUS(result);
  return result;
}