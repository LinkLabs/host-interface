#include "ll_ifc_symphony.h"
#include "utils_ifc_lib.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmockery.h"

void test_config_set_net_token_invalid(void **state)
{
    (void) state;
    uint32_t net_token = 0xffffffff;
    uint8_t app_token[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    enum ll_downlink_mode dl_mode = LL_DL_MAILBOX;
    uint8_t qos = 8;

    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_config_set(net_token, app_token, dl_mode, qos));
}

void test_config_set_app_token_invalid(void **state)
{
    (void) state;
    uint32_t net_token = 0x11223344;
    uint8_t* app_token = NULL;
    enum ll_downlink_mode dl_mode = LL_DL_MAILBOX;
    uint8_t qos = 8;

    uint8_t cmd[] = {0x11, 0x22, 0x33, 0x44};
    transport_expect(OP_NET_TOKEN_SET, cmd, sizeof(cmd), 0, 0, 0);

    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_config_set(net_token, app_token, dl_mode, qos));
}

void test_config_set_dl_mode_invalid(void **state)
{
    (void) state;
    uint32_t net_token = 0x11223344;
    uint8_t app_token[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    enum ll_downlink_mode dl_mode = NUM_DOWNLINK_MODES;
    uint8_t qos = 8;

    uint8_t cmd1[] = {0x11, 0x22, 0x33, 0x44};
    transport_expect(OP_NET_TOKEN_SET, cmd1, sizeof(cmd1), 0, 0, 0);

    transport_expect(OP_APP_TOKEN_SET, app_token, sizeof(app_token), 0, 0, 0);

    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_config_set(net_token, app_token, dl_mode, qos));
}

void test_config_set_qos_invalid(void **state)
{
    (void) state;
    uint32_t net_token = 0x11223344;
    uint8_t app_token[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    enum ll_downlink_mode dl_mode = NUM_DOWNLINK_MODES;
    uint8_t qos = 16;

    uint8_t cmd1[] = {0x11, 0x22, 0x33, 0x44};
    transport_expect(OP_NET_TOKEN_SET, cmd1, sizeof(cmd1), 0, 0, 0);
    transport_expect(OP_APP_TOKEN_SET, app_token, sizeof(app_token), 0, 0, 0);
    transport_expect(OP_RX_MODE_SET, (uint8_t*)&dl_mode, 1, 0, 0, 0);

    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_config_set(net_token, app_token, dl_mode, qos));
}

void test_config_set_ok(void **state)
{
    (void) state;
    uint32_t net_token = 0x11223344;
    uint8_t app_token[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    enum ll_downlink_mode dl_mode = LL_DL_MAILBOX;
    uint8_t qos = 8;

    uint8_t cmd1[] = {0x11, 0x22, 0x33, 0x44};
    transport_expect(OP_NET_TOKEN_SET, cmd1, sizeof(cmd1), 0, 0, 0);
    transport_expect(OP_APP_TOKEN_SET, app_token, sizeof(app_token), 0, 0, 0);
    transport_expect(OP_RX_MODE_SET, (uint8_t*)&dl_mode, 1, 0, 0, 0);
    transport_expect(OP_QOS_REQUEST, &qos, 1, 0, 0, 0);

    assert_int_equal(LL_IFC_ACK, ll_config_set(net_token, app_token, dl_mode, qos));
}

void test_config_get_net_token_invalid(void **state)
{
    (void) state;
    uint8_t app_token[APP_TOKEN_LEN] = {0};
    enum ll_downlink_mode dl_mode = NUM_DOWNLINK_MODES;
    uint8_t qos = 0xff;

    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_config_get(NULL, app_token, &dl_mode, &qos));
}

void test_config_get_net_token_incorrect_length(void **state)
{
    (void) state;
    uint32_t net_token = 0xffffffff;
    uint8_t app_token[APP_TOKEN_LEN] = {0};
    enum ll_downlink_mode dl_mode = NUM_DOWNLINK_MODES;
    uint8_t qos = 0xff;

    uint8_t rsp[] = {0x11, 0x22, 0x33};
    transport_expect(OP_NET_TOKEN_GET, 0, 0, 0, rsp, sizeof(rsp));

    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_config_get(&net_token, app_token, &dl_mode, &qos));
    assert_int_equal(0xffffffff, net_token);
}


void test_config_get_app_token_invalid(void **state)
{
    (void) state;
    uint32_t net_token = 0xffffffff;
    enum ll_downlink_mode dl_mode = NUM_DOWNLINK_MODES;
    uint8_t qos = 0xff;

    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44};
    transport_expect(OP_NET_TOKEN_GET, 0, 0, 0, rsp, sizeof(rsp));

    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_config_get(&net_token, NULL, &dl_mode, &qos));
    assert_int_equal(0x11223344, net_token);
}

void test_config_get_app_token_incorrect_length(void **state)
{
    (void) state;
    uint32_t net_token = 0xffffffff;
    uint8_t app_token[APP_TOKEN_LEN] = {0};
    enum ll_downlink_mode dl_mode = NUM_DOWNLINK_MODES;
    uint8_t qos = 0xff;

    uint8_t rsp1[] = {0x11, 0x22, 0x33, 0x44};
    transport_expect(OP_NET_TOKEN_GET, 0, 0, 0, rsp1, sizeof(rsp1));
    uint8_t rsp2[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    transport_expect(OP_APP_TOKEN_GET, 0, 0, 0, rsp2, sizeof(rsp2));

    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_config_get(&net_token, app_token, &dl_mode, &qos));
    assert_int_equal(0x11223344, net_token);
}

void test_config_get_dl_mode_invalid(void **state)
{
    (void) state;
    uint32_t net_token = 0xffffffff;
    uint8_t app_token[APP_TOKEN_LEN] = {0};
    uint8_t qos = 0xff;

    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44};
    transport_expect(OP_NET_TOKEN_GET, 0, 0, 0, rsp, sizeof(rsp));
    uint8_t rsp2[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    transport_expect(OP_APP_TOKEN_GET, 0, 0, 0, rsp2, sizeof(rsp2));

    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_config_get(&net_token, app_token, NULL, &qos));
    assert_int_equal(0x11223344, net_token);
    assert_memory_equal(app_token, rsp2, APP_TOKEN_LEN);
}

void test_config_get_dl_mode_incorrect_length(void **state)
{
    (void) state;
    uint32_t net_token = 0xffffffff;
    uint8_t app_token[APP_TOKEN_LEN] = {0};
    enum ll_downlink_mode dl_mode = NUM_DOWNLINK_MODES;
    uint8_t qos = 0xff;

    uint8_t rsp1[] = {0x11, 0x22, 0x33, 0x44};
    transport_expect(OP_NET_TOKEN_GET, 0, 0, 0, rsp1, sizeof(rsp1));
    uint8_t rsp2[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    transport_expect(OP_APP_TOKEN_GET, 0, 0, 0, rsp2, sizeof(rsp2));
    uint8_t rsp3[] = {0x11, 0x22};
    transport_expect(OP_RX_MODE_GET, 0, 0, 0, rsp3, sizeof(rsp3));

    assert_int_equal(LL_IFC_ERROR_BUFFER_TOO_SMALL, ll_config_get(&net_token, app_token, &dl_mode, &qos));
    assert_int_equal(0x11223344, net_token);
    assert_memory_equal(app_token, rsp2, APP_TOKEN_LEN);
}

void test_config_get_qos_invalid(void **state)
{
    (void) state;
    uint32_t net_token = 0xffffffff;
    uint8_t app_token[APP_TOKEN_LEN] = {0};
    enum ll_downlink_mode dl_mode = NUM_DOWNLINK_MODES;

    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44};
    transport_expect(OP_NET_TOKEN_GET, 0, 0, 0, rsp, sizeof(rsp));
    uint8_t rsp2[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    transport_expect(OP_APP_TOKEN_GET, 0, 0, 0, rsp2, sizeof(rsp2));
    uint8_t rsp3[] = {0x02};
    transport_expect(OP_RX_MODE_GET, 0, 0, 0, rsp3, sizeof(rsp3));

    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_config_get(&net_token, app_token, &dl_mode, NULL));
    assert_int_equal(0x11223344, net_token);
    assert_memory_equal(app_token, rsp2, APP_TOKEN_LEN);
    assert_int_equal(LL_DL_MAILBOX, dl_mode);
}

void test_config_get_qos_incorrect_length(void **state)
{
    (void) state;
    uint32_t net_token = 0xffffffff;
    uint8_t app_token[APP_TOKEN_LEN] = {0};
    enum ll_downlink_mode dl_mode = NUM_DOWNLINK_MODES;
    uint8_t qos = 0xff;

    uint8_t rsp1[] = {0x11, 0x22, 0x33, 0x44};
    transport_expect(OP_NET_TOKEN_GET, 0, 0, 0, rsp1, sizeof(rsp1));
    uint8_t rsp2[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    transport_expect(OP_APP_TOKEN_GET, 0, 0, 0, rsp2, sizeof(rsp2));
    uint8_t rsp3[] = {0x02};
    transport_expect(OP_RX_MODE_GET, 0, 0, 0, rsp3, sizeof(rsp3));
    uint8_t rsp4[] = {0x02, 0x23};
    transport_expect(OP_QOS_GET, 0, 0, 0, rsp4, sizeof(rsp4));

    assert_int_equal(LL_IFC_ERROR_BUFFER_TOO_SMALL, ll_config_get(&net_token, app_token, &dl_mode, &qos));
    assert_int_equal(0x11223344, net_token);
    assert_memory_equal(app_token, rsp2, APP_TOKEN_LEN);
    assert_int_equal(LL_DL_MAILBOX, dl_mode);
    assert_int_equal(0xff, qos);
}

void test_config_get_ok(void **state)
{
    (void) state;
    uint32_t net_token = 0xffffffff;
    uint8_t app_token[APP_TOKEN_LEN] = {0};
    enum ll_downlink_mode dl_mode = NUM_DOWNLINK_MODES;
    uint8_t qos = 0xff;

    uint8_t rsp1[] = {0x11, 0x22, 0x33, 0x44};
    transport_expect(OP_NET_TOKEN_GET, 0, 0, 0, rsp1, sizeof(rsp1));
    uint8_t rsp2[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
    transport_expect(OP_APP_TOKEN_GET, 0, 0, 0, rsp2, sizeof(rsp2));
    uint8_t rsp3[] = {0x02};
    transport_expect(OP_RX_MODE_GET, 0, 0, 0, rsp3, sizeof(rsp3));
    uint8_t rsp4[] = {0x07};
    transport_expect(OP_QOS_GET, 0, 0, 0, rsp4, sizeof(rsp4));

    assert_int_equal(LL_IFC_ACK, ll_config_get(&net_token, app_token, &dl_mode, &qos));
    assert_int_equal(0x11223344, net_token);
    assert_memory_equal(app_token, rsp2, APP_TOKEN_LEN);
    assert_int_equal(LL_DL_MAILBOX, dl_mode);
    assert_int_equal(0x07, qos);
}

void test_get_state_all_null(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ACK, ll_get_state(NULL, NULL, NULL));
}

void test_get_state_ok(void **state)
{
    (void) state;
    enum ll_state s = LL_STATE_ERROR;
    enum ll_tx_state stx = LL_TX_STATE_ERROR;
    enum ll_rx_state srx = LL_RX_STATE_NO_MSG;

    uint8_t rsp1[] = {3};
    transport_expect(OP_STATE, 0, 0, 0, rsp1, sizeof(rsp1));
    uint8_t rsp2[] = {1};
    transport_expect(OP_TX_STATE, 0, 0, 0, rsp2, sizeof(rsp2));
    uint8_t rsp3[] = {1};
    transport_expect(OP_RX_STATE, 0, 0, 0, rsp3, sizeof(rsp3));

    assert_int_equal(LL_IFC_ACK, ll_get_state(&s, &stx, &srx));
    assert_int_equal(LL_STATE_INITIALIZING, s);
    assert_int_equal(LL_TX_STATE_TRANSMITTING, stx);
    assert_int_equal(LL_RX_STATE_RECEIVED_MSG, srx);
}

void test_mailbox_request_ok(void **state)
{
    (void)state;
    transport_expect(OP_MAILBOX_REQUEST, 0, 0, 0, 0, 0);
    assert_int_equal(LL_IFC_ACK, ll_mailbox_request());
}

void test_app_reg_get_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_app_reg_get(NULL));
}

void test_app_reg_get_ok(void **state)
{
    (void)state;
    uint8_t is_registered = 0;
    uint8_t rsp[] = {0x01};
    transport_expect(OP_APP_TOKEN_REG_GET, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ACK, ll_app_reg_get(&is_registered));
    assert_int_equal(1, is_registered);
}

void test_crypto_request_ok(void **state)
{
    (void)state;
    transport_expect(OP_CRYPTO_KEY_XCHG_REQ, 0, 0, 0, 0, 0);
    assert_int_equal(LL_IFC_ACK, ll_encryption_key_exchange_request());
}

void test_net_info_get_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_net_info_get(NULL));
}

void test_net_info_get_incorrect_length(void **state)
{
    (void)state;
    llabs_network_info_t info;

    uint8_t rsp[NET_INFO_BUFF_SIZE - 1] = {0};
    transport_expect(OP_NET_INFO_GET, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_net_info_get(&info));
}

void test_net_info_get_ok(void **state)
{
    (void)state;
    llabs_network_info_t info;

    uint8_t rsp[NET_INFO_BUFF_SIZE] = {0};
    transport_expect(OP_NET_INFO_GET, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ACK, ll_net_info_get(&info));
}

void test_stats_get_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_stats_get(NULL));
}

void test_stats_get_incorrect_length(void **state)
{
    (void)state;
    llabs_stats_t s;

    uint8_t rsp[STATS_SIZE - 1] = {0};
    transport_expect(OP_STATS_GET, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_stats_get(&s));
}

void test_stats_get_ok(void **state)
{
    (void)state;
    llabs_stats_t s;

    uint8_t rsp[STATS_SIZE] = {0};
    transport_expect(OP_STATS_GET, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ACK, ll_stats_get(&s));
}

void test_retrieve_message_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_retrieve_message(NULL, NULL, NULL, NULL, NULL));
}

void test_retrieve_message_small_length(void **state)
{
    (void)state;
    uint8_t buff[256] = {0};
    uint16_t size = 1;

    uint8_t rsp[] = {0, 0};
    transport_expect(OP_MSG_RECV, 0, 0, 0, rsp, sizeof(rsp));

    assert_int_equal(LL_IFC_ERROR_BUFFER_TOO_SMALL, ll_retrieve_message(buff, &size, NULL, NULL, NULL));
}

void test_retrieve_message_incorrect_length(void **state)
{
    (void)state;
    uint8_t buff[256] = {0};
    uint16_t size = 256;

    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44};
    transport_expect(OP_MSG_RECV, 0, 0, 0, rsp, sizeof(rsp));

    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_retrieve_message(buff, &size, NULL, NULL, NULL));
}

void test_retrieve_message_ok(void **state)
{
    (void)state;
    uint8_t buff[256] = {0};
    uint16_t size = 256;
    uint8_t port = 0;
    int16_t rssi = 0;
    uint8_t snr = 0;

    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    transport_expect(OP_MSG_RECV, 0, 0, 0, rsp, sizeof(rsp));

    assert_int_equal(LL_IFC_ACK, ll_retrieve_message(buff, &size, &port, &rssi, &snr));
    assert_int_equal(2, size);
    assert_memory_equal(buff, &rsp[4], 2);
    assert_int_equal(0x44, port);
    assert_int_equal(0x2211, rssi);
    assert_int_equal(0x33, snr);
}

void test_dl_band_cfg_get_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_dl_band_cfg_get(NULL));
}

void test_dl_band_cfg_get_incorrect_length(void **state)
{
    (void)state;
    llabs_dl_band_cfg_t cfg;

    uint8_t rsp[] = {0, 0};
    transport_expect(OP_DL_BAND_CFG_GET, 0, 0, 0, rsp, sizeof(rsp));

    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_dl_band_cfg_get(&cfg));
}

void test_dl_band_cfg_get_ok(void **state)
{
    (void)state;
    llabs_dl_band_cfg_t cfg;

    uint8_t rsp[DL_BAND_CFG_SIZE] = {0};
    transport_expect(OP_DL_BAND_CFG_GET, 0, 0, 0, rsp, sizeof(rsp));

    assert_int_equal(LL_IFC_ACK, ll_dl_band_cfg_get(&cfg));
}

void test_dl_band_cfg_set_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_dl_band_cfg_set(NULL));
}

void test_dl_band_cfg_set_ok(void **state)
{
    (void)state;
    llabs_dl_band_cfg_t cfg = {0};

    uint8_t cmd[DL_BAND_CFG_SIZE] = {0};
    transport_expect(OP_DL_BAND_CFG_SET, cmd, sizeof(cmd), 0, 0, 0);

    assert_int_equal(LL_IFC_ACK, ll_dl_band_cfg_set(&cfg));
}


void test_connection_filter_get_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_connection_filter_get(NULL));
}

void test_connection_filter_get_incorrect_length(void **state)
{
    (void)state;
    uint8_t f = 0xff;

    uint8_t rsp[] = {0, 0};
    transport_expect(OP_CONN_FILT_GET, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ERROR_BUFFER_TOO_SMALL, ll_connection_filter_get(&f));
    assert_int_equal(0xff, f);
}

void test_connection_filter_get_ok(void **state)
{
    (void)state;
    uint8_t f = 0xff;

    uint8_t rsp[] = {0x11};
    transport_expect(OP_CONN_FILT_GET, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ACK, ll_connection_filter_get(&f));
    assert_int_equal(0x11, f);
}

void test_connection_filter_set_ok(void **state)
{
    (void)state;
    uint8_t f = 0x11;

    uint8_t cmd[] = {0x11};
    transport_expect(OP_CONN_FILT_SET, cmd, sizeof(cmd), 0, 0, 0);
    assert_int_equal(LL_IFC_ACK, ll_connection_filter_set(f));
}

void test_system_time_get_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_system_time_get(NULL));
}

void test_system_time_get_incorrect_length(void **state)
{
    (void)state;
    llabs_time_info_t info;

    uint8_t rsp[] = {0, 0};
    transport_expect(OP_SYSTEM_TIME_GET, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_system_time_get(&info));
    //assert_int_equal(0xff, f);
}

void test_system_time_get_ok(void **state)
{
    (void)state;
    llabs_time_info_t info;

    uint8_t rsp[TIME_INFO_SIZE] = {0};
    transport_expect(OP_SYSTEM_TIME_GET, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ACK, ll_system_time_get(&info));
}

void test_system_time_sync_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_system_time_sync(2));
}

void test_system_time_sync_ok(void **state)
{
    (void)state;

    uint8_t cmd[] = {1};
    transport_expect(OP_SYSTEM_TIME_SYNC, cmd, sizeof(cmd), 0, 0, 0);
    assert_int_equal(LL_IFC_ACK, ll_system_time_sync(1));
}

void test_message_send_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_message_send(NULL, 1, 1, 1));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_message_send(1, 0, 1, 1));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_message_send(1, 257, 0, 1));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_message_send(1, 1, 1, 128));
}

void test_message_send_ok(void **state)
{
    (void)state;
    uint8_t buf[] = {0x11, 0x22, 0x33};

    uint8_t cmd[] = {1, 0x55, 0x11, 0x22, 0x33};
    transport_expect(OP_MSG_SEND, cmd, sizeof(cmd), 0, 0, 0);
    assert_int_equal(LL_IFC_ACK, ll_message_send(buf, sizeof(buf), 1, 0x55));
}

void test_scan_config_set_ok(void **state)
{
    (void)state;

    int16_t threshold = 16;
    uint8_t cmd[] = { threshold & 0xff, (threshold >> 8) };
    uint16_t scan_attempts = 23;
    uint8_t cmd1[] = { scan_attempts & 0xff, (scan_attempts >> 8) };

    enum ll_scan_mode scan_mode = LLABS_NORMAL_SCAN_AND_CONNECT;

    transport_expect(OP_SCAN_MODE_SET, (uint8_t*)&scan_mode, 1, 0, 0, 0);
    transport_expect(OP_SCAN_THRESHOLD_SET, cmd, sizeof(cmd), 0, 0, 0);
    transport_expect(OP_SCAN_ATTEMPTS_SET, cmd1, sizeof(cmd1), 0, 0, 0);

    assert_int_equal(LL_IFC_ACK, ll_scan_config_set(scan_mode, threshold, scan_attempts));
}

void test_scan_config_set_invalid_scan_mode(void **state)
{
    (void)state;

    int16_t threshold = 16;
    uint8_t cmd[] = { threshold & 0xff, (threshold >> 8) };
    uint16_t scan_attempts = 23;
    uint8_t cmd1[] = { scan_attempts & 0xff, (scan_attempts >> 8) };
    enum ll_scan_mode scan_mode = -1;

    transport_expect(OP_SCAN_MODE_SET, (uint8_t*)&scan_mode, 1, LL_IFC_ERROR_INCORRECT_PARAMETER, 0, 0);
    transport_expect(OP_SCAN_THRESHOLD_SET, cmd, sizeof(cmd), 0, 0, 0);
    transport_expect(OP_SCAN_ATTEMPTS_SET, cmd1, sizeof(cmd1), 0, 0, 0);

    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_scan_config_set(scan_mode, threshold, scan_attempts));
}

void test_scan_config_get_ok(void **state)
{
    (void)state;

    // Variables
    enum ll_scan_mode scan_mode = 0;
    int16_t threshold = 0;
    uint16_t scan_attempts = 0;
    uint16_t scans_left = 0;

    // Expected Values
    enum ll_scan_mode ex_scan_mode = LLABS_INFO_SCAN;
    int16_t ex_threshold = -98;
    uint16_t ex_scan_attempts = 560;
    uint16_t ex_scans_left = 340;

    // Serialized Data "From the module"
    transport_expect(OP_SCAN_MODE_GET, 0, 0, 0, (uint8_t*)&ex_scan_mode, 1);
    transport_expect(OP_SCAN_THRESHOLD_GET, 0, 0, 0, (uint8_t*)&ex_threshold, 2);
    transport_expect(OP_SCAN_ATTEMPTS_GET, 0, 0, 0, (uint8_t*)&ex_scan_attempts, 2);
    transport_expect(OP_SCAN_ATTEMPTS_LEFT, 0, 0, 0, (uint8_t*)&ex_scans_left, 2);

    // Checks
    assert_int_equal(LL_IFC_ACK, ll_scan_config_get(&scan_mode, &threshold, &scan_attempts, &scans_left));
    assert_int_equal(scan_mode, ex_scan_mode);
    assert_int_equal(threshold, ex_threshold);
    assert_int_equal(scan_attempts, ex_scan_attempts);
    assert_int_equal(scans_left, ex_scans_left);
}

void test_connect_to_gw_channel_ok(void **state)
{
    (void)state;
    for (int i = 0; i <= 45; i += 3)
    {
        transport_expect(OP_CONN_TO_GW_CH, (uint8_t*)&i, 1, 0, 0, 0);
        assert_int_equal(LL_IFC_ACK, ll_connect_to_gw_channel(i));
    }
}

