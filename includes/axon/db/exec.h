#pragma once

#include <libpq-fe.h>
#include "axon/config.h"
#include "axon/utils.h"
#include "axon/codes.h"

#define KORO_EXECUTE_SQL_BUFFER_SIZE 1024
#define KORO_CONN_INFO_SIZE 128

typedef enum eAxonExecType {
  KORO_ONLY_QUERY = 0,
  KORO_TRANSACTION_QUERY = 1 << 0,
  KORO_USE_CURSOR_QUERY = 1 << 1,
  KORO_KEEP_CONNECTION = 1 << 2
} AxonExecType;

typedef struct sAxonExecContext {
  char *sql;
  char *connInfo;
  AxonExecType type;
  PGconn *conn;
} AxonExecContext;

AxonExecContext axon_getContext(char *sql, char *connInfo, AxonExecType type);

int axon_psqlExecute(AxonExecContext *context);

char *axon_connectionInfo(AxonEnvironmentConfig *config);

char *axon_getConnectionInfo();

