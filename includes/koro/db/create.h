#pragma once

#include "koro/utils.h"
#include "koro/db/exec.h"

char koro_isDatabaseCreate(char *str);

int koro_createDatabase();

char koro_isDatabaseDrop(char *str);

int koro_dropDatabase();
