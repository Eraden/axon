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

#define KORO_COLOR_NRM  "\x1B[0m"
#define KORO_COLOR_RED  "\x1B[31m"
#define KORO_COLOR_GRN  "\x1B[32m"
#define KORO_COLOR_YEL  "\x1B[33m"
#define KORO_COLOR_BLU  "\x1B[34m"
#define KORO_COLOR_MAG  "\x1B[35m"
#define KORO_COLOR_CYN  "\x1B[36m"
#define KORO_COLOR_WHT  "\x1B[37m"

#define __WEXITSTATUS(status) (((status) & 0xff00) >> 8)

typedef struct sKoroGraph {
  char *root;
  struct sKoroGraph *leafs;
  size_t len;
} KoroGraph;

void koro_createInfo(KoroGraph *koroGraph);

void koro_drawGraph(KoroGraph *koroGraph, size_t indent);

char koro_checkIO(const char *path);

void koro_ensureStructure(void);

char koro_mkdir(const char *path);

int koro_runCommand(const char *cmd);
