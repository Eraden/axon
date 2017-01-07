#pragma once

#include "axon/utils.h"

#include <yaml.h>

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

#define NO_DB_CONFIG_FOR_ENV_MSG fprintf(stderr, "No database config file for current env!\n");

void axon_createConfig(void);

void axon_createOrder(void);

char axon_configExists(void);

char axon_orderExists(void);

AxonConfig __attribute__((__malloc__))*axon_readConfig(void);

AxonEnvironmentConfig *axon_findEnvConfig(AxonConfig *config, const char *env);

AxonOrder __attribute__((__malloc__))*axon_readOrder(void);

void axon_freeOrder(AxonOrder *order);

void axon_freeConfig(AxonConfig *config);

char *axon_getFlavor(void);
