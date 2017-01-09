#include <axon/config.h>

static void axon_fetchDatabaseConfig(AxonEnvironmentConfig *c, char *name, char *value) {
  if (c == NULL) {
    /* LCOV_EXCL_START */
    if (name) free(name);
    if (value) free(value);
    return;
    /* LCOV_EXCL_STOP */
  }
  if (name == NULL) {
    /* LCOV_EXCL_START */
    if (value) free(value);
    return;
    /* LCOV_EXCL_STOP */
  }
  if (strcmp(name, "name") == 0) {
    c->name = value;
  } else if (strcmp(name, "host") == 0) {
    c->host = value;
  } else if (strcmp(name, "port") == 0) {
    c->port = value ? atoi(value) : 5432;
    free(value);
  } else {
    if (value) free(value); /* LCOV_EXCL_LINE */
  }
  free(name);
}

static void axon_readDatabaseConfigFile(AxonConfig *config) {
  FILE *f = fopen(AXON_DATABASE_CONFIG_FILE, "rb");
  if (f == NULL) return;
  yaml_parser_t parser;
  yaml_parser_initialize(&parser);
  yaml_parser_set_input_file(&parser, f);
  yaml_token_t token;
  AxonEnvironmentConfig *c = NULL;

  char *key = NULL;
  char *value = NULL;
  enum yaml_token_type_e type = YAML_NO_TOKEN;
  unsigned breakCounter = 0; /* LCOV_EXCL_LINE */

  do {
    if (breakCounter++ > 200) break; /* LCOV_EXCL_LINE */
    yaml_parser_scan(&parser, &token);

    switch (token.type) {
      case YAML_KEY_TOKEN:
        if (token.start_mark.column != 0)
          type = token.type;
        break;
      case YAML_VALUE_TOKEN:
        type = token.type;
        break;
      case YAML_BLOCK_MAPPING_START_TOKEN:
      case YAML_BLOCK_END_TOKEN:
        type = YAML_BLOCK_MAPPING_START_TOKEN;
        break;
      case YAML_SCALAR_TOKEN:
        switch (type) {
          case YAML_VALUE_TOKEN: {
            value = calloc(sizeof(char), strlen((char *) token.data.scalar.value) + 1);
            strcpy(value, (char *) token.data.scalar.value);
            type = YAML_NO_TOKEN;
            axon_fetchDatabaseConfig(c, key, value);
            key = NULL;
            value = NULL;
            break;
          }
          case YAML_KEY_TOKEN: {
            if (key != NULL) axon_fetchDatabaseConfig(c, key, value); /* LCOV_EXCL_LINE */
            key = calloc(sizeof(char), strlen((char *) token.data.scalar.value) + 1);
            strcpy(key, (char *) token.data.scalar.value);
            break;
          }
          case YAML_BLOCK_MAPPING_START_TOKEN: {
            type = YAML_NO_TOKEN;
            char *env = calloc(sizeof(char), strlen((char *) token.data.scalar.value) + 1);
            strcpy(env, (char *) token.data.scalar.value);
            c = calloc(sizeof(AxonEnvironmentConfig), 1);
            config->len += 1;
            config->configs = config->configs ?
                              realloc(config->configs, sizeof(AxonEnvironmentConfig *) * (config->len + 1)) :
                              calloc(sizeof(AxonEnvironmentConfig *), config->len + 1);
            config->configs[config->len - 1] = c;
            config->configs[config->len] = 0;
            config->environments = config->environments ?
                                   realloc(config->environments, sizeof(char *) * (config->len + 1)) :
                                   calloc(sizeof(char *), config->len + 1);
            config->environments[config->len - 1] = env;
            config->environments[config->len] = 0;
            break;
          }
          default:
            break; /* LCOV_EXCL_LINE */
        }
        break;
      case YAML_STREAM_START_TOKEN:
      case YAML_STREAM_END_TOKEN:
      case YAML_BLOCK_SEQUENCE_START_TOKEN:
      case YAML_BLOCK_ENTRY_TOKEN:
      default:
        break; /* LCOV_EXCL_LINE */
    }
    if (token.type == YAML_NO_TOKEN) {
      if (key) free(key);
      yaml_token_delete(&token);
      break;
    }
    if (token.type != YAML_STREAM_END_TOKEN)
      yaml_token_delete(&token);
  } while (token.type != YAML_STREAM_END_TOKEN);

  yaml_token_delete(&token);
  yaml_parser_delete(&parser);
  fclose(f);
}

AxonConfig *axon_readDatabaseConfig() {
  AxonConfig *config = calloc(sizeof(AxonConfig), 1);
  config->configs = NULL;
  config->environments = NULL;
  config->len = 0;

  if (axon_configExists() == 0) axon_createConfig();
  axon_readDatabaseConfigFile(config);

  return config;
}

AxonEnvironmentConfig *axon_findEnvDatabaseConfig(AxonConfig *config, const char *env) {
  if (config == NULL) return NULL;
  if (config->environments == NULL) return NULL;
  AxonEnvironmentConfig *c = NULL;
  char **keys = config->environments;
  int offset = 0;

  while (keys && *(keys + offset)) {
    if (strcmp(*(keys + offset), env) == 0)
      return *(config->configs + offset);
    offset += 1;
  }

  return c;
}

void axon_freeDatabaseConfig(AxonConfig *config) {
  char **keys = config->environments;
  AxonEnvironmentConfig **configs = config->configs;
  while (keys && *keys) {
    free(*keys);
    if ((*configs)->name) free((*configs)->name);
    if ((*configs)->connectionName) free((*configs)->connectionName);
    if ((*configs)->host) free((*configs)->host);
    free(*configs);
    keys += 1;
    configs += 1;
  }
  if (config->configs) free(config->configs);
  if (config->environments) free(config->environments);
  free(config);
}
