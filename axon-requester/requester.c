#include "./requester.h"

static char *axon_getSQL(const char *str) {
  char *sql = NULL;
  if (strcmp(str, "--dry") == 0) {
    sql = calloc(sizeof(char), strlen(AXON_DRY_MODE_SQL) + 1);
    strcpy(sql, AXON_DRY_MODE_SQL);
  } else {
    sql = axon_readFile(str);
  }
  return sql;
}

static char *axon_getCallbackName(const char *timestamp, const char *type) {
  char *buffer = calloc(sizeof(char), AXON_CALLBACK_FUNCTION_MAX);
  sprintf(buffer, "%s_%s_callback", type, timestamp);
  return buffer;
}

static int
axon_runCallback(const char *type, AxonRequesterContext *context) {
  if (context->triggersHandle == NULL) return AXON_SUCCESS;

  int (*callback)(AxonCallbackData *);
  int result;
  char *name = axon_getCallbackName(context->timestamp, type);
  callback = dlsym(context->triggersHandle, name);
  if (callback) {
    result = (*callback)(context->callbackData);
  } else {
    result = AXON_SUCCESS;
  }
  free(name);
  return result;
}

static int axon_runBeforeCallback(AxonRequesterContext *context) {
  return axon_runCallback("before", context);
}

static int axon_runAfterCallback(AxonRequesterContext *context) {
  return axon_runCallback("after", context);
}

static int axon_beginRequest(AxonRequesterContext *context) {
  int result;
  context->execContext->sql = "BEGIN";
  context->execContext->type = AXON_KEEP_CONNECTION;
  result = axon_psqlExecute(context->execContext);

  if (context->execContext->error) {
    /* LCOV_EXCL_START */
    fprintf(stderr, "%s\n", context->execContext->error);
    free(context->execContext->error);
    context->execContext->error = NULL;
    /* LCOV_EXCL_STOP */
  }
  return result;
}

static int axon_endRequest(AxonRequesterContext *context) {
  int result;
  char valid = context->result == AXON_SUCCESS;
  context->execContext->sql = valid ? "END" : "ROLLBACK";
  context->execContext->type = AXON_ONLY_QUERY;
  result = axon_psqlExecute(context->execContext);
  if (context->execContext->error) {
    /* LCOV_EXCL_START */
    fprintf(stderr, "%s\n", context->execContext->error);
    free(context->execContext->error);
    context->execContext->error = NULL;
    /* LCOV_EXCL_STOP */
  }
  if (valid) context->result = result;
  return context->result;
}

static int axon_sendSQL(AxonRequesterContext *context) {
  int result;
  context->execContext->sql = context->sql;
  result = axon_psqlExecute(context->execContext);
  if (context->execContext->error) {
    /* LCOV_EXCL_START */
    fprintf(stderr, "%s\n", context->execContext->error);
    free(context->execContext->error);
    context->execContext->error = NULL;
    /* LCOV_EXCL_STOP */
  }
  return result;
}

static AxonRequesterContext *axon_getRequesterContext(char *timestamp, const char *fileOrMode) {
  AxonRequesterContext *context = calloc(sizeof(AxonRequesterContext), 1);
  char *libraryPath = realpath(AXON_TRIGGERS_LIB_FILE, NULL);
  context->triggersHandle = dlopen(libraryPath, RTLD_LAZY);
  free(libraryPath);
  if (context->triggersHandle == NULL)
    fprintf(stderr, "Dynamic linking file could not be loaded:\n  %s\n", dlerror()); /* LCOV_EXCL_LINE */
  context->sql = axon_getSQL(fileOrMode);
  context->timestamp = timestamp;
  context->callbackData = calloc(sizeof(AxonCallbackData), 1);
  context->callbackData->flags = 0;
  context->callbackData->len = 0;
  context->callbackData->payload = 0;
  context->callbackData->freePayload = NULL;
  context->callbackData->sql = context->sql;
  context->execContext = calloc(sizeof(AxonExecContext), 1);
  context->execContext->connInfo = axon_getConnectionInfo();
  return context;
}

static void axon_freeRequesterContext(AxonRequesterContext *context) {
  if (context->callbackData->sql != context->sql)
    free(context->callbackData->sql);
  if (context->callbackData->payload && context->callbackData->freePayload)
    context->callbackData->freePayload(context->callbackData->payload);

  free(context->sql);
  free(context->callbackData);

  if (context->triggersHandle)
    dlclose(context->triggersHandle);
  if (context->execContext) {
    if (context->execContext->connInfo)
      free(context->execContext->connInfo);
    free(context->execContext);
  }
  free(context);
}

int axon_runRequester(int argc, char **argv) {
  if (argc < 3) return AXON_NOT_ENOUGH_ARGS;
  const char *timestamp = argv[1];
  if (atoi(timestamp) == 0)
    return AXON_INVALID_ARG_TYPE;

  AxonRequesterContext *context = axon_getRequesterContext(argv[1], argv[2]);

  context->result = axon_beginRequest(context);
  if (context->result == AXON_SUCCESS)
    context->result = axon_runBeforeCallback(context);
  if (context->result == AXON_SUCCESS)
    context->result = axon_sendSQL(context);
  if (context->result == AXON_SUCCESS)
    context->result = axon_runAfterCallback(context);
  context->result = axon_endRequest(context);

  const int result = context->result;
  axon_freeRequesterContext(context);

  return result;
}
