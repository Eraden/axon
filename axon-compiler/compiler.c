#include "compiler.h"

static const char *CREATE_TRIGGERS_OBJECTS = " -std=c11 -Wall -fPIC -c ";

#ifdef __clang__
static const char *CREATE_TRIGGERS_OBJECTS_MEM_CHECK = " -g -fsanitize=address -fno-omit-frame-pointer "
    "-std=c11 -Wall -fPIC -c ";
#else
static const char *CREATE_TRIGGERS_OBJECTS_MEM_CHECK = " -std=c11 -Wall -fPIC -c ";
#endif

static const char *ASSEMBLY_TRIGGERS_OBJECTS = " -ldl -shared -o ./.axon/triggers.so ";

#ifdef __clang__
static const char *ASSEMBLY_TRIGGERS_OBJECTS_MEM_CHECK = " -g -fsanitize=address -fno-omit-frame-pointer"
    " -ldl -shared -o ./.axon/triggers.so ";
#else
static const char *ASSEMBLY_TRIGGERS_OBJECTS_MEM_CHECK = " -ldl -shared -o ./.axon/triggers.so ";
#endif

static int axon_resolveCompiler(AxonCompileTriggersContext *context);

static int axon_collectSourceFiles(AxonCompileTriggersContext *context);

static void axon_freeAxonCompileTriggersData(AxonCompileTriggersContext *context);

static char *axon_appendLibsToCommand(char *command, char *libs) {
  if (libs == NULL) return command;
  const size_t len = strlen(libs);
  size_t tmpLen = 0;
  char *tmp = NULL;
  char prev = 0;
  for (size_t i = 0; i < len; i++) {
    char c = libs[i];
    if ((prev == ' ' || prev == 0) && c != ' ') {
      tmpLen = tmpLen + strlen(" -l") + 1;
      tmp = tmp ?
            realloc(tmp, sizeof(char) * (tmpLen + 1)) :
            calloc(sizeof(char), tmpLen + 1);
      strcat(tmp, " -l");
      tmp[tmpLen - 1] = c;
      tmp[tmpLen] = 0;
    } else {
      tmpLen += 1;
      tmp = tmp ?
            realloc(tmp, sizeof(char) * (tmpLen + 1)) :
            calloc(sizeof(char), tmpLen + 1);
      tmp[tmpLen - 1] = c;
      tmp[tmpLen] = 0;
    }
    prev = c;
  }
  const size_t oldLen = strlen(command);
  const size_t newLen = oldLen + strlen(" ") + strlen(tmp);
  command = realloc(command, sizeof(char) * (newLen + 1));
  char *ptr = command + oldLen;
  for (size_t i = 0; i < newLen - oldLen; i++) ptr[i] = 0;
  strcat(command, " ");
  strcat(command, tmp);
  command[newLen] = 0;
  free(tmp);
  return command;
}

static char *axon_getObjectFilePath(const char *sourceFilePath) {
  char *pwd = realpath("./", NULL);
  char *ptr = realloc(pwd, sizeof(char) * (strlen(pwd) + strlen("/.axon") + 1));
  if (ptr == NULL) {
    /* LCOV_EXCL_START */
    free(pwd);
    return NULL;
    /* LCOV_EXCL_STOP */
  } else {
    pwd = ptr;
    strcat(pwd, "/.axon");
  }
  char *basename = strrchr(sourceFilePath, '/');
  size_t pathLen = strlen(pwd) + strlen(basename);
  char *objectPath = calloc(sizeof(char), pathLen + 1);
  strcat(objectPath, pwd);
  strcat(objectPath, basename);
  objectPath[pathLen - 1] = 'o';
  free(pwd);
  return objectPath;
}

static int axon_resolveCompiler(AxonCompileTriggersContext *context) {
  char *compiler = getenv("CC");
  if (context->option == AXON_COMPILER_MEM_CHECK_OPTION) {
    compiler = "clang";
  }
  if (compiler == NULL) compiler = "cc";
  char *str = calloc(sizeof(char), strlen(compiler) + 1);
  strcpy(str, compiler);
  context->compiler = str;

  return AXON_SUCCESS;
}

static int axon_collectSourceFiles(AxonCompileTriggersContext *context) {
  char *path = realpath("./db/migrate", NULL);
  DIR *d = opendir(path);
  size_t pathLen = strlen(path);

  if (d == NULL) return AXON_FAILURE; /* LCOV_EXCL_LINE */

  struct dirent *p = readdir(d);

  while (p) {
    char *buf = 0;
    size_t len = 0;
    if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
      p = readdir(d);
      continue;
    }

    len = pathLen + strlen(p->d_name) + 2;
    buf = malloc(len);

    if (buf == NULL) {
      /* LCOV_EXCL_START */
      fprintf(stderr, "%s\n", strerror(errno));
      return AXON_FAILURE;
      /* LCOV_EXCL_STOP */
    }

    struct stat statBuf;
    snprintf(buf, len, "%s/%s", path, p->d_name);

    char isFile = !stat(buf, &statBuf) && S_ISREG(statBuf.st_mode);
    if (isFile && strstr(p->d_name, "_callback.c") != NULL) {
      context->len += 1;
      context->files = context->files ?
                       realloc(context->files, sizeof(char *) * (context->len + 1)) :
                       calloc(sizeof(char *), context->len + 1);
      context->files[context->len - 1] = buf;
      context->files[context->len] = 0;
      p = readdir(d);
    } else {
      /* LCOV_EXCL_START */
      free(buf);
      p = readdir(d);
      /* LCOV_EXCL_STOP */
    }
  }

  free(path);
  closedir(d);
  return AXON_SUCCESS;
}

static void axon_freeAxonCompileTriggersData(AxonCompileTriggersContext *context) {
  char **files = context->files;
  while (files && *files) {
    free(*files);
    files += 1;
  }
  if (context->compiler) free(context->compiler);
  if (context->files) free(context->files);
  if (context->pwd) free(context->pwd);
  free(context);
}

// gcc -fPIC -c ./db/migrate/source_1.c -o ./.axon/source_1.o
static int axon_createObjectFiles(AxonCompileTriggersContext *context) {
  int result = AXON_SUCCESS;

  char **files = context->files;
  while (files && *files) {
    char *file = *files;
    char *object = axon_getObjectFilePath(file);
    size_t len = strlen(context->compiler);
    len += 1;
    if (context->option == AXON_COMPILER_MEM_CHECK_OPTION) {
      len += strlen(CREATE_TRIGGERS_OBJECTS_MEM_CHECK);
    } else {
      len += strlen(CREATE_TRIGGERS_OBJECTS);
    }
    len += strlen(file);
    len += strlen(" -o ");
    len += strlen(object) + 1;

    char *command = calloc(sizeof(char), len + 1);
    strcpy(command, context->compiler);
    strcat(command, " ");
    if (context->option == AXON_COMPILER_MEM_CHECK_OPTION) {
      strcat(command, CREATE_TRIGGERS_OBJECTS_MEM_CHECK);
    } else {
      strcat(command, CREATE_TRIGGERS_OBJECTS);
    }
    strcat(command, " ");
    strcat(command, file);
    strcat(command, " -o ");
    strcat(command, object);
    files += 1;

    result = axon_runCommand(command);
    free(object);
    free(command);
    if (result != AXON_SUCCESS) break;
  }

  return result;
}

// gcc -shared -o libtest.so source_1.o source_2.o
static int axon_assemblyObjectFiles(AxonCompileTriggersContext *context) {
  size_t len = strlen(context->compiler);
  if (context->option == AXON_COMPILER_MEM_CHECK_OPTION) {
    len = len + strlen(ASSEMBLY_TRIGGERS_OBJECTS_MEM_CHECK);
  } else {
    len = len + strlen(ASSEMBLY_TRIGGERS_OBJECTS);
  }
  if (context->config->flags) len = len + strlen(" ") + strlen(context->config->flags);
  char *command = calloc(sizeof(char), len + 1);
  strcpy(command, context->compiler);
  if (context->option == AXON_COMPILER_MEM_CHECK_OPTION) {
    strcat(command, ASSEMBLY_TRIGGERS_OBJECTS_MEM_CHECK);
  } else {
    strcat(command, ASSEMBLY_TRIGGERS_OBJECTS);
  }
  if (context->config->flags) {
    strcat(command, " ");
    strcat(command, context->config->flags);
  }
  command = axon_appendLibsToCommand(command, context->config->libs);
  len = strlen(command);
  int result = AXON_SUCCESS;

  char **read = context->files;

  while (read && *read) {
    char *objectPath = axon_getObjectFilePath(*read);

    len = len + strlen(objectPath) + 1;
    char *ptr = realloc(command, sizeof(char) * (len + 1));
    if (ptr == NULL) {
      /* LCOV_EXCL_START */
      free(objectPath);
      result = AXON_FAILURE;
      break;
      /* LCOV_EXCL_STOP */
    } else {
      command = ptr;
    }
    strcat(command, " ");
    strcat(command, objectPath);
    command[len] = 0;
    read += 1;
    free(objectPath);
  }

  if (result == AXON_SUCCESS) result = axon_runCommand(command);
  free(command);

  return result;
}

static int axon_callCompiler(AxonCompileTriggersContext *context) {
  int result;
  chdir(context->pwd);
  result = axon_createObjectFiles(context);
  chdir(context->pwd);
  if (result != AXON_SUCCESS) return result;
  chdir(context->pwd);
  result = axon_assemblyObjectFiles(context);
  chdir(context->pwd);
  return result;
}

static void axon_compilerCheckOptions(int argc, char **argv, AxonCompileTriggersContext *context) {
  for (unsigned i = 2; i < argc; i++) {
    if (strcmp(argv[i], "--mem-check") == 0) {
      context->option = AXON_COMPILER_MEM_CHECK_OPTION;
    }
  }
}

int axon_runCompiler(int argc, char **argv) {
  if (argc < 2) return AXON_FAILURE;

  if (strcmp(argv[1], "triggers") == 0) {
    return axon_buildTriggers(argc, argv);
  } else {
    return AXON_UNKNOWN_COMMAND;
  }
}

int axon_buildTriggers(int argc, char **argv) {
  int result = axon_createConfig();
  if (result != AXON_SUCCESS) return result;

  AxonCompileTriggersContext *context = calloc(sizeof(AxonCompileTriggersContext), 1);
  context->option = AXON_COMPILER_NO_OPTION;
  context->pwd = calloc(sizeof(char), 1024);
  getcwd(context->pwd, 1024);
  context->config = axon_readTriggersConfig();
  axon_compilerCheckOptions(argc, argv, context);
  result = axon_resolveCompiler(context);
  if (result == AXON_SUCCESS) result = axon_collectSourceFiles(context);
  if (result == AXON_SUCCESS) result = axon_callCompiler(context);
  axon_freeTriggersConfig(context->config);
  axon_freeAxonCompileTriggersData(context);
  return result;
}
