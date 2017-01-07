#include "koro/config.h"

void koro_createConfig() {
  koro_ensureStructure();
  FILE *config = fopen("./conf/database.yml", "w+");
  if (config == NULL) {
    fprintf(stderr, "Error: %s\n", strerror(errno));
    return;
  }
  fprintf(
      config,
      ""
          "dev:\n"
          "  name: \"kore_dev\"\n"
          "  port: 5432\n"
          "  host: \"localhost\"\n"
          "prod:\n"
          "  name: \"kore_prod\"\n"
          "  port: 5432\n"
          "  host: \"localhost\"\n"
          "test:\n"
          "  name: \"kore_test\"\n"
          "  port: 5432\n"
          "  host: \"localhost\"\n"
  );
  fclose(config);
}

char koro_configExists() {
  return koro_checkIO("./conf/database.yml");
}

static void koro_fetchConfig(KoroEnvironmentConfig *c, char *name, char *value) {
  if (c == NULL) {
    if (name) free(name);
    if (value) free(value);
    return;
  }
  if (name == NULL) {
    if (value) free(value);
    return;
  }
  if (strcmp(name, "name") == 0) {
    c->name = value;
  } else if (strcmp(name, "host") == 0) {
    c->host = value;
  } else if (strcmp(name, "port") == 0) {
    c->port = value ? atoi(value) : 5432;
    free(value);
  } else {
    if (value) free(value);
  }
  free(name);
}

static void koro_readFile(KoroConfig *config) {
  FILE *f = fopen("./conf/database.yml", "rb");
  if (f == NULL) return;
  yaml_parser_t parser;
  yaml_parser_initialize(&parser);
  yaml_parser_set_input_file(&parser, f);
  yaml_token_t token;
  KoroEnvironmentConfig *c = NULL;

  char *key = NULL;
  char *value = NULL;
  enum yaml_token_type_e type = YAML_NO_TOKEN;

  do {
    yaml_parser_scan(&parser, &token);

    switch (token.type) {
      case YAML_STREAM_START_TOKEN:
      case YAML_STREAM_END_TOKEN:
        break;
      case YAML_KEY_TOKEN:
        if (token.start_mark.column != 0)
          type = token.type;
        break;
      case YAML_VALUE_TOKEN:
        type = token.type;
        break;
      case YAML_BLOCK_SEQUENCE_START_TOKEN:
      case YAML_BLOCK_ENTRY_TOKEN:
        break;
      case YAML_BLOCK_END_TOKEN:
        type = YAML_BLOCK_MAPPING_START_TOKEN;
        break;
      case YAML_BLOCK_MAPPING_START_TOKEN:
        type = YAML_BLOCK_MAPPING_START_TOKEN;
        break;
      case YAML_SCALAR_TOKEN:
        switch (type) {
          case YAML_VALUE_TOKEN: {
            value = calloc(sizeof(char), strlen((char *) token.data.scalar.value) + 1);
            strcpy(value, (char *) token.data.scalar.value);
            type = YAML_NO_TOKEN;
            koro_fetchConfig(c, key, value);
            key = NULL;
            value = NULL;
            break;
          }
          case YAML_KEY_TOKEN: {
            if (key != NULL) {
              koro_fetchConfig(c, key, value);
            }
            key = calloc(sizeof(char), strlen((char *) token.data.scalar.value) + 1);
            strcpy(key, (char *) token.data.scalar.value);
            break;
          }
          case YAML_BLOCK_MAPPING_START_TOKEN: {
            type = YAML_NO_TOKEN;
            char *env = calloc(sizeof(char), strlen((char *) token.data.scalar.value) + 1);
            strcpy(env, (char *) token.data.scalar.value);
            c = calloc(sizeof(KoroEnvironmentConfig), 1);
            config->len += 1;
            config->configs = config->configs ?
                              realloc(config->configs, sizeof(KoroEnvironmentConfig *) * (config->len + 1)) :
                              calloc(sizeof(KoroEnvironmentConfig *), config->len + 1);
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
            break;
        }
        break;
      default:
        break;
    }
    if (token.type != YAML_STREAM_END_TOKEN)
      yaml_token_delete(&token);
  } while (token.type != YAML_STREAM_END_TOKEN);

  yaml_token_delete(&token);
  yaml_parser_delete(&parser);
  fclose(f);
}

KoroConfig *koro_readConfig() {
  KoroConfig *config = calloc(sizeof(KoroConfig), 1);
  config->configs = NULL;
  config->environments = NULL;
  config->len = 0;

  if (koro_configExists() == 0) {
    fprintf(stderr, "No database config found, creating from template...\n");
    koro_createConfig();
  }
  koro_readFile(config);

  return config;
}

KoroEnvironmentConfig *koro_findEnvConfig(KoroConfig *config, const char *env) {
  if (config == NULL) return NULL;
  if (config->environments == NULL) return NULL;
  KoroEnvironmentConfig *c = NULL;
  char **keys = config->environments;
  int offset = 0;

  while (keys && *(keys + offset)) {
    if (strcmp(*(keys + offset), env) == 0)
      return *(config->configs + offset);
    offset += 1;
  }

  return c;
}

void koro_freeConfig(KoroConfig *config) {
  char **keys = config->environments;
  KoroEnvironmentConfig **configs = config->configs;
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

char *koro_getFlavor(void) {
  char *flavor = NULL;
  if (getenv("KORE_ENV")) {
    const char *env = getenv("KORE_ENV");
    flavor = calloc(sizeof(char), strlen(env) + 1);
    strcpy(flavor, env);
    return flavor;
  }
  // This is kore .flavor file
  FILE *f = fopen(".flavor", "r");
  if (f == NULL) {
    flavor = calloc(sizeof(char), strlen("dev") + 1);
    strcpy(flavor, "dev");
    return flavor;
  }
  while (!feof(f)) {
    char c = (char) fgetc(f);
    switch (c) {
      case ' ':
      case '\r':
      case '\n':
        fclose(f);
        return flavor;
      default: {
        if (isascii(c)) {
          const size_t len = flavor ? strlen(flavor) + 1 : 1;
          flavor = flavor ?
                   realloc(flavor, sizeof(char) * (len + 1)) :
                   calloc(sizeof(char), len + 1);
          flavor[len - 1] = c;
          flavor[len] = 0;
        }
        break;
      }
    }
  }
  setenv("KORE_ENV", flavor, 0);
  fclose(f);
  return flavor;
}
