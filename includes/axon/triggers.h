#pragma once

#include <axon/utils.h>
#include <axon/codes.h>

typedef struct sAxonCallbackData {
  void *payload;
  char *sql;
  size_t len;
  int flags;
};

int axon_buildTriggers(void);
