#include "test_ifc_lib_gen.h"

void test_config_set_net_token_invalid(void **state);
void test_config_set_app_token_invalid(void **state);
void test_config_set_dl_mode_invalid(void **state);
void test_config_set_qos_invalid(void **state);
void test_config_set_ok(void **state);
void test_config_get_net_token_invalid(void **state);
void test_config_get_net_token_incorrect_length(void **state);
void test_config_get_app_token_invalid(void **state);
void test_config_get_app_token_incorrect_length(void **state);
void test_config_get_dl_mode_invalid(void **state);
void test_config_get_dl_mode_incorrect_length(void **state);
void test_config_get_qos_invalid(void **state);
void test_config_get_qos_incorrect_length(void **state);
void test_config_get_ok(void **state);
void test_get_state_all_null(void **state);
void test_get_state_ok(void **state);
void test_mailbox_request_ok(void **state);
void test_app_reg_get_invalid(void **state);
void test_app_reg_get_ok(void **state);
void test_crypto_request_ok(void **state);
void test_net_info_get_invalid(void **state);
void test_net_info_get_incorrect_length(void **state);
void test_net_info_get_ok(void **state);
void test_stats_get_invalid(void **state);
void test_stats_get_incorrect_length(void **state);
void test_stats_get_ok(void **state);
void test_dl_band_cfg_get_invalid(void **state);
void test_dl_band_cfg_get_incorrect_length(void **state);
void test_dl_band_cfg_get_ok(void **state);
void test_dl_band_cfg_set_invalid(void **state);
void test_dl_band_cfg_set_ok(void **state);
void test_connection_filter_get_invalid(void **state);
void test_connection_filter_get_incorrect_length(void **state);
void test_connection_filter_get_ok(void **state);
void test_connection_filter_set_ok(void **state);
void test_system_time_get_invalid(void **state);
void test_system_time_get_incorrect_length(void **state);
void test_system_time_get_ok(void **state);
void test_system_time_sync_invalid(void **state);
void test_system_time_sync_ok(void **state);
void test_message_send_invalid(void **state);
void test_message_send_ok(void **state);
void test_retrieve_message_invalid(void **state);
void test_retrieve_message_small_length(void **state);
void test_retrieve_message_incorrect_length(void **state);
void test_retrieve_message_ok(void **state);
void test_scan_config_set_ok(void **state);
void test_scan_config_set_invalid_scan_mode(void **state);
void test_scan_config_get_ok(void **state);
void test_connect_to_gw_channel_ok(void **state);

#define SYMPHONY_IFC_LIB_TESTS \
        unit_test_setup_teardown(test_config_set_net_token_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_config_set_app_token_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_config_set_dl_mode_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_config_set_qos_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_config_set_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_config_get_net_token_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_config_get_net_token_incorrect_length, setUp, tearDown), \
        unit_test_setup_teardown(test_config_get_app_token_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_config_get_app_token_incorrect_length, setUp, tearDown), \
        unit_test_setup_teardown(test_config_get_dl_mode_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_config_get_dl_mode_incorrect_length, setUp, tearDown), \
        unit_test_setup_teardown(test_config_get_qos_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_config_get_qos_incorrect_length, setUp, tearDown), \
        unit_test_setup_teardown(test_config_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_get_state_all_null, setUp, tearDown), \
        unit_test_setup_teardown(test_get_state_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_mailbox_request_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_app_reg_get_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_app_reg_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_crypto_request_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_net_info_get_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_net_info_get_incorrect_length, setUp, tearDown), \
        unit_test_setup_teardown(test_net_info_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_stats_get_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_stats_get_incorrect_length, setUp, tearDown), \
        unit_test_setup_teardown(test_stats_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_dl_band_cfg_get_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_dl_band_cfg_get_incorrect_length, setUp, tearDown), \
        unit_test_setup_teardown(test_dl_band_cfg_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_dl_band_cfg_set_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_dl_band_cfg_set_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_connection_filter_get_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_connection_filter_get_incorrect_length, setUp, tearDown), \
        unit_test_setup_teardown(test_connection_filter_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_connection_filter_set_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_system_time_get_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_system_time_get_incorrect_length, setUp, tearDown), \
        unit_test_setup_teardown(test_system_time_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_system_time_sync_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_system_time_sync_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_message_send_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_message_send_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_retrieve_message_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_retrieve_message_small_length, setUp, tearDown), \
        unit_test_setup_teardown(test_retrieve_message_incorrect_length, setUp, tearDown), \
        unit_test_setup_teardown(test_retrieve_message_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_scan_config_set_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_scan_config_set_invalid_scan_mode, setUp, tearDown), \
        unit_test_setup_teardown(test_scan_config_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_connect_to_gw_channel_ok, setUp, tearDown), \

