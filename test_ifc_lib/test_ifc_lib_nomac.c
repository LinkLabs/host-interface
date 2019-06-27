#include "ll_ifc_no_mac.h"
#include "ll_ifc.h"
#include "utils_ifc_lib.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmockery.h"

void test_rssi_scan_set_ok(void **state)
{
    (void) state;
    uint8_t cmd[16] = {0};
    transport_expect(OP_RSSI_SET, cmd, sizeof(cmd), 0, 0, 0);
    assert_int_equal(LL_IFC_ACK, ll_rssi_scan_set(0, 0, 0, 0));
}

void test_rssi_scan_get_invalid(void **state)
{
    (void) state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_rssi_scan_get(NULL, 1, 1));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_rssi_scan_get(1, 0, 1));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_rssi_scan_get(1, 1, NULL));
}

void test_rssi_scan_get_short_length(void **state)
{
    (void) state;
    uint8_t buf[255] = {0};
    uint8_t bytes_received = 0xff;

    uint8_t rsp[16] = {0};
    transport_expect(OP_RSSI_GET, 0, 0, 0, rsp, sizeof(rsp));

    assert_int_equal(LL_IFC_ERROR_BUFFER_TOO_SMALL, ll_rssi_scan_get(buf, 1, &bytes_received));
    assert_int_equal(0, bytes_received);
}

void test_rssi_scan_get_ok(void **state)
{
    (void) state;
    uint8_t buf[255] = {0};
    uint8_t bytes_received = 0xff;

    uint8_t rsp[16] = {0};
    transport_expect(OP_RSSI_GET, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ACK, ll_rssi_scan_get(buf, 255, &bytes_received));
    assert_int_equal(16, bytes_received);
}

void test_radio_params_get_invalid(void **state)
{
    (void) state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_radio_params_get(NULL, 1, 1, 1, 1, 1, 1, 1));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_radio_params_get(1, NULL, 1, 1, 1, 1, 1, 1));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_radio_params_get(1, 1, NULL, 1, 1, 1, 1, 1));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_radio_params_get(1, 1, 1, NULL, 1, 1, 1, 1));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_radio_params_get(1, 1, 1, 1, NULL, 1, 1, 1));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_radio_params_get(1, 1, 1, 1, 1, NULL, 1, 1));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_radio_params_get(1, 1, 1, 1, 1, 1, NULL, 1));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_radio_params_get(1, 1, 1, 1, 1, 1, 1, NULL));
}

void test_radio_params_get_incorrect_length(void **state)
{
    (void) state;
    uint8_t sf, cr, bw, header_enabled, crc_enabled, iq_inverted;
    uint32_t freq;
    uint16_t preamble_syms;

    uint8_t rsp[7] = {0};
    transport_expect(OP_GET_RADIO_PARAMS, 0, 0, 0, rsp, sizeof(rsp));

    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_radio_params_get(&sf, &cr, &bw, &freq, &preamble_syms,
                                                                            &header_enabled, &crc_enabled, &iq_inverted));
}

void test_radio_params_get_ok(void **state)
{
    (void) state;
    uint8_t sf, cr, bw, header_enabled, crc_enabled, iq_inverted;
    uint32_t freq;
    uint16_t preamble_syms;

    uint8_t rsp[] = {0x37, 0x03, 0x12, 0x34, 0x36, 0x89, 0xca, 0xc0};
    transport_expect(OP_GET_RADIO_PARAMS, 0, 0, 0, rsp, sizeof(rsp));

    assert_int_equal(LL_IFC_ACK, ll_radio_params_get(&sf, &cr, &bw, &freq, &preamble_syms,
                                                                            &header_enabled, &crc_enabled, &iq_inverted));
    assert_int_equal(9, sf);
    assert_int_equal(2, cr);
    assert_int_equal(3, bw);
    assert_int_equal(1, header_enabled);
    assert_int_equal(1, crc_enabled != 0);
    assert_int_equal(0, iq_inverted);
    assert_int_equal(915000000, freq);
    assert_int_equal(0x1234, preamble_syms);
}

void test_radio_params_set_invalid(void **state)
{
    (void) state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_radio_params_set(RADIO_PARAM_FLAGS_SF, 3, 0, 0, 0, 0, 0, 0, 0));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_radio_params_set(RADIO_PARAM_FLAGS_CR, 0, 5, 0, 0, 0, 0, 0, 0));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_radio_params_set(RADIO_PARAM_FLAGS_BW, 0, 0, 4, 0, 0, 0, 0, 0));
}

void test_radio_params_set_ok(void **state)
{
    (void) state;
    uint8_t flags = RADIO_PARAM_FLAGS_SF | RADIO_PARAM_FLAGS_CR | RADIO_PARAM_FLAGS_BW | RADIO_PARAM_FLAGS_HEADER |
                    RADIO_PARAM_FLAGS_CRC | RADIO_PARAM_FLAGS_IQ | RADIO_PARAM_FLAGS_PREAMBLE | RADIO_PARAM_FLAGS_FREQ;
    uint8_t sf = 9;
    uint8_t cr = 1;
    uint8_t bw = 3;
    uint32_t freq = 915000000;
    uint16_t preamble_syms = 8;
    uint8_t enable_header = 1;
    uint8_t enable_crc = 1;
    uint8_t enable_iq_inversion = 1;

    uint8_t cmd[] = {flags, ((sf-6)<<4)|((cr-1)<<2)|(bw), 7, 0, preamble_syms, 0x36, 0x89, 0xca, 0xc0};
    transport_expect(OP_SET_RADIO_PARAMS, cmd, sizeof(cmd), 0, 0, 0);

    assert_int_equal(LL_IFC_ACK, ll_radio_params_set(flags, sf, cr, bw, freq, preamble_syms,
                                                     enable_header, enable_crc, enable_iq_inversion));
}

void test_tx_power_set_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_tx_power_set(27));
}

void test_tx_power_set_ok(void **state)
{
    (void)state;

    uint8_t cmd[] = {20};
    transport_expect(OP_TX_POWER_SET, cmd, sizeof(cmd), 0, 0, 0);

    assert_int_equal(LL_IFC_ACK, ll_tx_power_set(20));
}

void test_tx_power_get_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_tx_power_get(NULL));
}

void test_tx_power_get_ok(void **state)
{
    (void)state;
    uint8_t pwr = 0;
    uint8_t rsp[] = {18};
    transport_expect(OP_TX_POWER_GET, 0, 0, 0, rsp, sizeof(rsp));

    assert_int_equal(LL_IFC_ACK, ll_tx_power_get(&pwr));
    assert_int_equal(18, pwr);
}

void test_sync_word_set_ok(void **state)
{
    (void)state;
    uint8_t sync_word = 0xa5;

    uint8_t cmd[] = {0xa5};
    transport_expect(OP_SYNC_WORD_SET, cmd, sizeof(cmd), 0, 0, 0);

    assert_int_equal(LL_IFC_ACK, ll_sync_word_set(sync_word));
}

void test_sync_word_get_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_sync_word_get(NULL));
}

void test_sync_word_get_ok(void **state)
{
    (void)state;
    uint8_t sync_word = 0;

    uint8_t rsp[] = {0xa5};
    transport_expect(OP_SYNC_WORD_GET, 0, 0, 0, rsp, sizeof(rsp));

    assert_int_equal(LL_IFC_ACK, ll_sync_word_get(&sync_word));
    assert_int_equal(0xa5, sync_word);
}

void test_echo_mode_ok(void **state)
{
    (void)state;
    transport_expect(OP_PKT_ECHO, 0, 0, 0, 0, 0);
    assert_int_equal(LL_IFC_ACK, ll_echo_mode());
}

void test_packet_send_queue_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_packet_send_queue(NULL, 1));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_packet_send_queue(1, 0));
}

void test_packet_send_queue_queued(void **state)
{
    (void)state;
    uint8_t buf[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint16_t len = 5;

    uint8_t cmd[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t rsp[] = {1};
    transport_expect(OP_PKT_SEND_QUEUE, cmd, sizeof(cmd), 0, rsp, sizeof(rsp));
    
    assert_int_equal(1, ll_packet_send_queue(buf, len));
}

void test_packet_send_queue_full(void **state)
{
    (void)state;
    uint8_t buf[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint16_t len = 5;

    uint8_t cmd[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t rsp[] = {0};
    transport_expect(OP_PKT_SEND_QUEUE, cmd, sizeof(cmd), 0, rsp, sizeof(rsp));
    
    assert_int_equal(0, ll_packet_send_queue(buf, len));
}

void test_packet_send_queue_error(void **state)
{
    (void)state;
    uint8_t buf[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint16_t len = 5;

    uint8_t cmd[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint8_t rsp[] = {-1};
    transport_expect(OP_PKT_SEND_QUEUE, cmd, sizeof(cmd), 0, rsp, sizeof(rsp));
    
    assert_int_equal(-1, (int8_t)ll_packet_send_queue(buf, len));
}

void test_transmit_cw_ok(void **state)
{
    (void)state;
    transport_expect(OP_TX_CW, 0, 0, 0, 0, 0);
    assert_int_equal(LL_IFC_ACK, ll_transmit_cw());
}

void test_packet_recv_cont_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_packet_recv_cont(NULL, 1, 1, false));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_packet_recv_cont(1, 0, 1, false));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_packet_recv_cont(1, 1, NULL, false));
}

void test_packet_recv_cont_small_length(void **state)
{
    (void)state;
    uint8_t buf[] = {0x11, 0x22};
    uint16_t len = sizeof(buf);
    uint8_t bytes_received = 0;
    bool freq_error_requested = false;
    
    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    transport_expect(OP_PKT_RECV_CONT, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ERROR_BUFFER_TOO_SMALL, ll_packet_recv_cont(buf, len, &bytes_received, freq_error_requested));
}

void test_packet_recv_cont_ok(void **state)
{
    (void)state;
    uint8_t buf[255] = {0};
    uint16_t len = sizeof(buf);
    uint8_t bytes_received = 0;
    bool freq_error_requested = false;
    
    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    transport_expect(OP_PKT_RECV_CONT, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ACK, ll_packet_recv_cont(buf, len, &bytes_received, freq_error_requested));
    assert_memory_equal(buf, rsp, bytes_received);
}

void test_packet_recv_with_rssi_invalid(void **state)
{
    (void)state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_packet_recv_with_rssi(0, NULL, 1, 1, false));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_packet_recv_with_rssi(0, 1, 0, 1, false));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_packet_recv_with_rssi(0, 1, 1, NULL, false));
}

void test_packet_recv_with_rssi_small_length(void **state)
{
    (void)state;
    uint8_t buf[] = {0x11, 0x22};
    uint16_t len = sizeof(buf);
    uint8_t bytes_received = 0;
    uint16_t num_timeout_symbols = 0x4455;
    bool freq_error_requested = false;
     
    uint8_t cmd[] = {0x44, 0x55};
    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    transport_expect(OP_MSG_RECV_RSSI, cmd, sizeof(cmd), 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ERROR_BUFFER_TOO_SMALL, ll_packet_recv_with_rssi(num_timeout_symbols, buf, len, &bytes_received, freq_error_requested));
}

void test_packet_recv_with_rssi_ok(void **state)
{
    (void)state;
    uint8_t buf[255] = {0};
    uint16_t len = sizeof(buf);
    uint8_t bytes_received = 0;
    uint16_t num_timeout_symbols = 0x4455;
    bool freq_error_requested = false;
     
    uint8_t cmd[] = {0x44, 0x55};
    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    transport_expect(OP_MSG_RECV_RSSI, cmd, sizeof(cmd), 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ACK, ll_packet_recv_with_rssi(num_timeout_symbols, buf, len, &bytes_received, freq_error_requested));
}

