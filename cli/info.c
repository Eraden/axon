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
      "axon %i.%i.%i\n"
          "  db - execute database operations\n"
          "db:\n"
          "  axon db init\n"
          "  axon db new table TABLE_NAME COLUMN COLUMN:TYPE\n"
          "  axon db change TABLE_NAME add COLUMN:TYPE\n"
          "  axon db change TABLE_NAME drop COLUMN\n"
          "  axon db create\n"
          "  axon db drop\n"
          "  axon db migrate\n"
          "  axon db setup\n"
          "",
      AXON_MAJOR_VERSION, AXON_MINOR_VERSION, AXON_PATCH_VERSION
  );
}
