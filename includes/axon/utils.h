#pragma once

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unitypes.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>

#include "axon/config.h"

#define KORO_COLOR_NRM  "\x1B[0m"
#define KORO_COLOR_RED  "\x1B[31m"
#define KORO_COLOR_GRN  "\x1B[32m"
#define KORO_COLOR_YEL  "\x1B[33m"
#define KORO_COLOR_BLU  "\x1B[34m"
#define KORO_COLOR_MAG  "\x1B[35m"
#define KORO_COLOR_CYN  "\x1B[36m"
#define KORO_COLOR_WHT  "\x1B[37m"

#define __WEXITSTATUS(status) (((status) & 0xff00) >> 8)

typedef struct sAxonGraph {
  char *root;
  struct sAxonGraph *leafs;
  size_t len;
} AxonGraph;

void axon_createInfo(AxonGraph *axonGraph);

void axon_drawGraph(AxonGraph *axonGraph, size_t indent);

char axon_checkIO(const char *path);

void axon_ensureStructure(void);

char axon_touch(const char *path);

char axon_mkdir(const char *path);

int axon_runCommand(const char *cmd);

char *axon_getDatabaseName(void);
