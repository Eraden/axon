#pragma once

#include <axon/utils.h>
#include <axon/codes.h>

#include <yaml.h>

typedef enum eAxonOrderTarget {
  AXON_ORDER_TARGET_NONE,
  AXON_ORDER_TARGET_SEED,
  AXON_ORDER_TARGET_SETUP
} AxonOrderTarget;

typedef enum eAxonTriggersTarget {
  AXON_TRIGGERS_TARGET_NONE,
  AXON_TRIGGERS_TARGET_LIBS,
  AXON_TRIGGERS_TARGET_FLAGS,
} AxonTriggersTarget;

typedef struct sAxonEnvironmentConfig {
  char *connectionName;
  char *host;
  int port;
  char *name;
} AxonEnvironmentConfig;

typedef struct sAxonConfig {
  char **environments;
  AxonEnvironmentConfig **configs;
  size_t len;
} AxonConfig;

typedef struct sAxonOrder {
  char **seedFiles;
  size_t seedLen;
  char **setupFiles;
  size_t setupLen;
} AxonOrder;

typedef struct sAxonTriggersConfig {
  char *libs;
  char *flags;
} AxonTriggersConfig;

#define AXON_NO_SRC_DIRECTORY_MSG fprintf(stderr, "No src directory in path!\n");
#define AXON_NO_DB_CONFIG_FOR_ENV_MSG fprintf(stderr, "No database config file for current env!\n");
#define AXON_NO_CONN_INFO fprintf(stderr, "No connection information!\n");

#define AXON_DATABASE_CONFIG_FILE "./conf/database.yml"
#define AXON_ORDER_CONFIG_FILE "./db/order.yml"
#define AXON_TRIGGERS_FILE "./conf/triggers.yml"

char axon_isDatabaseInitExists(void);

char *axon_getFlavor(void);

char axon_configExists(void);

int axon_createConfig(void);

AxonEnvironmentConfig *axon_findEnvDatabaseConfig(AxonConfig *config, const char *env);

AxonConfig __attribute__((__malloc__)) *axon_readDatabaseConfig(void);

void axon_freeDatabaseConfig(AxonConfig *config);

AxonOrder __attribute__((__malloc__)) *axon_readOrderConfig(void);

void axon_freeOrderConfig(AxonOrder *order);

AxonTriggersConfig *axon_readTriggersConfig(void);

void axon_freeTriggersConfig(AxonTriggersConfig *config);
