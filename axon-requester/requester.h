#pragma once

#include <axon/utils.h>
#include <axon/config.h>
#include <axon/codes.h>
#include <axon/triggers.h>
#include <axon/db/exec.h>
#include <dlfcn.h>

#define AXON_TRIGGERS_LIB_FILE "./.axon/triggers.so"
#define AXON_DRY_MODE_SQL "CREATE LOCAL TEMPORARY TABLE IF NOT EXISTS dry_run_table ( id serial );"
#define AXON_CALLBACK_FUNCTION_MAX 1024

typedef struct sAxonRequesterContext {
  char *timestamp;
  void *triggersHandle;
  AxonCallbackData *callbackData;
  AxonExecContext *execContext;
  int result;
  char *sql;
} AxonRequesterContext;

int axon_runRequester(int argc, char **argv);
