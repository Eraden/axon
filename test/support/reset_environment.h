#pragma once

#include "../test.h"
#include "./ck_io.h"

void _prepare_clear_state(void);

#define WITHIN(path, code) do { \
    char here[1024]; \
    memset(here, 0, 1024); \
    getcwd(here, 1024); \
    chdir(path); \
    code \
    chdir(here); \
  } while (0);

#define IN_CLEAR_STATE(code) \
  _prepare_clear_state(); \
  ck_redirectStderr(ck_redirectStdout(code))