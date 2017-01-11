#pragma once

#include <axon/utils.h>
#include <axon/db/exec.h>
#include <axon/db/exec_sequence.h>

#define AXON_SEED_FILES_DIRECTORY "./db/seed/"

char axon_isDatabaseSeed(char *str);

int axon_databaseSeed();
