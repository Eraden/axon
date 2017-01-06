#pragma once

#include "utils.h"
#include "koro/db/init.h"
#include "koro/db/table.h"
#include "koro/utils.h"
#include "koro/codes.h"

char koro_isDB(const char *str);

char koro_dbExec(int argc, char **argv);
