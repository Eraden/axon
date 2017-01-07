#include "axon/db/setup.h"

char axon_isSetup(char *str) {
  return strcmp(str, "setup") == 0;
}

int axon_setup() {
  return AXON_FAILURE;
}
