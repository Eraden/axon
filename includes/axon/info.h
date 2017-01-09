#pragma once

#include <string.h>
#include <stdio.h>

#include <axon/version.h>

#define AXON_CLI_INFO "axon %i.%i.%i\n" \
  "  db - execute database operations\n" \
  "db:\n" \
  "  axon db init\n" \
  "  axon db new table TABLE_NAME COLUMN COLUMN:TYPE\n" \
  "  axon db drop table TABLE_NAME\n" \
  "  axon db rename table OLD_NAME NEW_NAME\n" \
  "  axon db new enum TYPE_NAME VALUE1 [VALUE2]\n" \
  "  axon db drop enum TYPE_NAME\n" \
  "  axon db change TABLE_NAME add COLUMN:TYPE\n" \
  "  axon db change TABLE_NAME drop COLUMN\n" \
  "  axon db change TABLE_NAME retype COLUMN:NEW_TYPE\n" \
  "  axon db create\n" \
  "  axon db drop\n" \
  "  axon db migrate\n" \
  "  axon db setup\n"

#define AXON_CREATOR_INFO "axon-creator %i.%i.%i\n" \
  "  axon-creator init\n" \
  "  axon-creator new table TABLE_NAME COLUMN COLUMN:TYPE\n" \
  "  axon-creator drop table\n" \
  "  axon-creator rename table OLD_NAME NEW_NAME\n" \
  "  axon-creator new enum TYPE_NAME VALUE1 [VALUE2]\n" \
  "  axon-creator drop enum TYPE_NAME\n" \
  "  axon-creator change TABLE_NAME add COLUMN:TYPE\n" \
  "  axon-creator change TABLE_NAME drop COLUMN\n" \
  "  axon-creator change TABLE_NAME retype COLUMN:NEW_TYPE\n"

#define AXON_MIGRATOR_INFO "axon-migrator %i.%i.%i\n" \
  "  axon-migrator create\n" \
  "  axon-migrator drop\n" \
  "  axon-migrator migrate\n" \
  "  axon-migrator setup\n"

char axon_isInfo(const char *str);

char axon_migrator_isInfo(const char *str);

char axon_creator_isInfo(const char *str);

void axon_info();

void axon_migrator_info();

void axon_creator_info();
