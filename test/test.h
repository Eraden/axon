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
#include "axon/db/create.h"
#include "axon/db/drop.h"
#include "axon/db/migrate.h"
#include "axon/db/init.h"
#include "axon/db/setup.h"
#include "axon/db.h"
#include "axon/info.h"

#define GO_TO_DUMMY chdir(dummyRoot);
