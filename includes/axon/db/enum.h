#pragma once

#include <axon/utils.h>
#include <axon/codes.h>
#include <axon/config.h>
#include <axon/db/write.h>

typedef struct sAxonEnum {
  char *name;
  char **values;
  size_t len;
} AxonEnum;

int axon_newEnum(int argc, char **argv);

int axon_dropEnum(int argc, char **argv);
