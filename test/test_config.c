#include "test_config.h"
#include "support/reset_environment.h"

START_TEST(test_readConfig)
  ck_unlink("./conf/database.yml");

  AxonConfig *axonConfig = axon_readConfig();
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

  axon_freeConfig(axonConfig);
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
  ck_unlink("./db/order.yml");
  AxonOrder *order = NULL;
  ck_redirectStderr(order = axon_readOrder();)
  ck_assert_ptr_eq(order, NULL);
  ck_path_contains("./log/error.log", "db/order.yml does not exists!");
END_TEST

START_TEST(test_readOrderWithEmptyOrderFile)
  GO_TO_DUMMY
  IN_CLEAR_STATE(/* */)
  ck_unlink("./db/order.yml");
  axon_touch("./db/order.yml");
  AxonOrder *order = NULL;
  ck_redirectStderr(order = axon_readOrder();)
  ck_assert_ptr_ne(order, NULL);
  ck_assert_int_eq(order->seedLen, 0);
  ck_assert_int_eq(order->setupLen, 0);
  ck_assert_ptr_eq(order->seedFiles, NULL);
  ck_assert_ptr_eq(order->setupFiles, NULL);
  axon_freeOrder(order);
END_TEST

void test_config(Suite *s) {
  TCase *testCaseConfig = tcase_create("Config");
  tcase_add_test(testCaseConfig, test_readConfig);
  tcase_add_test(testCaseConfig, test_flavorFromFile);
  tcase_add_test(testCaseConfig, test_flavorFromMultilineFile);
  tcase_add_test(testCaseConfig, test_flavorNoSource);
  tcase_add_test(testCaseConfig, test_readOrderWithoutOrderFile);
  tcase_add_test(testCaseConfig, test_readOrderWithEmptyOrderFile);
  suite_add_tcase(s, testCaseConfig);
}
