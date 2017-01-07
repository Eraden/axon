#include "axon/info.h"
#include "axon/db.h"

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");
  if (argc <= 1) {
    axon_info();
    return 0;
  }
  const char *op = argv[1];
  if (axon_isInfo(op)) axon_info();
  if (axon_isDB(op)) axon_dbExec(argc, argv);
  return 0;
}
