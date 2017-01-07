#include "koro/utils.h"

static const u_int32_t KORO_LAST_LEAF = L'└';
static const u_int32_t KORO_INTERSECTION_LEAF = L'├';
static const u_int32_t KORO_PATH_TO_LEAF = L'─';
static const size_t KORO_PATH_LEN = 3;

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
  koro_mkdir("./src");
  koro_mkdir("./src/db");
  koro_mkdir("./db");
  koro_mkdir("./db/migrate");
  koro_mkdir("./db/setup");
  koro_mkdir("./db/seed");
}

static char *koro_cpyEnv(char *buffer, char *name) {
  char *val = getenv(name);
  if (val == NULL) return buffer;
  const size_t len = strlen(buffer) + strlen(name) + strlen("=") + strlen(val) + strlen(" ") + 1;
  char *ptr = calloc(sizeof(char), len);
  strcat(ptr, name);
  strcat(ptr, "=");
  strcat(ptr, val);
  strcat(ptr, " ");
  strcat(ptr, buffer);
  free(buffer);
  return ptr;
}

int koro_runCommand(const char *command) {
  char *flavor = koro_getFlavor();
  free(flavor);
  char *runCmd = calloc(sizeof(char), strlen(command) + 1);
  strcpy(runCmd, command);
  runCmd = koro_cpyEnv(runCmd, "KORO_ENV");
  runCmd = koro_cpyEnv(runCmd, "PATH");
  FILE *cmd = popen(runCmd, "r");
  free(runCmd);
  while (!feof(cmd)) {
    char c = (char) fgetc(cmd);
    if (c && isascii(c)) printf("%c", c);
  }
  int result = pclose(cmd);
  result = __WEXITSTATUS(result);
  return result;
}

void koro_createInfo(KoroGraph *koroGraph) {
  fprintf(stdout, "Files created:\n");
  koro_drawGraph(koroGraph, 0);
}

void koro_drawGraph(KoroGraph *koroGraph, size_t indent) {
  fprintf(stdout, "%s\n", koroGraph->root);
  const size_t len = koroGraph->len;
  for (int i = 0; i < len; i++) {
    char isIntersection = i < len - 1;
    const size_t printIndent = indent + strlen(koroGraph->root) - 1;
    for (int indentIndex = 0; indentIndex < printIndent; indentIndex++) {
      fprintf(stdout, " ");
    }
    if (isIntersection) fprintf(stdout, "%lc", KORO_INTERSECTION_LEAF);
    else fprintf(stdout, "%lc", KORO_LAST_LEAF);
    for (size_t l = 0; l < 2; l++) fprintf(stdout, "%lc", KORO_PATH_TO_LEAF);
    koro_drawGraph(&koroGraph->leafs[i], printIndent + KORO_PATH_LEN);
  }
}
