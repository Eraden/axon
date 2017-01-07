#pragma once

#include "koro/utils.h"
#include "koro/db/exec.h"

typedef struct sKoroMigration {
  int timestamp;
  char *path;
  char perform;
} KoroMigration;

typedef struct sKoroMigratorContext {
  size_t len;
  KoroMigration **migrations;
} KoroMigratorContext;

KoroMigratorContext *koro_loadMigrations(void);

void koro_freeMigrations(KoroMigratorContext *koroMigratorContext, const char writeToSave);

char koro_isMigrate(const char *arg);

int koro_migrate();
