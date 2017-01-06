#pragma once

#include "utils.h"
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

void koro_createConfig();

char koro_configExists();

KoroConfig *koro_readConfig();
KoroEnvironmentConfig *koro_findEnvConfig(KoroConfig *config, const char *env);

void koro_freeConfig(KoroConfig *config);

char *koro_getFlavor(void);
