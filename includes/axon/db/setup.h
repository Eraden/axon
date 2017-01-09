#pragma once

#include "axon/utils.h"
#include "axon/db/exec.h"
#include "axon/db/exec_sequence.h"

#define AXON_SETUP_FILES_DIRECTORY "./db/setup/"

char axon_isSetup(char *str);

int axon_setup();
