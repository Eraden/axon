#pragma once

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

#define __WEXITSTATUS(status) (((status) & 0xff00) >> 8)

char koro_checkIO(const char *path);

void koro_ensureStructure(void);

char koro_mkdir(const char *path);

int koro_runCommand(const char *cmd);
