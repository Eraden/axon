#pragma once

#include "../test.h"

#define CK_FS_OK 0
#define CK_FS_MISSING 1
#define CK_FS_PERM_DENIED 2

#define NOW() ((unsigned long long) time(NULL))

void ck_dropTestDb();

void ck_createTestDb();

void ck_catchStderr(const char *newStream);

void ck_releaseStderr();

void ck_catchStdout(const char *newStream);

void ck_releaseStdout();

int _ck_io_check(const char *path);

int _ck_unlink(const char *path);

int _ck_io_contains(const char *path, const char *content);

char *_ck_findFile(const char *dir, const char *pattern);

void _ck_make_dummy_sql(const char *name, const char *sql, long long int timestamp);

#define ck_find_file_in(dir, pattern) _ck_findFile(dir, pattern)

#define ck_make_dummy_sql(name, sql, timestamp) _ck_make_dummy_sql(name, sql, timestamp);

#define ck_path_contains(path, content) do { \
     ck_assert_msg(_ck_io_contains(path, content) == 1, ""#path" contains "#content" failed"); \
} while (0);


#define ck_path_exists(path) do { \
     ck_assert_msg(_ck_io_check(path) == CK_FS_OK, "io path "#path" exists failed"); \
} while (0);

#define ck_assert_file_in(dir, name) do { \
    char *path = _ck_findFile(dir, name); \
    char exists = 0; \
    if (path != NULL) { exists = 1; free(path); } \
     ck_assert_msg(exists, "file "#name" in "#dir" exists failed"); \
  } while (0);

#define ck_redirectStdout(code) \
  ck_catchStdout("./log/info.log"); \
  code; \
  ck_releaseStdout();

#define ck_redirectStderr(code) \
  ck_catchStderr("./log/error.log"); \
  code; \
  ck_releaseStderr();

#define ck_unlink(path) _ck_unlink(path)