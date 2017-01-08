#pragma once

#include <string.h>
#include <stdio.h>

#include <axon/version.h>

char axon_isInfo(const char *str);

char axon_migrator_isInfo(const char *str);

void axon_info();

void axon_migrator_info();
