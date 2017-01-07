#include "koro/db/exec.h"

KoroExecContext koro_getContext(char *sql, char *connInfo, KoroExecType type) {
  KoroExecContext context;
  context.connInfo = connInfo;
  context.sql = sql;
  context.type = type;
  context.conn = NULL;
  return context;
}

char *koro_getConnectionInfo() {
  char *env = koro_getFlavor();
  KoroConfig *config = koro_readConfig();
  KoroEnvironmentConfig *envConfig = koro_findEnvConfig(config, env);
  free(env);
  if (envConfig == NULL) {
    koro_freeConfig(config);
    NO_DB_CONFIG_FOR_ENV_MSG
    return NULL;
  }

  char *connInfo = koro_connectionInfo(envConfig);
  if (connInfo == NULL) {
    koro_freeConfig(config);
    return NULL;
  }

  koro_freeConfig(config);
  return connInfo;
}

char *koro_connectionInfo(KoroEnvironmentConfig *config) {
  if (config->name == NULL)
    return NULL;
  char *connectionInfo = calloc(sizeof(char), KORO_CONN_INFO_SIZE);
  sprintf(connectionInfo, "dbname = %s", config->name);
  return connectionInfo;
}

static void koro_exitNicely(KoroExecContext *context) {
  if (context->type & KORO_KEEP_CONNECTION)
    return;
  PGconn **conn = &context->conn;
  if (*conn == NULL) return;
  PQfinish(*conn);
  *conn = NULL;
}

int koro_psqlExecute(KoroExecContext *context) {
  if (context->connInfo == NULL && context->conn == NULL)
    return KORO_CONFIG_MISSING;
  PGconn *conn = context->conn ? context->conn : PQconnectdb(context->connInfo);
  context->conn = conn;

  if (PQstatus(conn) != CONNECTION_OK) {
    fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
    koro_exitNicely(context);
    return KORO_FAILURE;
  }

  PGresult *res = NULL;

  if (context->type & KORO_TRANSACTION_QUERY) {
    res = PQexec(conn, "BEGIN");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
      fprintf(stderr, "BEGIN command failed: %s\n", PQerrorMessage(conn));
      PQclear(res);
      koro_exitNicely(context);
      return KORO_FAILURE;
    }
    PQclear(res);
  }

  char *formatted = calloc(sizeof(char), KORO_EXECUTE_SQL_BUFFER_SIZE);
  if (context->type & KORO_USE_CURSOR_QUERY) strcat(formatted, "DECLARE migrator CURSOR FOR ");
  strcat(formatted, context->sql);
  res = PQexec(conn, formatted);

  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    char *reason = PQerrorMessage(conn);
    fprintf(stderr, "Execute sql failed: '%s'\n", reason);
    PQclear(res);
    free(formatted);
    koro_exitNicely(context);
    return KORO_FAILURE;
  }

  free(formatted);
  PQclear(res);

  if (context->type & KORO_USE_CURSOR_QUERY) {
    res = PQexec(conn, "FETCH ALL in migrator");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
      fprintf(stderr, "FETCH ALL failed: '%s'\n", PQerrorMessage(conn));
      PQclear(res);
      koro_exitNicely(context);
      return KORO_FAILURE;
    }
    PQclear(res);
  }

  if (context->type & KORO_USE_CURSOR_QUERY) {
    res = PQexec(conn, "CLOSE migrator");
    PQclear(res);
  }

  if (context->type & KORO_TRANSACTION_QUERY) {
    res = PQexec(conn, "END");
    PQclear(res);
  }

  koro_exitNicely(context);

  return KORO_SUCCESS;
}
