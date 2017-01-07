#include "axon/utils.h"

static const u_int32_t KORO_LAST_LEAF = L'└';
static const u_int32_t KORO_INTERSECTION_LEAF = L'├';
static const u_int32_t KORO_PATH_TO_LEAF = L'─';
static const size_t KORO_PATH_LEN = 3;

char axon_checkIO(const char *path) {
  FILE *f = fopen(path, "r");
  if (f == NULL) return 0;
  char r = 0;
  if (access(path, F_OK) != -1) r = 1;
  fclose(f);
  return r;
}

char axon_mkdir(const char *path) {
  if (axon_checkIO(path) == 0 && mkdir(path, S_IRWXU | S_IRWXG) != 0) {
    fprintf(stderr, "No '%s' directory in path!\n", path);
    return 0;
  }
  return 1;
}

char axon_touch(const char *path) {
  const size_t len = strlen(path);
  char *p = calloc(sizeof(char), len + 1);
  strcpy(p, path);
  for (int i = (int) len; i >= 0; --i) {
    if (p[i] == '/') {
      p[i] = 0;
      break;
    } else {
      p[i] = 0;
    }
  }
  int res = axon_mkdir(p);
  free(p);
  if (res == 0) return 0;
  FILE *f = fopen(path, "a+");
  if (f == NULL) return 0;
  fflush(f);
  fclose(f);

  return 1;
}

void axon_ensureStructure(void) {
  axon_mkdir("./conf");
  axon_mkdir("./src");
  axon_mkdir("./src/db");
  axon_mkdir("./db");
  axon_mkdir("./db/migrate");
  axon_mkdir("./db/setup");
  axon_mkdir("./db/seed");
}

static char *axon_cpyEnv(char *buffer, char *name) {
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

int axon_runCommand(const char *command) {
  char *flavor = axon_getFlavor();
  free(flavor);
  char *runCmd = calloc(sizeof(char), strlen(command) + 1);
  strcpy(runCmd, command);
  runCmd = axon_cpyEnv(runCmd, "KORO_ENV");
  runCmd = axon_cpyEnv(runCmd, "PATH");
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

void axon_createInfo(AxonGraph *axonGraph) {
  fprintf(stdout, "Files created:\n");
  axon_drawGraph(axonGraph, 0);
}

void axon_drawGraph(AxonGraph *axonGraph, size_t indent) {
  fprintf(stdout, "%s\n", axonGraph->root);
  const size_t len = axonGraph->len;
  for (int i = 0; i < len; i++) {
    char isIntersection = i < len - 1;
    const size_t printIndent = indent + strlen(axonGraph->root) - 1;
    for (int indentIndex = 0; indentIndex < printIndent; indentIndex++) {
      fprintf(stdout, " ");
    }
    if (isIntersection) fprintf(stdout, "%lc", KORO_INTERSECTION_LEAF);
    else fprintf(stdout, "%lc", KORO_LAST_LEAF);
    for (size_t l = 0; l < 2; l++) fprintf(stdout, "%lc", KORO_PATH_TO_LEAF);
    axon_drawGraph(&axonGraph->leafs[i], printIndent + KORO_PATH_LEN);
  }
}

char *axon_getDatabaseName() {
  char *env = axon_getFlavor();
  AxonConfig *config = axon_readConfig();
  AxonEnvironmentConfig *envConfig = axon_findEnvConfig(config, env);
  free(env);
  if (envConfig == NULL) {
    axon_freeConfig(config);
    NO_DB_CONFIG_FOR_ENV_MSG
    return NULL;
  }

  char *name = calloc(sizeof(char), strlen(envConfig->name) + 1);
  strcat(name, envConfig->name);
  axon_freeConfig(config);
  return name;
}
