#include <axon/info.h>

char axon_isInfo(const char *str) {
  if (strcmp(str, "info") == 0) return 1;
  if (strcmp(str, "--info") == 0) return 1;
  if (strcmp(str, "-i") == 0) return 1;
  if (strcmp(str, "help") == 0) return 1;
  if (strcmp(str, "--help") == 0) return 1;
  if (strcmp(str, "-h") == 0) return 1;
  return 0;
}

void axon_info() {
  printf(
      AXON_CLI_INFO,
      AXON_MAJOR_VERSION, AXON_MINOR_VERSION, AXON_PATCH_VERSION
  );
}
