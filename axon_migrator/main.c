#include "axon/db/migrate.h"
#include "axon/db/create.h"
#include "axon/db/drop.h"

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");

  if (argc < 2) return KORO_FAILURE;
  if (axon_isMigrate(argv[1])) {
    return axon_migrate();
  } else if (axon_isDatabaseCreate(argv[1])) {
    return axon_createDatabase();
  } else if (axon_isDatabaseDrop(argv[1])) {
    return axon_dropDatabase();
  } else {
    return KORO_FAILURE;
  }
}
