#pragma once

#include <axon/db/init.h>
#include <axon/db/table.h>
#include <axon/utils.h>
#include <axon/codes.h>

char axon_isDatabaseCommand(const char *str);

int axon_execDatabaseCommand(int argc, char **argv);
