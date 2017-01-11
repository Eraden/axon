#include <axon/db/seed.h>

char axon_isDatabaseSeed(char *str) {
  return strcmp(str, "seed") == 0;
}

int axon_databaseSeed() {
  char *connInfo = axon_getConnectionInfo();
  if (connInfo == NULL) return AXON_NO_CONN_INFO;
  AxonOrder *axonOrder = axon_readOrderConfig();
  if (axonOrder->seedFiles == NULL) {
    free(connInfo);
    axon_freeOrderConfig(axonOrder);
    fprintf(stdout, "%sNothing to do or order malformed!%s\n", AXON_COLOR_MAG, AXON_COLOR_NRM);
    return AXON_SUCCESS;
  }
  char **files = axonOrder->seedFiles;
  size_t len = 0;
  char **sequenceFiles = NULL;

  while (*files) {
    char *file = *files;
    char *path = calloc(sizeof(char), strlen(AXON_SEED_FILES_DIRECTORY) + strlen(file) + 1);
    strcat(path, AXON_SEED_FILES_DIRECTORY);
    strcat(path, file);
    if (axon_checkIO(path) == 0) {
      fprintf(stdout, "%-90s %s[NOT EXISTS]%s\n", path, AXON_COLOR_YEL, AXON_COLOR_NRM);
      free(path);
    } else {
      len += 1;
      sequenceFiles = sequenceFiles ?
                      realloc(sequenceFiles, sizeof(char *) * (len + 1)) :
                      calloc(sizeof(char *), len + 1);
      sequenceFiles[len - 1] = path;
      sequenceFiles[len] = 0;
    }
    files += 1;
  }
  axon_freeOrderConfig(axonOrder);
  AxonSequence *axonSequence = axon_getSequence(connInfo, sequenceFiles, len);
  int result = axon_execSequence(axonSequence);
  if (axonSequence->errorMessage) {
    fprintf(stderr, "      %s%s%s\n", AXON_COLOR_RED, axonSequence->errorMessage, AXON_COLOR_NRM);
  }
  axon_freeSequence(axonSequence);
  free(connInfo);
  char **ptr = sequenceFiles;
  while (ptr && *ptr) {
    free(*ptr);
    ptr += 1;
  }
  if (sequenceFiles) free(sequenceFiles);
  return result;
}
