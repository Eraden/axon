#include <axon/db/exec_sequence.h>

static void axon_markAsFailure(AxonSequence *axonSequence) {
  axonSequence->errorMessage = axonSequence->axonExecContext.error;
  axonSequence->axonExecContext.error = NULL;
  axonSequence->status = AXON_EXEC_STATUS_FAILED;
}

static int axon_beginSequence(AxonSequence *axonSequence) {
  axonSequence->axonExecContext.sql = "BEGIN";
  axonSequence->axonExecContext.type = AXON_ONLY_QUERY | AXON_KEEP_CONNECTION;
  axonSequence->axonExecContext.error = NULL;
  axon_psqlExecute(&axonSequence->axonExecContext);

  /* LCOV_EXCL_START */
  if (axonSequence->axonExecContext.error) {
    axon_markAsFailure(axonSequence);
    return AXON_FAILURE;
  }
  /* LCOV_EXCL_STOP */
  return AXON_SUCCESS;
}

static int axon_endSequence(AxonSequence *axonSequence) {
  axonSequence->axonExecContext.sql = axonSequence->status == AXON_EXEC_STATUS_OK ? "END" : "ROLLBACK";
  axonSequence->axonExecContext.type = AXON_ONLY_QUERY;
  axonSequence->axonExecContext.error = NULL;
  axon_psqlExecute(&axonSequence->axonExecContext);

  /* LCOV_EXCL_START */
  if (axonSequence->axonExecContext.error) {
    axon_markAsFailure(axonSequence);
    return AXON_FAILURE;
  }
  /* LCOV_EXCL_STOP */
  return axonSequence->status == AXON_EXEC_STATUS_OK ? AXON_SUCCESS : AXON_FAILURE;
}

static int axon_runFile(AxonSequence *axonSequence, char *filePath) {
  char *sql = axon_readFile(filePath);
  axonSequence->axonExecContext.sql = sql;
  axonSequence->axonExecContext.type = AXON_ONLY_QUERY | AXON_KEEP_CONNECTION;
  int result = axon_psqlExecute(&axonSequence->axonExecContext);

  if (result != AXON_SUCCESS) {
    fprintf(stderr, "%-90s %s[FAILED]%s\n", filePath, AXON_COLOR_RED, AXON_COLOR_NRM);
    fprintf(stderr, "  aborting (all changes will be reversed)...\n\n%s[SQL]%s %s\n", AXON_COLOR_CYN, AXON_COLOR_NRM,
            sql);
    axon_markAsFailure(axonSequence);
  } else {
    fprintf(stdout, "%-90s %s[OK]%s\n", filePath, AXON_COLOR_GRN, AXON_COLOR_NRM);
  }
  free(sql);
  return result;
}

int axon_execSequence(AxonSequence *axonSequence) {
  char *connInfo = axonSequence->axonExecContext.connInfo ?
                   axonSequence->axonExecContext.connInfo :
                   axon_getConnectionInfo();
  if (connInfo == NULL) return AXON_FAILURE; /* LCOV_EXCL_LINE */

  int result = axon_beginSequence(axonSequence);
  if (result != AXON_SUCCESS) return result;

  char **files = axonSequence->files;
  while (files && *files) {
    result = axon_runFile(axonSequence, *files);
    if (result != AXON_SUCCESS) break;
    files += 1;
  }

  return axon_endSequence(axonSequence);
}

AxonSequence *axon_getSequence(char *connInfo, char **files, size_t len) {
  AxonSequence *axonSequence = calloc(sizeof(AxonSequence), 1);
  axonSequence->axonExecContext = axon_getContext(NULL, connInfo, AXON_ONLY_QUERY);
  axonSequence->status = AXON_EXEC_STATUS_OK;
  axonSequence->errorMessage = NULL;
  axonSequence->files = files;
  axonSequence->len = len;
  return axonSequence;
}

void axon_freeSequence(AxonSequence *axonSequence) {
  if (axonSequence->errorMessage) free(axonSequence->errorMessage);
  free(axonSequence);
}
