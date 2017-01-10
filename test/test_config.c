#include "test_config.h"
#include "support/reset_environment.h"

START_TEST(test_readConfig)
  GO_TO_DUMMY
  ck_unlink(AXON_DATABASE_CONFIG_FILE);

  AxonConfig *axonConfig = axon_readDatabaseConfig();
  ck_assert_int_eq(axonConfig->len, 3);

  AxonEnvironmentConfig **configs = axonConfig->configs;
  char **keys = axonConfig->environments;

  ck_assert_ptr_ne(keys, NULL);
  ck_assert_ptr_ne(configs, NULL);

  AxonEnvironmentConfig *c = NULL;
  const char *key = NULL;

  c = *configs;
  key = *keys;
  ck_assert_ptr_ne(key, NULL);
  ck_assert_str_eq(key, "dev");
  ck_assert_ptr_ne(c, NULL);

  ck_assert_ptr_ne(c->host, NULL);
  ck_assert_str_eq(c->host, "localhost");
  ck_assert_ptr_ne(c->name, NULL);
  ck_assert_str_eq(c->name, "kore_dev");
  ck_assert_int_eq(c->port, 5432);

  keys += 1;
  configs += 1;

  c = *configs;
  key = *keys;
  ck_assert_ptr_ne(key, NULL);
  ck_assert_str_eq(key, "prod");
  ck_assert_ptr_ne(c, NULL);

  ck_assert_ptr_ne(c->host, NULL);
  ck_assert_str_eq(c->host, "localhost");
  ck_assert_ptr_ne(c->name, NULL);
  ck_assert_str_eq(c->name, "kore_prod");
  ck_assert_int_eq(c->port, 5432);

  keys += 1;
  configs += 1;

  c = *configs;
  key = *keys;
  ck_assert_ptr_ne(key, NULL);
  ck_assert_str_eq(key, "test");
  ck_assert_ptr_ne(c, NULL);

  ck_assert_ptr_ne(c->host, NULL);
  ck_assert_str_eq(c->host, "localhost");
  ck_assert_ptr_ne(c->name, NULL);
  ck_assert_str_eq(c->name, "kore_test");
  ck_assert_int_eq(c->port, 5432);

  axon_freeDatabaseConfig(axonConfig);
END_TEST

START_TEST(test_malformedDatabaseFile)
  GO_TO_DUMMY

  ck_overrideFile(AXON_DATABASE_CONFIG_FILE, "test:\n  - asd\n  -dsa");
  AxonConfig *axonConfig = axon_readDatabaseConfig();
  ck_assert_ptr_ne(axonConfig, NULL);
  axon_freeDatabaseConfig(axonConfig);
END_TEST

START_TEST(test_flavorFromFile)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */);
  ck_overrideFile("./.flavor", "test");
  unsetenv("KORE_ENV");
  char *flavor = axon_getFlavor();
  ck_assert_str_eq(flavor, "test");
  free(flavor);
  putenv("KORE_ENV=test");
END_TEST

START_TEST(test_flavorFromMultilineFile)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */);
  ck_overrideFile("./.flavor", "test\ndev");
  unsetenv("KORE_ENV");
  char *flavor = axon_getFlavor();
  ck_assert_str_eq(flavor, "test");
  free(flavor);
  putenv("KORE_ENV=test");
END_TEST

START_TEST(test_flavorNoSource)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */);
  ck_unlink("./.flavor");
  unsetenv("KORE_ENV");
  char *flavor = axon_getFlavor();
  ck_assert_str_eq(flavor, "dev");
  free(flavor);
  putenv("KORE_ENV=test");
END_TEST

START_TEST(test_readOrderWithoutOrderFile)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  ck_unlink(AXON_ORDER_CONFIG_FILE);
  AxonOrder *order = NULL;
  order = axon_readOrderConfig();
  ck_assert_ptr_ne(order, NULL);
  axon_freeOrderConfig(order);
END_TEST

START_TEST(test_readOrderWithEmptyOrderFile)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  ck_unlink(AXON_ORDER_CONFIG_FILE);
  axon_touch(AXON_ORDER_CONFIG_FILE);
  AxonOrder *order = NULL;
  ck_redirectStderr(order = axon_readOrderConfig();)
  ck_assert_ptr_ne(order, NULL);
  ck_assert_int_eq(order->seedLen, 0);
  ck_assert_int_eq(order->setupLen, 0);
  ck_assert_ptr_eq(order->seedFiles, NULL);
  ck_assert_ptr_eq(order->setupFiles, NULL);
  axon_freeOrderConfig(order);
END_TEST

START_TEST(test_axonOrder)
  GO_TO_DUMMY
  AxonOrder *axonOrder = NULL;

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "setup:\nseed:\n");
  axonOrder = axon_readOrderConfig();
  ck_assert_ptr_ne(axonOrder, NULL);
  ck_assert_int_eq(axonOrder->setupLen, 0);
  ck_assert_int_le(axonOrder->seedLen, 0);
  ck_assert_ptr_eq(axonOrder->setupFiles, NULL);
  ck_assert_ptr_eq(axonOrder->seedFiles, NULL);
  axon_freeOrderConfig(axonOrder);

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "setup:\n  - one\n  - two\nseed:\n");
  axonOrder = axon_readOrderConfig();
  ck_assert_ptr_ne(axonOrder, NULL);
  ck_assert_int_eq(axonOrder->setupLen, 2);
  ck_assert_int_le(axonOrder->seedLen, 0);
  ck_assert_ptr_ne(axonOrder->setupFiles, NULL);
  ck_assert_ptr_eq(axonOrder->seedFiles, NULL);
  ck_assert_ptr_ne(axonOrder->setupFiles[0], NULL);
  ck_assert_str_eq(axonOrder->setupFiles[0], "one");
  ck_assert_ptr_ne(axonOrder->setupFiles[1], NULL);
  ck_assert_str_eq(axonOrder->setupFiles[1], "two");
  axon_freeOrderConfig(axonOrder);

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "setup:\nseed:\n  - one\n  - two\n");
  axonOrder = axon_readOrderConfig();
  ck_assert_ptr_ne(axonOrder, NULL);
  ck_assert_int_eq(axonOrder->setupLen, 0);
  ck_assert_int_le(axonOrder->seedLen, 2);
  ck_assert_ptr_eq(axonOrder->setupFiles, NULL);
  ck_assert_ptr_ne(axonOrder->seedFiles, NULL);
  ck_assert_ptr_ne(axonOrder->seedFiles[0], NULL);
  ck_assert_str_eq(axonOrder->seedFiles[0], "one");
  ck_assert_ptr_ne(axonOrder->seedFiles[1], NULL);
  ck_assert_str_eq(axonOrder->seedFiles[1], "two");
  axon_freeOrderConfig(axonOrder);

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "setup:\n  seed:\n  - test\n");
  axonOrder = axon_readOrderConfig();
  ck_assert_ptr_ne(axonOrder, NULL);
  axon_freeOrderConfig(axonOrder);
END_TEST

START_TEST(test_malformedOrder)
  GO_TO_DUMMY
  AxonOrder *axonOrder = NULL;

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "hello:\n  seed:\n  - test\n");
  axonOrder = axon_readOrderConfig();
  ck_assert_ptr_ne(axonOrder, NULL);
  axon_freeOrderConfig(axonOrder);

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "seed:\n  -seed:\n  - test\n");
  axonOrder = axon_readOrderConfig();
  ck_assert_ptr_ne(axonOrder, NULL);
  axon_freeOrderConfig(axonOrder);

  ck_overrideFile(AXON_ORDER_CONFIG_FILE, "seed:\n  - seed:\n  -test\n");
  axonOrder = axon_readOrderConfig();
  ck_assert_ptr_ne(axonOrder, NULL);
  axon_freeOrderConfig(axonOrder);
END_TEST

START_TEST(test_triggersConfig)
  GO_TO_DUMMY
  ck_unlink("./conf/triggers.yml");
  IN_CLEAR_STATE(/* */)

  AxonTriggersConfig *config = NULL;
  config = axon_readTriggersConfig();
  ck_assert_ptr_ne(config, NULL);
  ck_assert_ptr_ne(config->flags, NULL);
  ck_assert_ptr_ne(config->libs, NULL);
  axon_freeTriggersConfig(config);

  ck_overrideFile(AXON_TRIGGERS_FILE, "  ");
  config = axon_readTriggersConfig();
  ck_assert_ptr_ne(config, NULL);
  axon_freeTriggersConfig(config);

  ck_overrideFile(AXON_TRIGGERS_FILE, "  :");
  config = axon_readTriggersConfig();
  ck_assert_ptr_ne(config, NULL);
  axon_freeTriggersConfig(config);

  ck_overrideFile(AXON_TRIGGERS_FILE, "foo: bar");
  config = axon_readTriggersConfig();
  ck_assert_ptr_ne(config, NULL);
  axon_freeTriggersConfig(config);
END_TEST

void test_config(Suite *s) {
  TCase *testCaseConfig = tcase_create("Config");
  tcase_add_test(testCaseConfig, test_readConfig);
  tcase_add_test(testCaseConfig, test_malformedDatabaseFile);
  tcase_add_test(testCaseConfig, test_flavorFromFile);
  tcase_add_test(testCaseConfig, test_flavorFromMultilineFile);
  tcase_add_test(testCaseConfig, test_flavorNoSource);
  tcase_add_test(testCaseConfig, test_readOrderWithoutOrderFile);
  tcase_add_test(testCaseConfig, test_readOrderWithEmptyOrderFile);
  tcase_add_test(testCaseConfig, test_axonOrder);
  tcase_add_test(testCaseConfig, test_malformedOrder);
  tcase_add_test(testCaseConfig, test_triggersConfig);
  suite_add_tcase(s, testCaseConfig);
}
