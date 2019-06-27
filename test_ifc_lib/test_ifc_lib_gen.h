
void setUp(void **state);
void tearDown(void **state);

void test_myself_transport_write(void **state);
void test_timestamp_get(void **state);
void test_timestamp_get_with_response_error(void **state);
void test_timestamp_get_twice_to_check_message_identifier(void **state);
void test_timestamp_set(void **state);
void test_timestamp_set_with_nack(void **state);
void test_nack_with_payload(void **state);
void test_timestamp_set_with_invalid_operation(void **state);
void test_packet_send_timestamp(void **state);
void test_fw_type_get_null_input(void **state);
void test_fw_type_get_return_oor(void **state);
void test_fw_type_get_return_ok(void **state);
void test_hw_type_get_null_input(void **state);
void test_hw_type_get_return_oor(void **state);
void test_hw_type_get_return_ok(void **state);
void test_ifc_version_get_null_input(void **state);
void test_ifc_version_get_return_oor(void **state);
void test_ifc_version_get_return_ok(void **state);
void test_version_get_null_input(void **state);
void test_version_get_return_oor(void **state);
void test_version_get_return_ok(void **state);
void test_sleep_block_ok(void **state);
void test_sleep_unblock_ok(void **state);
void test_mac_mode_set_invalid_mode(void **state);
void test_mac_mode_set_ok(void **state);
void test_mac_mode_get_null_input(void **state);
void test_mac_mode_get_ok(void **state);
void test_antenna_set_invalid_input(void **state);
void test_antenna_set_ok(void **state);
void test_antenna_get_invalid_input(void **state);
void test_antenna_get_ok(void **state);
void test_unique_id_get_invalid_input(void **state);
void test_unique_id_get_oor(void **state);
void test_unique_id_get_ok(void **state);
void test_irq_flags_invalid_input(void **state);
void test_irq_flags_oor(void **state);
void test_irq_flags_ok(void **state);


#define GEN_IFC_LIB_TESTS \
        unit_test_setup_teardown(test_myself_transport_write, setUp, tearDown), \
        unit_test_setup_teardown(test_timestamp_get, setUp, tearDown), \
        unit_test_setup_teardown(test_timestamp_get_with_response_error, setUp, tearDown), \
        unit_test_setup_teardown(test_timestamp_get_twice_to_check_message_identifier, setUp, tearDown), \
        unit_test_setup_teardown(test_timestamp_set, setUp, tearDown), \
        unit_test_setup_teardown(test_timestamp_set_with_nack, setUp, tearDown), \
        unit_test_setup_teardown(test_nack_with_payload, setUp, tearDown), \
        unit_test_setup_teardown(test_timestamp_set_with_invalid_operation, setUp, tearDown), \
        unit_test_setup_teardown(test_packet_send_timestamp, setUp, tearDown), \
        unit_test_setup_teardown(test_fw_type_get_null_input, setUp, tearDown), \
        unit_test_setup_teardown(test_fw_type_get_return_oor, setUp, tearDown), \
        unit_test_setup_teardown(test_fw_type_get_return_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_hw_type_get_null_input, setUp, tearDown), \
        unit_test_setup_teardown(test_hw_type_get_return_oor, setUp, tearDown), \
        unit_test_setup_teardown(test_hw_type_get_return_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_ifc_version_get_null_input, setUp, tearDown), \
        unit_test_setup_teardown(test_ifc_version_get_return_oor, setUp, tearDown), \
        unit_test_setup_teardown(test_ifc_version_get_return_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_version_get_null_input, setUp, tearDown), \
        unit_test_setup_teardown(test_version_get_return_oor, setUp, tearDown), \
        unit_test_setup_teardown(test_version_get_return_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_sleep_block_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_sleep_unblock_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_mac_mode_set_invalid_mode, setUp, tearDown), \
        unit_test_setup_teardown(test_mac_mode_set_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_mac_mode_get_null_input, setUp, tearDown), \
        unit_test_setup_teardown(test_mac_mode_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_antenna_set_invalid_input, setUp, tearDown), \
        unit_test_setup_teardown(test_antenna_set_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_antenna_get_invalid_input, setUp, tearDown), \
        unit_test_setup_teardown(test_antenna_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_unique_id_get_invalid_input, setUp, tearDown), \
        unit_test_setup_teardown(test_unique_id_get_oor, setUp, tearDown), \
        unit_test_setup_teardown(test_unique_id_get_ok, setUp, tearDown), \
        unit_test_setup_teardown(test_irq_flags_invalid_input, setUp, tearDown), \
        unit_test_setup_teardown(test_irq_flags_oor, setUp, tearDown), \
        unit_test_setup_teardown(test_irq_flags_ok, setUp, tearDown), \


