#pragma once

#include <axon/utils.h>
#include <axon/codes.h>

typedef struct sAxonCompileTriggersData {
  char *pwd;
  char *compiler;
  char **files;
  size_t len;
  AxonTriggersConfig *config;
} AxonCompileTriggersData;

int axon_buildTriggers(void);

int axon_runCompiler(int argc, char **argv);
