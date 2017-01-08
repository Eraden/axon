#pragma once

#include <libpq-fe.h>
#include "axon/config.h"
#include "axon/utils.h"
#include "axon/codes.h"

#define AXON_EXECUTE_SQL_BUFFER_SIZE 1024
#define AXON_CONN_INFO_SIZE 128

typedef enum eAxonExecType {
  AXON_ONLY_QUERY = 0,
  AXON_TRANSACTION_QUERY = 1 << 0,
  AXON_USE_CURSOR_QUERY = 1 << 1,
  AXON_KEEP_CONNECTION = 1 << 2
} AxonExecType;

typedef struct sAxonExecContext {
  char *sql;
  char *connInfo;
  AxonExecType type;
  PGconn *conn;
  char *error;
} AxonExecContext;

AxonExecContext axon_getContext(char *sql, char *connInfo, AxonExecType type);

int axon_psqlExecute(AxonExecContext *context);

char *axon_connectionInfo(AxonEnvironmentConfig *config);

char *axon_getConnectionInfo();

