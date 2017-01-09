#pragma once

#include "axon/db/exec.h"

typedef enum eAxonExecStatus {
  AXON_EXEC_STATUS_OK,
  AXON_EXEC_STATUS_FAILED
} AxonExecStatus;

typedef struct sAxonSequence {
  char *errorMessage;
  AxonExecStatus status;
  char **files;
  size_t len;
  AxonExecContext axonExecContext;
} AxonSequence;

int axon_execSequence(AxonSequence *axonSequence);

void axon_freeSequence(AxonSequence *axonSequence);

AxonSequence *axon_getSequence(char *connInfo, char **files, size_t len);
