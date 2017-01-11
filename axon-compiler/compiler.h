#pragma once

#include <axon/utils.h>
#include <axon/codes.h>

#define AXON_COMPILER_NO_OPTION 0
#define AXON_COMPILER_MEM_CHECK_OPTION 1

typedef struct sAxonCompileTriggersContext {
  char *pwd;
  char *compiler;
  char **files;
  size_t len;
  AxonTriggersConfig *config;
  unsigned int option;
} AxonCompileTriggersContext;

int axon_buildTriggers(int argc, char **argv);

int axon_runCompiler(int argc, char **argv);
