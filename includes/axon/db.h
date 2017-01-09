#pragma once

#include "utils.h"
#include "axon/db/init.h"
#include "axon/db/table.h"
#include "axon/utils.h"
#include "axon/codes.h"

char axon_isDB(const char *str);

char axon_dbExec(int argc, char **argv);
