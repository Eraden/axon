#include "./build_dummy_triggers.h"

void ck_axon_dummy_triggers(char *beforePath, char *beforeContent, char *afterPath, char *afterContent) {
  ck_unlink(beforePath);
  ck_unlink(afterPath);

  if (beforeContent) ck_overrideFile(beforePath, beforeContent);
  if (afterContent) ck_overrideFile(afterPath, afterContent);

  ck_unlink("./log");
  axon_mkdir("./log");
  ck_unlink("./.axon");
  ck_writeTestTriggersConfig();
  char *args[3] = {"inline", "triggers", "--mem-check"};
  axon_buildTriggers(3, args);
}
