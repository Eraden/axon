#include "axon/db/exec.h"

AxonExecContext axon_getContext(char *sql, char *connInfo, AxonExecType type) {
  AxonExecContext context;
  context.connInfo = connInfo;
  context.sql = sql;
  context.type = type;
  context.conn = NULL;
  context.error = NULL;
  return context;
}

char *axon_getConnectionInfo() {
  char *env = axon_getFlavor();
  AxonConfig *config = axon_readConfig();
  AxonEnvironmentConfig *envConfig = axon_findEnvConfig(config, env);
  free(env);
  if (envConfig == NULL) {
    axon_freeConfig(config);
    NO_DB_CONFIG_FOR_ENV_MSG
    return NULL;
  }

  char *connInfo = axon_connectionInfo(envConfig);
  if (connInfo == NULL) {
    axon_freeConfig(config);
    return NULL;
  }

  axon_freeConfig(config);
  return connInfo;
}

char *axon_connectionInfo(AxonEnvironmentConfig *config) {
  if (config->name == NULL)
    return NULL;
  char *connectionInfo = calloc(sizeof(char), AXON_CONN_INFO_SIZE);
  sprintf(connectionInfo, "dbname = %s", config->name);
  return connectionInfo;
}

static void axon_exitNicely(AxonExecContext *context) {
  if (context->type & AXON_KEEP_CONNECTION)
    return;
  PGconn **conn = &context->conn;
  if (*conn == NULL) return;
  PQfinish(*conn);
  *conn = NULL;
}

int axon_psqlExecute(AxonExecContext *context) {
  if (context->connInfo == NULL && context->conn == NULL)
    return AXON_CONFIG_MISSING;
  PGconn *conn = context->conn ? context->conn : PQconnectdb(context->connInfo);
  context->conn = conn;

  if (PQstatus(conn) != CONNECTION_OK) {
    fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
    axon_exitNicely(context);
    return AXON_FAILURE;
  }

  PGresult *res = NULL;

  if (context->type & AXON_TRANSACTION_QUERY) {
    res = PQexec(conn, "BEGIN");
    /* LCOV_EXCL_START */
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
      char *psqlError = PQerrorMessage(conn);
      char *error = calloc(sizeof(char), strlen(psqlError) + 1);
      strcpy(error, psqlError);
      context->error = error;

      PQclear(res);
      axon_exitNicely(context);
      return AXON_FAILURE;
    }
    /* LCOV_EXCL_STOP */
    PQclear(res);
  }

  char *formatted = calloc(sizeof(char), AXON_EXECUTE_SQL_BUFFER_SIZE);
  if (context->type & AXON_USE_CURSOR_QUERY) strcat(formatted, "DECLARE migrator CURSOR FOR ");
  strcat(formatted, context->sql);
  res = PQexec(conn, formatted);

  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    char *psqlError = PQerrorMessage(conn);
    char *error = calloc(sizeof(char), strlen(psqlError) + 1);
    strcpy(error, psqlError);
    context->error = error;

    PQclear(res);
    free(formatted);
    axon_exitNicely(context);
    return AXON_FAILURE;
  }

  free(formatted);
  PQclear(res);

  if (context->type & AXON_USE_CURSOR_QUERY) {
    res = PQexec(conn, "FETCH ALL in migrator");
    /* LCOV_EXCL_START */
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
      char *psqlError = PQerrorMessage(conn);
      char *error = calloc(sizeof(char), strlen(psqlError) + 1);
      strcpy(error, psqlError);
      context->error = error;

      PQclear(res);
      axon_exitNicely(context);
      return AXON_FAILURE;
    }
    /* LCOV_EXCL_STOP */
    PQclear(res);
  }

  if (context->type & AXON_USE_CURSOR_QUERY) {
    res = PQexec(conn, "CLOSE migrator");
    PQclear(res);
  }

  if (context->type & AXON_TRANSACTION_QUERY) {
    res = PQexec(conn, "END");
    PQclear(res);
  }

  axon_exitNicely(context);

  return AXON_SUCCESS;
}
