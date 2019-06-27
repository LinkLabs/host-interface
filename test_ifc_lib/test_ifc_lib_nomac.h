#include "test_ifc_lib_gen.h"

void test_rssi_scan_set_ok(void **state);
void test_rssi_scan_get_invalid(void **state);
void test_rssi_scan_get_short_length(void **state);
void test_rssi_scan_get_ok(void **state);
void test_radio_params_get_invalid(void **state);
void test_radio_params_get_incorrect_length(void **state);
void test_radio_params_get_ok(void **state);
void test_radio_params_set_invalid(void **state);
void test_radio_params_set_ok(void **state);
void test_tx_power_set_invalid(void **state);
void test_tx_power_set_ok(void **state);
void test_tx_power_get_invalid(void **state);
void test_tx_power_get_ok(void **state);
void test_sync_word_set_ok(void **state);
void test_sync_word_get_invalid(void **state);
void test_sync_word_get_ok(void **state);
void test_echo_mode_ok(void **state);
void test_packet_send_queue_invalid(void **state);
void test_packet_send_queue_queued(void **state);
void test_packet_send_queue_full(void **state);
void test_packet_send_queue_error(void **state);
void test_transmit_cw_ok(void **state);
void test_packet_recv_cont_invalid(void **state);
void test_packet_recv_cont_small_length(void **state);
void test_packet_recv_cont_ok(void **state);
void test_packet_recv_with_rssi_invalid(void **state);
void test_packet_recv_with_rssi_small_length(void **state);
void test_packet_recv_with_rssi_ok(void **state);

#define NOMAC_IFC_LIB_TESTS \
        unit_test_setup_teardown(test_rssi_scan_set_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_rssi_scan_get_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_rssi_scan_get_short_length, setUp, tearDown), \
        unit_test_setup_teardown(test_rssi_scan_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_radio_params_get_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_radio_params_get_incorrect_length, setUp, tearDown), \
        unit_test_setup_teardown(test_radio_params_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_radio_params_set_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_radio_params_set_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_tx_power_set_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_tx_power_set_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_tx_power_get_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_tx_power_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_sync_word_set_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_sync_word_get_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_sync_word_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_echo_mode_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_packet_send_queue_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_packet_send_queue_queued, setUp, tearDown), \
        unit_test_setup_teardown(test_packet_send_queue_full, setUp, tearDown), \
        unit_test_setup_teardown(test_packet_send_queue_error, setUp, tearDown), \
        unit_test_setup_teardown(test_transmit_cw_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_packet_recv_cont_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_packet_recv_cont_small_length, setUp, tearDown), \
        unit_test_setup_teardown(test_packet_recv_cont_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_packet_recv_with_rssi_invalid, setUp, tearDown), \
        unit_test_setup_teardown(test_packet_recv_with_rssi_small_length, setUp, tearDown), \
        unit_test_setup_teardown(test_packet_recv_with_rssi_ok, setUp, tearDown), \
        
        


