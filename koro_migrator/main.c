#include "koro/db/migrate.h"
#include "koro/db/create.h"

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");

  if (argc < 2) return KORO_FAILURE;
  if (koro_isMigrate(argv[1])) {
    return koro_migrate();
  } else if (koro_isDatabaseCreate(argv[1])) {
    return koro_createDatabase();
  } else if (koro_isDatabaseDrop(argv[1])) {
    return koro_dropDatabase();
  } else {
    return KORO_FAILURE;
  }
}
