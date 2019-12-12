#include <stdio.h>
#include <string.h>
#include <time.h>

#include "ifc_struct_defs.h"
#include "ll_ifc.h"
#include "ll_ifc_consts.h"
#include "ll_ifc_symphony.h"
#include "ll_ifc_no_mac.h"

#ifndef NULL    // <time.h> defines NULL on *some* platforms
#define NULL                (0)
#endif
#define CMD_HEADER_LEN      (5)
#define RESP_HEADER_LEN     (6)

#ifdef _PLATFORM_LINUX
    #define WAKEUP_BYTE     (0xFF)
#else
    #define WAKEUP_BYTE     (0x07)
#endif

#define LL_IFC_MSG_BUFF_SIZE 258
#define LL_IFC_MSG_BUFF_IN_SIZE 126

static uint8_t _ll_ifc_msg_buff[LL_IFC_MSG_BUFF_SIZE];
static uint8_t _ll_ifc_msg_in_buff[LL_IFC_MSG_BUFF_IN_SIZE];

static uint16_t compute_checksum(uint8_t *hdr, uint16_t hdr_len, uint8_t *payload, uint16_t payload_len);
static void send_packet(opcode_t op, uint8_t message_num, uint8_t *buf, uint16_t len);
static int32_t recv_packet(opcode_t op, uint8_t message_num, uint8_t *buf, uint16_t len);
static int32_t recv_packet2(opcode_t op, uint8_t message_num, uint8_t *buf, uint16_t len);

const uint32_t OPEN_NET_TOKEN = 0x4f50454e;

static int32_t message_num = 0;

int32_t hal_read_write(opcode_t op, uint8_t buf_in[], uint16_t in_len, uint8_t buf_out[], uint16_t out_len)
{
    int32_t ret;

    // Error checking:
    // Only valid combinations of buffer & length pairs are:
    // buf == NULL, len = 0
    // buf != NULL, len > 0
    if (((buf_in  != NULL) && ( in_len == 0)) || (( buf_in == NULL) && ( in_len > 0)))
    {
        return(LL_IFC_ERROR_INCORRECT_PARAMETER);
    }
    if (((buf_out != NULL) && (out_len == 0)) || ((buf_out == NULL) && (out_len > 0)))
    {
        return(LL_IFC_ERROR_INCORRECT_PARAMETER);
    }

    // OK, inputs have been sanitized. Carry on...
    send_packet(op, message_num, buf_in, in_len);

    ret = recv_packet2(op, message_num, buf_out, out_len);

    message_num++;

    return(ret);
}

int32_t hal_read_write_exact(opcode_t op, uint8_t buf_in[], uint16_t in_len, uint8_t buf_out[], uint16_t out_len)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    } 

    int32_t ret = hal_read_write(op, buf_in, in_len, buf_out, out_len);
    if (ret >= 0)
    {
        if (ret != out_len)
        {
            ret = LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
        }
        else
        {
            ret = 0;
        }
    }

    transport_mutex_release();

    return ret;
}

char const * ll_return_code_name(int32_t return_code)
{
    switch (return_code)
    {
        case -LL_IFC_ACK:                            return "ACK";
        case -LL_IFC_NACK_CMD_NOT_SUPPORTED:         return "CMD_NOT_SUPPORTED";
        case -LL_IFC_NACK_INCORRECT_CHKSUM:          return "INCORRECT_CHKSUM ";
        case -LL_IFC_NACK_PAYLOAD_LEN_OOR:           return "PAYLOAD_LEN_OOR";
        case -LL_IFC_NACK_PAYLOAD_OOR:               return "PAYLOAD_OOR";
        case -LL_IFC_NACK_BOOTUP_IN_PROGRESS:        return "BOOTUP_IN_PROGRESS";
        case -LL_IFC_NACK_BUSY_TRY_AGAIN:            return "BUSY_TRY_AGAIN";
        case -LL_IFC_NACK_APP_TOKEN_REG:             return "APP_TOKEN_REG";
        case -LL_IFC_NACK_PAYLOAD_LEN_EXCEEDED:      return "PAYLOAD_LEN_EXCEEDED";
        case -LL_IFC_NACK_NOT_IN_MAILBOX_MODE:       return "NOT IN MAILBOX MODE";
        case -LL_IFC_NACK_PAYLOAD_BAD_PROPERTY:      return "BAD PROPERTY ID";
        case -LL_IFC_NACK_NODATA:                    return "NO DATA AVAIL";
        case -LL_IFC_NACK_QUEUE_FULL:                return "QUEUE FULL";
        case -LL_IFC_NACK_OTHER:                     return "OTHER";

        case LL_IFC_ERROR_INCORRECT_PARAMETER:       return "INCORRECT_PARAMETER";
        case LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH: return "INCORRECT_RESPONSE_LENGTH";
        case LL_IFC_ERROR_MESSAGE_NUMBER_MISMATCH:   return "MESSAGE_NUMBER_MISMATCH";
        case LL_IFC_ERROR_CHECKSUM_MISMATCH:         return "CHECKSUM_MISMATCH";
        case LL_IFC_ERROR_COMMAND_MISMATCH:          return "COMMAND_MISMATCH";
        case LL_IFC_ERROR_HOST_INTERFACE_TIMEOUT:    return "HOST_INTERFACE_TIMEOUT";
        case LL_IFC_ERROR_BUFFER_TOO_SMALL:          return "BUFFER_TOO_SMALL";
        case LL_IFC_ERROR_START_OF_FRAME:            return "START_OF_FRAME";
        case LL_IFC_ERROR_HEADER:                    return "HEADER";
        case LL_IFC_ERROR_TIMEOUT:                   return "TIMEOUT";
        case LL_IFC_ERROR_INCORRECT_MESSAGE_SIZE:    return "INCORRECT_MESSAGE_SIZE";
        case LL_IFC_ERROR_NO_NETWORK:                return "NO_NETWORK";

        default:                                     return "UNKNOWN";
    }
}

char const * ll_return_code_description(int32_t return_code)
{
    switch (return_code)
    {
        case -LL_IFC_ACK:                            return "Success";
        case -LL_IFC_NACK_CMD_NOT_SUPPORTED:         return "Command not supported";
        case -LL_IFC_NACK_INCORRECT_CHKSUM:          return "Incorrect Checksum";
        case -LL_IFC_NACK_PAYLOAD_LEN_OOR:           return "Length of payload sent in command was out of range";
        case -LL_IFC_NACK_PAYLOAD_OOR:               return "Payload sent in command was out of range.";
        case -LL_IFC_NACK_BOOTUP_IN_PROGRESS:        return "Not allowed since firmware bootup still in progress. Wait.";
        case -LL_IFC_NACK_BUSY_TRY_AGAIN:            return "Operation prevented by temporary event. Retry later.";
        case -LL_IFC_NACK_APP_TOKEN_REG:             return "Application token is not registered for this node.";
        case -LL_IFC_NACK_PAYLOAD_LEN_EXCEEDED:      return "Payload length is greater than the max supported length";
        case -LL_IFC_NACK_NOT_IN_MAILBOX_MODE:       return "Command invalid, not in mailbox mode";
        case -LL_IFC_NACK_PAYLOAD_BAD_PROPERTY:      return "Bad property ID specified";
        case -LL_IFC_NACK_NODATA:                    return "No msg data available to return";
        case -LL_IFC_NACK_QUEUE_FULL:                return "Data cannot be enqueued for transmission, queue is full";
        case -LL_IFC_NACK_OTHER:                     return "Unspecified error";

        case LL_IFC_ERROR_INCORRECT_PARAMETER:       return "The parameter value was invalid";
        case LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH: return "Module response was not the expected size";
        case LL_IFC_ERROR_MESSAGE_NUMBER_MISMATCH:   return "Message number in response doesn't match expected";
        case LL_IFC_ERROR_CHECKSUM_MISMATCH:         return "Checksum mismatch";
        case LL_IFC_ERROR_COMMAND_MISMATCH:          return "Command mismatch (responding to a different command)";
        case LL_IFC_ERROR_HOST_INTERFACE_TIMEOUT:    return "Timed out waiting for Rx bytes from interface";
        case LL_IFC_ERROR_BUFFER_TOO_SMALL:          return "Response larger than provided output buffer";
        case LL_IFC_ERROR_START_OF_FRAME:            return "transport_read failed getting FRAME_START";
        case LL_IFC_ERROR_HEADER:                    return "transport_read failed getting header";
        case LL_IFC_ERROR_TIMEOUT:                   return "The operation timed out";
        case LL_IFC_ERROR_INCORRECT_MESSAGE_SIZE:    return "The message size from the device was incorrect";
        case LL_IFC_ERROR_NO_NETWORK:                return "No network was available";

        default:                                     return "unknown error";
    }
}

int32_t ll_firmware_type_get(ll_firmware_type_t *t)
{

    int32_t ret;
    if(NULL == t)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    ret = hal_read_write(OP_FIRMWARE_TYPE, NULL, 0, &_ll_ifc_msg_buff[0], FIRMWARE_TYPE_LEN);
    transport_mutex_release();

    if (ret == 0)
    {
        //return ret;
    }
    else if(FIRMWARE_TYPE_LEN != ret)
    {
        ret = LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }
    else
    {
        t->cpu_code = _ll_ifc_msg_buff[0] << 8 | _ll_ifc_msg_buff[1];
        t->functionality_code = _ll_ifc_msg_buff[2] << 8 | _ll_ifc_msg_buff[3];
    }

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_hardware_type_get(ll_hardware_type_t *t)
{

    int32_t ret;
    if(NULL == t)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    ret = hal_read_write(OP_HARDWARE_TYPE, NULL, 0, &_ll_ifc_msg_buff[0], sizeof(uint8_t));

    if(ret < 0)
    {
        //return ret;
    }
    else if(sizeof(uint8_t) != ret)
    {
        ret = LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }
    else
    {
        *t = (ll_hardware_type_t) _ll_ifc_msg_buff[0];
    }
    
    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

const char * ll_hardware_type_string(ll_hardware_type_t t)
{
    switch(t)
    {
        case UNAVAILABLE: return "unavailable";
        case LLRLP20_V2:  return "LLRLP20 v2";
        case LLRXR26_V2:  return "LLRXR26 v2";
        case LLRLP20_V3:  return "LLRLP20 v3";
        case LLRXR26_V3:  return "LLRXR26 v3";
        default:          return "unknown";
    }
}

int32_t ll_interface_version_get(ll_version_t *version)
{
    int32_t ret;
    if(NULL == version)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    ret = hal_read_write(OP_IFC_VERSION, NULL, 0, &_ll_ifc_msg_buff[0], VERSION_LEN);
    
    if(ret < 0)
    {
        //return ret;
    }
    else if (VERSION_LEN != ret)
    {
        ret = LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }
    else
    {
        version->major = _ll_ifc_msg_buff[0];
        version->minor = _ll_ifc_msg_buff[1];
        version->tag = _ll_ifc_msg_buff[2] << 8 | _ll_ifc_msg_buff[3];
    }
    
    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_version_get(ll_version_t *version)
{
    uint8_t buf[VERSION_LEN];
    int32_t ret;
    if(NULL == version)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    ret = hal_read_write(OP_VERSION, NULL, 0, &_ll_ifc_msg_buff[0], VERSION_LEN);

    if(ret < 0)
    {
        //return ret;
    }
    else if (VERSION_LEN != ret)
    {
        ret = LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }
    else 
    {
        version->major = _ll_ifc_msg_buff[0];
        version->minor = _ll_ifc_msg_buff[1];
        version->tag = _ll_ifc_msg_buff[2] << 8 | _ll_ifc_msg_buff[3];
    }
 
    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_sleep_block(void)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }
    _ll_ifc_msg_buff[0] = '1';
    _ll_ifc_msg_buff[1] = 0;
    int32_t ret = hal_read_write(OP_SLEEP_BLOCK, &_ll_ifc_msg_buff[0], 1, NULL, 0);

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_sleep_unblock(void)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    _ll_ifc_msg_buff[0] = '0';
    _ll_ifc_msg_buff[1] = 0;
    int32_t ret = hal_read_write(OP_SLEEP_BLOCK, &_ll_ifc_msg_buff[0], 1, NULL, 0);

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_mac_mode_set(ll_mac_type_t mac_mode)
{
    if (mac_mode >= NUM_MACS)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    _ll_ifc_msg_buff[0] = (uint8_t)mac_mode;
    int32_t ret = hal_read_write(OP_MAC_MODE_SET, &_ll_ifc_msg_buff[0], 1, NULL, 0);

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_mac_mode_get(ll_mac_type_t *mac_mode)
{
    int32_t ret;
    if (NULL == mac_mode)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }


    ret = hal_read_write(OP_MAC_MODE_GET, NULL, 0, &_ll_ifc_msg_buff[0], sizeof(uint8_t));
    *mac_mode = _ll_ifc_msg_buff[0];

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_antenna_set(uint8_t ant)
{
    if((ant == 1) || (ant == 2))
    {
        if (!(transport_mutex_grab()))
        {
            return LL_IFC_ERROR_HAL_CALL_FAILED;
        }

        _ll_ifc_msg_buff[0] = ant;

        int32_t ret = hal_read_write(OP_ANTENNA_SET, &_ll_ifc_msg_buff[0], 1, NULL, 0);

        transport_mutex_release();

        return (ret >= 0) ? LL_IFC_ACK : ret;
    }
    else
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
}

int32_t ll_antenna_get(uint8_t *ant)
{
    if (ant == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_ANTENNA_GET, NULL, 0, &_ll_ifc_msg_buff[0], 1);
    *ant = _ll_ifc_msg_buff[0];

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_unique_id_get(uint64_t *unique_id)
{
    int32_t ret;
    uint8_t i;

    if (unique_id == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    ret = hal_read_write(OP_MODULE_ID, NULL, 0,  &_ll_ifc_msg_buff[0], UNIQUE_ID_LEN);

    if(ret < 0)
    {
        //return ret;
    }
    else if (UNIQUE_ID_LEN != ret)
    {
        ret = LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }
    else
    {
        *unique_id = 0;
        for (i = 0; i < UNIQUE_ID_LEN; i++)
        {
            *unique_id |= ((uint64_t) _ll_ifc_msg_buff[i]) << (8 * (7 - i));
        }
    }
    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_settings_store(void)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_STORE_SETTINGS, NULL, 0, NULL, 0);

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_settings_delete(void)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_DELETE_SETTINGS, NULL, 0, NULL, 0);

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_restore_defaults(void)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_RESET_SETTINGS, NULL, 0, NULL, 0);

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_sleep(void)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_SLEEP, NULL, 0, NULL, 0);

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_reset_mcu(void)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_RESET_MCU, NULL, 0, NULL, 0);

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_bootloader_mode(void)
{
    send_packet(OP_TRIGGER_BOOTLOADER, message_num, NULL, 0);
    return LL_IFC_ACK;
}

/**
 * @return
 *   0 - success
 *   negative = error, as defined by hal_read_write
 */
int32_t ll_irq_flags(uint32_t flags_to_clear, uint32_t *flags)
{
    if(NULL == flags)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    // Assuming big endian convention over the interface
    _ll_ifc_msg_buff[0] = (uint8_t)((flags_to_clear >> 24) & 0xFF);
    _ll_ifc_msg_buff[1] = (uint8_t)((flags_to_clear >> 16) & 0xFF);
    _ll_ifc_msg_buff[2] = (uint8_t)((flags_to_clear >>  8) & 0xFF);
    _ll_ifc_msg_buff[3] = (uint8_t)((flags_to_clear      ) & 0xFF);

    int32_t ret = hal_read_write(OP_IRQ_FLAGS, &_ll_ifc_msg_buff[0], 4, &_ll_ifc_msg_in_buff[0], 4);

    if(ret < 0)
    {
        //return ret;
    }
    else if(4 != ret)
    {
        ret = LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }
    else 
    {
        uint32_t flags_temp = 0;
        flags_temp |= (((uint32_t)_ll_ifc_msg_in_buff[0]) << 24);
        flags_temp |= (((uint32_t)_ll_ifc_msg_in_buff[1]) << 16);
        flags_temp |= (((uint32_t)_ll_ifc_msg_in_buff[2]) << 8);
        flags_temp |= (((uint32_t)_ll_ifc_msg_in_buff[3]));
        *flags = flags_temp;
    }
    
    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

//STRIPTHIS!START
int32_t ll_timestamp_get(uint32_t * timestamp_us)
{
    return ll_timestamp_set(LL_TIMESTAMP_NO_OPERATION, 0, timestamp_us);
}

int32_t ll_timestamp_set(ll_timestamp_operation_t operation, uint32_t timestamp_us, uint32_t * actual_timestamp_us)
{
    uint8_t * b_out = &_ll_ifc_msg_buff[0];

    if (actual_timestamp_us == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    write_uint8((uint8_t) operation, &b_out);
    write_uint32(timestamp_us, &b_out);

    int32_t ret = hal_read_write_exact(OP_TIMESTAMP, &_ll_ifc_msg_buff[0], 5, &_ll_ifc_msg_in_buff[0], 4);

    if (ret >= 0)
    {
        uint8_t const * b_in = &_ll_ifc_msg_in_buff[0];
        *actual_timestamp_us = read_uint32(&b_in);
        ret = LL_IFC_ACK;
    }

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_trigger_watchdog(void)
{
    uint8_t cmd_buf[2];

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    _ll_ifc_msg_buff[0] = 0x00;
    _ll_ifc_msg_buff[1] = 0x01;

    int32_t ret = hal_read_write(OP_RESERVED1, &_ll_ifc_msg_buff[0], 2, NULL, 0);

    transport_mutex_release();

    return ret;
}

int32_t ll_get_assert_info(char *filename, uint16_t filename_len, uint32_t *line)
{

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_GET_ASSERT, NULL, 0, &_ll_ifc_msg_in_buff[0], 4 + 20);

    if (ret > 0)
    {
        *line = 0;
        *line += (uint32_t)(_ll_ifc_msg_in_buff[0]) << 24;
        *line += (uint32_t)(_ll_ifc_msg_in_buff[1]) << 16;
        *line += (uint32_t)(_ll_ifc_msg_in_buff[2]) <<  8;
        *line += (uint32_t)(_ll_ifc_msg_in_buff[3]) <<  0;

        uint16_t cpy_len = filename_len < 20 ? filename_len : 20;
        memcpy(filename, &_ll_ifc_msg_in_buff[4], cpy_len);
    }

    transport_mutex_release();
    return ret;
}

int32_t ll_trigger_assert(void)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret =  hal_read_write(OP_SET_ASSERT, NULL, 0, NULL, 0);
    transport_mutex_release();
    return ret;
}

//STRIPTHIS!STOP

int32_t ll_reset_state( void )
{
    message_num = 0;
    return 0;
}


/**
 * @brief
 *  send_packet
 *
 * @param[in] op
 *   opcode of the command being sent to the module
 *
 * @param[in] message_num
 *   message_num
 *
 * @param[in] buf
 *   byte array containing the data payload to be sent to the module
 *
 * @param[in] len
 *   size of the output buffer in bytes
 *
 * @return
 *   none
 */
#define SP_NUM_WAKEUP_BYTES (5)
#define SP_NUM_ZEROS (3)
#define SP_NUM_CKSUM_BYTES (2)
#define SP_HEADER_SIZE (CMD_HEADER_LEN + SP_NUM_ZEROS)
uint8_t wakeup_buf[SP_NUM_WAKEUP_BYTES];
uint8_t header_buf[(SP_HEADER_SIZE>RESP_HEADER_LEN)?SP_HEADER_SIZE:RESP_HEADER_LEN];
uint8_t checksum_buff[SP_NUM_CKSUM_BYTES];

static void send_packet(opcode_t op, uint8_t message_num, uint8_t *buf, uint16_t len)
{
    uint16_t computed_checksum;
    uint16_t header_idx = 0;
    uint16_t i;

    // Send the wakeup bytes
    memset((uint8_t *) wakeup_buf, WAKEUP_BYTE, (size_t) SP_NUM_WAKEUP_BYTES);

    transport_write(wakeup_buf, SP_NUM_WAKEUP_BYTES);

#ifndef _PLATFORM_LINUX
    for (i = 0; i < 2000; i++)
    {
        asm("nop");
    }
#endif
    
    memset((uint8_t *) header_buf, 0xFF, (size_t) SP_NUM_ZEROS);
 
    header_idx = SP_NUM_ZEROS;

    header_buf[header_idx++] = FRAME_START;
    header_buf[header_idx++] = op;
    header_buf[header_idx++] = message_num;
    header_buf[header_idx++] = (uint8_t)(0xFF & (len >> 8));
    header_buf[header_idx++] = (uint8_t)(0xFF & (len >> 0));

    computed_checksum = compute_checksum(&header_buf[SP_NUM_ZEROS], CMD_HEADER_LEN, buf, len);

    transport_write(header_buf, SP_HEADER_SIZE);

    if (buf != NULL)
    {
        transport_write(buf, len);
    }

    checksum_buff[0] = (computed_checksum >> 8);
    checksum_buff[1] = (computed_checksum >> 0);
    transport_write(checksum_buff, 2);
}

/**
 * @brief
 *   recv_packet
 *
 * @param[in] op
 *   opcode of the command that we're trying to receive
 *
 * @param[in] message_num
 *   message number of the command that we're trying to receive
 *
 * @param[in] buf
 *   byte array for storing data returned from the module
 *
 * @param[in] len
 *   size of the output buffer in bytes
 *
 * @return
 *   positive number of bytes returned,
 *   negative if an error
 *   Error Codes:
 *       -1 NACK received - Command not supported
 *       -2 NACK received - Incorrect Checksum
 *       -3 NACK received - Payload length out of range
 *       -4 NACK received - Payload out of range
 *       -5 NACK received - Not allowed, bootup in progress
 *       -6 NACK received - Busy try again
 *       -7 NACK received - Application token not registered
 *       -8 NACK received - Payload length greater than maximum supported length
 *      -99 NACK received - Other
 *     -103 Message Number in response doesn't match expected
 *     -104 Checksum mismatch
 *     -105 Command mismatch (responding to a different command)
 *     -106 Timed out waiting for Rx bytes from interface
 *     -107 Response larger than provided output buffer
 *     -108 transport_read failed getting FRAME_START
 *     -109 transport_read failed getting header
 */
static uint8_t _ll_ifc_start_byte = 0;
static uint8_t _ll_ifc_temp_byte;

static int32_t recv_packet(opcode_t op, uint8_t message_num, uint8_t *buf, uint16_t len)
{

    uint16_t header_idx;
    uint16_t computed_checksum;
    int32_t ret_value = 0;
    int32_t ret;

    memset(header_buf, 0, sizeof(header_buf));

    struct time time_start, time_now;

    if (gettime(&time_start) < 0)
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    do
    {

        /* Timeout of infinite Rx loop if responses never show up*/
        ret = transport_read(&_ll_ifc_start_byte, 1);

        if (gettime(&time_now) < 0)
        {
            return LL_IFC_ERROR_HAL_CALL_FAILED;
       
        }

        if(time_now.tv_sec - time_start.tv_sec > 2)
        {
            len = 0;
            return LL_IFC_ERROR_HOST_INTERFACE_TIMEOUT;
        }
    } while(_ll_ifc_start_byte != FRAME_START);

    if (ret < 0)
    {
        /* transport_read failed - return an error */
        return LL_IFC_ERROR_START_OF_FRAME;
    }

    header_idx = 0;
    header_buf[header_idx++] = FRAME_START;

    ret = transport_read(&header_buf[header_idx], RESP_HEADER_LEN - 1);
    if (ret < 0)
    {
        /* transport_read failed - return an error */
        return LL_IFC_ERROR_HEADER;
    }

    uint16_t len_from_header = (uint16_t)header_buf[5] + ((uint16_t)header_buf[4] << 8);

    if (header_buf[1] != op)
    {
        // Command Byte should match what was sent
        ret_value = LL_IFC_ERROR_COMMAND_MISMATCH;
    }
    if (header_buf[2] != message_num)
    {
        // Message Number should match
        ret_value = LL_IFC_ERROR_MESSAGE_NUMBER_MISMATCH;
    }
    if (header_buf[3] != 0x00)
    {
        // NACK Received
        // Map NACK code to error code
        ret_value = 0 - header_buf[3];
    }
    if (len_from_header > len)
    {
        // response is larger than the caller expects.
        // Pull the bytes out of the Rx fifo
        int32_t ret;
        do
        {

            ret = transport_read(&_ll_ifc_temp_byte, 1);
        }
        while (ret == 0);

        return LL_IFC_ERROR_BUFFER_TOO_SMALL;
    }
    else if (len_from_header < len)
    {
        // response is shorter than caller expects.
        len = len_from_header;
    }

    if (ret_value == 0)
    {

        // If we got here, then we:
        // 1) Received the FRAME_START in the response
        // 2) The message number matched
        // 3) The ACK byte was ACK (not NACK)
        // 4) The received payload length is less than or equal to the size of the buffer
        //      allocated for the payload

        // Grab the payload if there is supposed to be one
        if ((buf != NULL) && (len > 0))
        {
             ret = transport_read(buf, len);
             if (ret < 0)
              {
                 return -2;
              }
        }
    }

    // Finally, make sure the checksum matches
    ret = transport_read(checksum_buff, 2);
    if (ret < 0)
    {
       return -2;
    }


    computed_checksum = compute_checksum(header_buf, RESP_HEADER_LEN, buf, len);
    uint16_t rx_checksum = ((uint16_t)checksum_buff[0] << 8) + checksum_buff[1];
    if (rx_checksum != computed_checksum)
    {
        return LL_IFC_ERROR_CHECKSUM_MISMATCH;
    }

    if (ret_value == 0)
    {
        // Success! Return the number of bytes in the payload (0 or positive number)
        return len;
    }
    else
    {
        // Failure! Return an error, such as NACK response from the firmware
        return ret_value;
    }
}



static int32_t recv_packet2(opcode_t op, uint8_t message_num, uint8_t *buf, uint16_t len)
{
    uint8_t  header_buf[RESP_HEADER_LEN];
    uint16_t header_idx;



    uint16_t computed_checksum;
    int32_t ret_value = 0;
    int32_t ret;

    memset(header_buf, 0, sizeof(header_buf));

    struct time time_start, time_now;

    if (gettime(&time_start) < 0)
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    do
    {

        /* Timeout of infinite Rx loop if responses never show up*/
        ret = transport_read2(&_ll_ifc_start_byte, 1);

        if (gettime(&time_now) < 0)
        {
            return LL_IFC_ERROR_HAL_CALL_FAILED;
       
        }

        if(time_now.tv_sec - time_start.tv_sec > 2)
        {
            len = 0;
            return LL_IFC_ERROR_HOST_INTERFACE_TIMEOUT;
        }
    } while(_ll_ifc_start_byte != FRAME_START);

    if (ret < 0)
    {
        /* transport_read failed - return an error */
        return LL_IFC_ERROR_START_OF_FRAME;
    }

    header_idx = 0;
    header_buf[header_idx++] = FRAME_START;

    ret = transport_read2(header_buf + 1, RESP_HEADER_LEN - 1);
    if (ret < 0)
    {
        /* transport_read failed - return an error */
        return LL_IFC_ERROR_HEADER;
    }

    uint16_t len_from_header = (uint16_t)header_buf[5] + ((uint16_t)header_buf[4] << 8);

    if (header_buf[1] != op)
    {
        // Command Byte should match what was sent
        ret_value = LL_IFC_ERROR_COMMAND_MISMATCH;
    }
    if (header_buf[2] != message_num)
    {
        // Message Number should match
        ret_value = LL_IFC_ERROR_MESSAGE_NUMBER_MISMATCH;
    }
    if (header_buf[3] != 0x00)
    {
        // NACK Received
        // Map NACK code to error code
        ret_value = 0 - header_buf[3];
    }
    if (len_from_header > len)
    {
        // response is larger than the caller expects.
        // Pull the bytes out of the Rx fifo
        int32_t ret;
        do
        {

            ret = transport_read2(&_ll_ifc_temp_byte, 1);
        }
        while (ret == 0);

        return LL_IFC_ERROR_BUFFER_TOO_SMALL;
    }
    else if (len_from_header < len)
    {
        // response is shorter than caller expects.
        len = len_from_header;
    }

    if (ret_value == 0)
    {

        // If we got here, then we:
        // 1) Received the FRAME_START in the response
        // 2) The message number matched
        // 3) The ACK byte was ACK (not NACK)
        // 4) The received payload length is less than or equal to the size of the buffer
        //      allocated for the payload

        // Grab the payload if there is supposed to be one
        if ((buf != NULL) && (len > 0))
        {
             ret = transport_read2(buf, len);
             if (ret < 0)
              {
                 return -2;
              }
        }
    }

    // Finally, make sure the checksum matches
    ret = transport_read2(checksum_buff, 2);
    if (ret < 0)
    {
       return -2;
    }


    computed_checksum = compute_checksum(header_buf, RESP_HEADER_LEN, buf, len);
    uint16_t rx_checksum = ((uint16_t)checksum_buff[0] << 8) + checksum_buff[1];
    if (rx_checksum != computed_checksum)
    {
        return LL_IFC_ERROR_CHECKSUM_MISMATCH;
    }

    if (ret_value == 0)
    {
        // Success! Return the number of bytes in the payload (0 or positive number)
        return len;
    }
    else
    {
        // Failure! Return an error, such as NACK response from the firmware
        return ret_value;
    }
}


/**
 * @brief
 *   compute_checksum
 *
 * @param[in] hdr
 *   header array to compute checksum on
 *
 * @param[in] hdr_len
 *   size of the header array in bytes
 *
 * @param[in] payload
 *   payload array to compute checksum on
 *
 * @param[in] payload_len
 *   size of the payload array in bytes
 *
 * @return
 *   The 8-bit checksum
 */
static uint16_t compute_checksum(uint8_t *hdr, uint16_t hdr_len, uint8_t *payload, uint16_t payload_len)
{
    uint16_t crc = 0x0;
    uint16_t i;

    for (i = 0; i < hdr_len; i++)
    {
        crc  = (crc >> 8) | (crc << 8);
        crc ^= hdr[i];
        crc ^= (crc & 0xff) >> 4;
        crc ^= crc << 12;
        crc ^= (crc & 0xff) << 5;
    }

    for (i = 0; i < payload_len; i++)
    {
        crc  = (crc >> 8) | (crc << 8);
        crc ^= payload[i];
        crc ^= (crc & 0xff) >> 4;
        crc ^= crc << 12;
        crc ^= (crc & 0xff) << 5;
    }

    return crc;
}
