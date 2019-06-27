#include "ll_ifc.h"
#include "utils_ifc_lib.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmockery.h"

void setUp(void **state)
{
    (void) state;
    ifc_utils_setup();
    ll_reset_state();
}

void tearDown(void **state)
{
    (void) state;
    ifc_utils_teardown();
}

void test_myself_transport_write(void **state)
{
    (void) state;
    uint8_t cmd_payload[] = {1, 2, 3, 4};
    uint8_t cmd[] = {0xff, 0xff, 0xff, 0xff, 0xc4, 1, 0, 0, 4, 1, 2, 3, 4, 0, 0};
    uint8_t rsp_payload[] = {5, 6, 7, 8};
    uint8_t rsp[TRANSPORT_LEN_MAX];

    uint16_t crc = compute_checksum(cmd + 4, sizeof(cmd) - 6);
    cmd[sizeof(cmd) - 2] = (crc >> 8) & 0xff;
    cmd[sizeof(cmd) - 1] = (crc >> 0) & 0xff;

    transport_expect(1, cmd_payload, sizeof(cmd_payload), 0, rsp_payload, sizeof(rsp_payload));
    assert_int_equal(0, transport_write(cmd, sizeof(cmd)));
    assert_int_equal(0, transport_read(rsp, sizeof(rsp_payload) + 8));
    assert_memory_equal(rsp_payload, rsp + 6, sizeof(rsp_payload));
}

void test_timestamp_get(void **state)
{
    (void) state;
    uint8_t cmd[] = {0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44};

    transport_expect(OP_TIMESTAMP, cmd, sizeof(cmd), 0, rsp, sizeof(rsp));

    uint32_t actual_timestamp_us = 0;
    assert_int_equal(0, ll_timestamp_get(&actual_timestamp_us));
    assert_int_equal(0x11223344, actual_timestamp_us);
}

void test_timestamp_get_with_response_error(void **state)
{
    (void) state;
    uint8_t cmd[] = {0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t rsp[] = {0x11, 0x22, 0x33}; // too short

    transport_expect(OP_TIMESTAMP, cmd, sizeof(cmd), 0, rsp, sizeof(rsp));

    uint32_t actual_timestamp_us = 0;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_timestamp_get(&actual_timestamp_us));
}

void test_timestamp_get_twice_to_check_message_identifier(void **state)
{
    (void) state;
    uint8_t cmd[] = {0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44};

    transport_expect(OP_TIMESTAMP, cmd, sizeof(cmd), 0, rsp, sizeof(rsp));
    transport_expect(OP_TIMESTAMP, cmd, sizeof(cmd), 0, rsp, sizeof(rsp));

    uint32_t actual_timestamp_us = 0;
    assert_int_equal(0, ll_timestamp_get(&actual_timestamp_us));
    assert_int_equal(0x11223344, actual_timestamp_us);

    assert_int_equal(0, ll_timestamp_get(&actual_timestamp_us));
    assert_int_equal(0x11223344, actual_timestamp_us);
}

void test_timestamp_set(void **state)
{
    (void) state;
    uint8_t cmd[] = {0x01, 0x10, 0x20, 0x30, 0x40};
    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44};

    transport_expect(OP_TIMESTAMP, cmd, sizeof(cmd), 0, rsp, sizeof(rsp));

    uint32_t actual_timestamp_us = 0;
    assert_int_equal(0, ll_timestamp_set(1, 0x10203040, &actual_timestamp_us));
    assert_int_equal(0x11223344, actual_timestamp_us);
}

void test_timestamp_set_with_nack(void **state)
{
    (void) state;
    uint8_t cmd[] = {0x01, 0x10, 0x20, 0x30, 0x40};

    transport_expect(OP_TIMESTAMP, cmd, sizeof(cmd), LL_IFC_NACK_BUSY_TRY_AGAIN, 0, 0);

    uint32_t actual_timestamp_us = 0;
    assert_int_equal(-LL_IFC_NACK_BUSY_TRY_AGAIN, ll_timestamp_set(1, 0x10203040, &actual_timestamp_us));
}

void test_nack_with_payload(void **state)
{
    (void) state;
    uint8_t cmd[] = {0x01, 0x10, 0x20, 0x30, 0x40};
    uint8_t rsp[] = {1};

    transport_expect(OP_TIMESTAMP, cmd, sizeof(cmd), LL_IFC_NACK_BUSY_TRY_AGAIN, rsp, sizeof(rsp));

    uint32_t actual_timestamp_us = 0;
    // todo : indicating a CRC error does not seem correct here
    assert_int_equal(-104, ll_timestamp_set(1, 0x10203040, &actual_timestamp_us));
}

void test_timestamp_set_with_invalid_operation(void **state)
{
    (void) state;
    //TODO: not sure what this is supposed to be testing
//    uint32_t actual_timestamp_us = 0;
//    printf("negative\n");
//    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER,
//            ll_timestamp_set((ll_timestamp_operation_t) -1,
//                             0x10203040, &actual_timestamp_us));
//    printf("too big\n");
//    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER,
//            ll_timestamp_set((ll_timestamp_operation_t) LL_TIMESTAMP_SYNC + 1,
//                             0x10203040, &actual_timestamp_us));
}

void test_packet_send_timestamp(void **state)
{
    (void) state;
    uint8_t cmd1[] = {0x10, 0x20, 0x30, 0x40};

    uint8_t cmd2[] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t rsp2[] = {0};

    transport_expect(OP_SEND_TIMESTAMP, cmd1, sizeof(cmd1), 0, 0, 0);
    transport_expect(OP_PKT_SEND_QUEUE, cmd2, sizeof(cmd2), 0, rsp2, sizeof(rsp2));
    assert_int_equal(0, ll_packet_send_timestamp(0x10203040, cmd2, sizeof(cmd2)));
}

void test_fw_type_get_null_input(void **state)
{
    (void) state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_firmware_type_get(NULL));
}

void test_fw_type_get_return_oor(void **state)
{
    (void) state;
    uint8_t rsp[] = {0x11, 0x22}; // too short

    transport_expect(OP_FIRMWARE_TYPE, 0, 0, 0, rsp, sizeof(rsp));

    ll_firmware_type_t t;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_firmware_type_get(&t));
}

void test_fw_type_get_return_ok(void **state)
{
    (void) state;
    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44};

    transport_expect(OP_FIRMWARE_TYPE, 0, 0, 0, rsp, sizeof(rsp));

    ll_firmware_type_t t;
    assert_int_equal(LL_IFC_ACK, ll_firmware_type_get(&t));
    assert_int_equal(0x1122, t.cpu_code);
    assert_int_equal(0x3344, t.functionality_code);
}

void test_hw_type_get_null_input(void **state)
{
    (void) state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_hardware_type_get(NULL));
}

void test_hw_type_get_return_oor(void **state)
{
    (void) state;
    uint8_t rsp[] = {0x11, 0x22}; // too short

    transport_expect(OP_HARDWARE_TYPE, 0, 0, 0, rsp, sizeof(rsp));

    ll_hardware_type_t t;
    assert_int_equal(LL_IFC_ERROR_BUFFER_TOO_SMALL, ll_hardware_type_get(&t));
}

void test_hw_type_get_return_ok(void **state)
{
    (void) state;
    uint8_t rsp[] = {0x11};

    transport_expect(OP_HARDWARE_TYPE, 0, 0, 0, rsp, sizeof(rsp));

    ll_hardware_type_t t;
    assert_int_equal(LL_IFC_ACK, ll_hardware_type_get(&t));
    assert_int_equal(rsp[0], t);
}

void test_ifc_version_get_null_input(void **state)
{
    (void) state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_interface_version_get(NULL));
}

void test_ifc_version_get_return_oor(void **state)
{
    (void) state;
    uint8_t rsp[] = {0x11, 0x22}; // too short

    transport_expect(OP_IFC_VERSION, 0, 0, 0, rsp, sizeof(rsp));

    ll_version_t v;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_interface_version_get(&v));
}

void test_ifc_version_get_return_ok(void **state)
{
    (void) state;
    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44};

    transport_expect(OP_IFC_VERSION, 0, 0, 0, rsp, sizeof(rsp));

    ll_version_t v;
    assert_int_equal(LL_IFC_ACK, ll_interface_version_get(&v));
    assert_int_equal(0x11, v.major);
    assert_int_equal(0x22, v.minor);
    assert_int_equal(0x3344, v.tag);
}

void test_version_get_null_input(void **state)
{
    (void) state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_version_get(NULL));
}

void test_version_get_return_oor(void **state)
{
    (void) state;
    uint8_t rsp[] = {0x11, 0x22}; // too short

    transport_expect(OP_VERSION, 0, 0, 0, rsp, sizeof(rsp));

    ll_version_t v;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_version_get(&v));
}

void test_version_get_return_ok(void **state)
{
    (void) state;
    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44};

    transport_expect(OP_VERSION, 0, 0, 0, rsp, sizeof(rsp));

    ll_version_t v;
    assert_int_equal(LL_IFC_ACK, ll_version_get(&v));
    assert_int_equal(0x11, v.major);
    assert_int_equal(0x22, v.minor);
    assert_int_equal(0x3344, v.tag);
}

void test_sleep_block_ok(void **state)
{
    (void)state;
    uint8_t cmd[] = {'1'};

    transport_expect(OP_SLEEP_BLOCK, cmd, sizeof(cmd), 0, 0, 0);
    assert_int_equal(LL_IFC_ACK, ll_sleep_block());
}

void test_sleep_unblock_ok(void **state)
{
    (void)state;
    uint8_t cmd[] = {'0'};

    transport_expect(OP_SLEEP_BLOCK, cmd, sizeof(cmd), 0, 0, 0);
    assert_int_equal(LL_IFC_ACK, ll_sleep_unblock());
}

void test_mac_mode_set_invalid_mode(void **state)
{
    (void) state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_mac_mode_set(NUM_MACS));
}

void test_mac_mode_set_ok(void **state)
{
    (void) state;
    uint8_t cmd[] = {0x03};

    transport_expect(OP_MAC_MODE_SET, cmd, sizeof(cmd), 0, 0, 0);
    assert_int_equal(LL_IFC_ACK, ll_mac_mode_set(SYMPHONY_LINK));
}

void test_mac_mode_get_null_input(void **state)
{
    (void) state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_mac_mode_get(NULL));
}

void test_mac_mode_get_ok(void **state)
{
    (void) state;
    uint8_t rsp[] = {0x03};
    ll_mac_type_t t = LORA_NO_MAC;

    transport_expect(OP_MAC_MODE_GET, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ACK, ll_mac_mode_get(&t));

    assert_int_equal(SYMPHONY_LINK, t);
}

void test_antenna_set_invalid_input(void **state)
{
    (void) state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_antenna_set(0));
}

void test_antenna_set_ok(void **state)
{
    (void) state;
    uint8_t cmd[] = {0x02};

    transport_expect(OP_ANTENNA_SET, cmd, sizeof(cmd), 0, 0, 0);
    assert_int_equal(LL_IFC_ACK, ll_antenna_set(2));
}

void test_antenna_get_invalid_input(void **state)
{
    (void) state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_antenna_get(NULL));
}

void test_antenna_get_ok(void **state)
{
    (void) state;
    uint8_t rsp[] = {0x02};
    uint8_t ant = 0;

    transport_expect(OP_ANTENNA_GET, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ACK, ll_antenna_get(&ant));
    assert_int_equal(2, ant);
}

void test_unique_id_get_invalid_input(void **state)
{
    (void) state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_unique_id_get(NULL));
}

void test_unique_id_get_oor(void **state)
{
    (void) state;
    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    uint64_t uid = 0;

    transport_expect(OP_MODULE_ID, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_unique_id_get(&uid));
}

void test_unique_id_get_ok(void **state)
{
    (void) state;
    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint64_t uid = 0;

    transport_expect(OP_MODULE_ID, 0, 0, 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ACK, ll_unique_id_get(&uid));
    assert_int_equal(0x1122334455667788, uid);
}

void test_irq_flags_invalid_input(void **state)
{
    (void) state;
    assert_int_equal(LL_IFC_ERROR_INCORRECT_PARAMETER, ll_irq_flags(0, NULL));
}

void test_irq_flags_oor(void **state)
{
    (void) state;
    uint8_t cmd[] = {0xa5, 0x00, 0x00, 0x5a};
    uint8_t rsp[] = {0x11, 0x22, 0x33};
    uint32_t flags = 0;

    transport_expect(OP_IRQ_FLAGS, cmd, sizeof(cmd), 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH, ll_irq_flags(0xa500005a, &flags));
    assert_int_equal(0, flags);
}

void test_irq_flags_ok(void **state)
{
    (void) state;
    uint8_t cmd[] = {0xa5, 0x00, 0x00, 0x5a};
    uint8_t rsp[] = {0x11, 0x22, 0x33, 0x44};
    uint32_t flags = 0;

    transport_expect(OP_IRQ_FLAGS, cmd, sizeof(cmd), 0, rsp, sizeof(rsp));
    assert_int_equal(LL_IFC_ACK, ll_irq_flags(0xa500005a, &flags));
    assert_int_equal(0x11223344, flags);
}


