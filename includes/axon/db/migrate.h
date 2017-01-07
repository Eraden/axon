#pragma once

#include "axon/utils.h"
#include "axon/db/exec.h"

typedef struct sAxonMigration {
  int timestamp;
  char *path;
  char perform;
} AxonMigration;

typedef struct sAxonMigratorContext {
  size_t len;
  AxonMigration **migrations;
} AxonMigratorContext;

AxonMigratorContext *axon_loadMigrations(void);

void axon_freeMigrations(AxonMigratorContext *axonMigratorContext, const char writeToSave);

char axon_isMigrate(const char *arg);

int axon_migrate();
