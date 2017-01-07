#pragma once

#include <check.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <unitypes.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

#include "./support/variables.h"
#include "./support/ck_io.h"
#include "./support/prepare.h"
#include "koro/db/create.h"
#include "koro/db/migrate.h"
#include "koro/db/init.h"
#include "koro/db.h"
#include "koro/info.h"

#define GO_TO_DUMMY chdir(dummyRoot);
