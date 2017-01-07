#pragma once

#include <libpq-fe.h>
#include "koro/config.h"
#include "koro/utils.h"
#include "koro/codes.h"

#define KORO_EXECUTE_SQL_BUFFER_SIZE 1024
#define KORO_CONN_INFO_SIZE 128

typedef enum eKoroExecType {
  KORO_ONLY_QUERY = 0,
  KORO_TRANSACTION_QUERY = 1 << 0,
  KORO_USE_CURSOR_QUERY = 1 << 1,
  KORO_KEEP_CONNECTION = 1 << 2
} KoroExecType;

typedef struct sKoroExecContext {
  char *sql;
  char *connInfo;
  KoroExecType type;
  PGconn *conn;
} KoroExecContext;

KoroExecContext koro_getContext(char *sql, char *connInfo, KoroExecType type);

int koro_psqlExecute(KoroExecContext *context);

char *koro_connectionInfo(KoroEnvironmentConfig *config);

char *koro_getConnectionInfo();

