#include "koro/info.h"

char koro_isInfo(const char *str) {
  return strcmp(str, "info") == 0 ||
         strcmp(str, "--info") == 0 ||
         strcmp(str, "-i") == 0 ||
         strcmp(str, "help") == 0 ||
         strcmp(str, "--help") == 0 ||
         strcmp(str, "-h") == 0;
}

void koro_info() {
  printf(
      "koro %i.%i.%i\n"
          "  db - execute database operations\n"
          "  assets - execute assets operations\n"
          "init:\n"
          "db:\n"
          "  kore db init\n"
          "  kore db new table TABLE_NAME COLUMN COLUMN:TYPE\n"
          "  kore db change TABLE_NAME add COLUMN:TYPE\n"
          "  kore db change TABLE_NAME drop COLUMN\n",
      KORO_MAJOR_VERSION, KORO_MINOR_VERSION, KORO_PATCH_VERSION
  );
}
