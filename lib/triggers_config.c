#include <axon/config.h>

static void axon_fetchTriggerConfig(AxonTriggersConfig *config, AxonTriggersTarget target, char **buffer, size_t *len) {
  if (buffer == NULL) return;
  if (target == AXON_TRIGGERS_TARGET_LIBS)
    config->libs = *buffer;
  else if (target == AXON_TRIGGERS_TARGET_FLAGS)
    config->flags = *buffer;
  else
    free(*buffer);
  *buffer = NULL;
  *len = 0;
}

static int axon_readTriggersConfigFile(AxonTriggersConfig *config) {
  int result = AXON_SUCCESS;
  FILE *f = fopen(AXON_TRIGGERS_FILE, "r");
  if (f == NULL) return AXON_CONFIG_MISSING;
  char *buffer = NULL;
  size_t len = 0;
  AxonTriggersTarget target = AXON_TRIGGERS_TARGET_NONE;

  while (!feof(f)) {
    char c = (char) fgetc(f);

    switch (c) {
      case '\n': {
        axon_fetchTriggerConfig(config, target, &buffer, &len);
        target = AXON_TRIGGERS_TARGET_NONE;
        break;
      }
      case ' ': {
        if (target != AXON_TRIGGERS_TARGET_NONE)
          goto axon_parse_triggers_config_append;
        break;
      }
      case ':': {
        if (buffer == NULL) {
          target = AXON_TRIGGERS_TARGET_NONE;
          break;
        }
        if (strcmp(buffer, "libs") == 0) {
          target = AXON_TRIGGERS_TARGET_LIBS;
        } else if (strcmp(buffer, "flags") == 0) {
          target = AXON_TRIGGERS_TARGET_FLAGS;
        } else {
          target = AXON_TRIGGERS_TARGET_NONE;
        }
        len = 0;
        free(buffer);
        buffer = NULL;
        break;
      }
      default: {
        axon_parse_triggers_config_append:
        len += 1;
        buffer = buffer ?
                 realloc(buffer, sizeof(char) * (len + 1)) :
                 calloc(sizeof(char), len + 1);
        buffer[len - 1] = c;
        buffer[len] = 0;
        break;
      }
    }
  }

  axon_fetchTriggerConfig(config, target, &buffer, &len);

  fclose(f);
  return result;
}

AxonTriggersConfig *axon_readTriggersConfig(void) {
  int result = axon_createConfig();
  if (result != AXON_SUCCESS) return NULL;

  AxonTriggersConfig *config = calloc(sizeof(AxonTriggersConfig), 1);
  config->flags = NULL;
  config->libs = NULL;
  axon_readTriggersConfigFile(config);
  return config;
}

void axon_freeTriggersConfig(AxonTriggersConfig *config) {
  if (config->libs != NULL) free(config->libs);
  if (config->flags != NULL) free(config->flags);
  free(config);
}
