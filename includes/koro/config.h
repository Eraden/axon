#pragma once

#include "koro/utils.h"

#include <yaml.h>

typedef struct sKoroEnvironmentConfig {
  char *connectionName;
  char *host;
  int port;
  char *name;
} KoroEnvironmentConfig;

typedef struct sKoroConfig {
  char **environments;
  KoroEnvironmentConfig **configs;
  size_t len;
} KoroConfig;

#define NO_DB_CONFIG_MSG fprintf(stderr, "No database config found!\n");
#define NO_DB_CONFIG_FOR_ENV_MSG fprintf(stderr, "No database config file for current env!\n");

void koro_createConfig();

char koro_configExists();

KoroConfig __attribute__((__malloc__))*koro_readConfig();
KoroEnvironmentConfig *koro_findEnvConfig(KoroConfig *config, const char *env);

void koro_freeConfig(KoroConfig *config);

char *koro_getFlavor(void);
