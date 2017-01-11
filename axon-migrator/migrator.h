#pragma once

#include <axon/info.h>
#include <axon/db/migrate.h>
#include <axon/db/create.h>
#include <axon/db/drop.h>
#include <axon/db/setup.h>
#include <axon/db/seed.h>

int axon_runMigrator(int argc, char **argv);
