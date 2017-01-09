#include "axon/db/setup.h"

char axon_isSetup(char *str) {
  return strcmp(str, "setup") == 0;
}

int axon_setup() {
  char *connInfo = axon_getConnectionInfo();
  if (connInfo == NULL) return AXON_FAILURE;
  AxonOrder *axonOrder = axon_readOrder();
  if (axonOrder->setupFiles == NULL) return AXON_FAILURE;
  char **files = axonOrder->setupFiles;
  size_t len = 0;
  char **sequenceFiles = NULL;

  while (*files) {
    char *file = *files;
    char *path = calloc(sizeof(char), strlen(AXON_SETUP_FILES_DIRECTORY) + strlen(file) + 1);
    strcat(path, AXON_SETUP_FILES_DIRECTORY);
    strcat(path, file);
    if (axon_checkIO(path) == 0) {
      fprintf(stderr, "%-90s %s[NOT EXISTS]%s\n", path, AXON_COLOR_YEL, AXON_COLOR_NRM);
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
  axon_freeOrder(axonOrder);
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
