#pragma once

#include <axon/utils.h>
#include <axon/codes.h>

typedef void (*freeFunction)(void *payload);

typedef struct sAxonCallbackData {
  freeFunction freePayload;
  void *payload;
  char *sql;
  size_t len;
  int flags;
} AxonCallbackData;

