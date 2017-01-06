#include "koro/info.h"
#include "koro/db.h"

int main(int argc, char **argv) {
  if (argc <= 1) {
    koro_info();
    return 0;
  }
  const char *op = argv[1];
  if (koro_isInfo(op)) koro_info();
  if (koro_isDB(op)) koro_dbExec(argc, argv);
  return 0;
}
