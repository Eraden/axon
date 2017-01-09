#include <axon/info.h>

char axon_migrator_isInfo(const char *str) {
  if (strcmp(str, "info") == 0) return 1;
  if (strcmp(str, "--info") == 0) return 1;
  if (strcmp(str, "-i") == 0) return 1;
  if (strcmp(str, "help") == 0) return 1;
  if (strcmp(str, "--help") == 0) return 1;
  if (strcmp(str, "-h") == 0) return 1;
  return 0;
}

/* LCOV_EXCL_START */
void axon_migrator_info() {
  printf(
      "axon-migrator %i.%i.%i\n"
          "  axon-migrator create\n"
          "  axon-migrator drop\n"
          "  axon-migrator migrate\n"
          "  axon-migrator setup\n",
      AXON_MAJOR_VERSION, AXON_MINOR_VERSION, AXON_PATCH_VERSION
  );
}
/* LCOV_EXCL_STOP */
