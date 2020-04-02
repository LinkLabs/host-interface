#include "ll_ifc_symphony.h"
#include "ll_ifc_private.h"
#include "ifc_struct_defs.h"
#include <string.h> // memmove

//uint8_t _uplink_message_buff[256 + 2];
//uint8_t *uplink_message_buff = _uplink_message_buff + 2;

static uint8_t _ll_ifc_message_buff[256 + 2];
static uint8_t isMsgBuffInUse;

//uint8_t *uplink_message_buff = _uplink_message_buff + 2;

#define UPLINK_DATA_INDEX 2

uint8_t ll_ul_max_port = 127;

const llabs_dl_band_cfg_t DL_BAN_FCC = {902000000, 928000000, 386703, 3, 0};  // USA / Mexico
const llabs_dl_band_cfg_t DL_BAN_BRA = {916000000, 928000000, 386703, 3, 0};  // Brazil
const llabs_dl_band_cfg_t DL_BAN_AUS = {918000000, 926000000, 386703, 3, 0};  // Australia
const llabs_dl_band_cfg_t DL_BAN_NZL = {921000000, 928000000, 386703, 3, 0};  // New Zealand
const llabs_dl_band_cfg_t DL_BAN_ETSI = {869100000, 871000000, 386703, 1, 0}; // Europe


int32_t ll_net_token_get(uint32_t *p_net_token)
{
    if (p_net_token == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_NET_TOKEN_GET, NULL, 0, &_ll_ifc_message_buff[0], 4);
    transport_mutex_release();

    if (ret < 0)
    {
        return ret;
    }

    if (4 != ret)
    {
        return LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }

    *p_net_token = 0;
    *p_net_token |= (uint32_t) _ll_ifc_message_buff[0] << 24;
    *p_net_token |= (uint32_t) _ll_ifc_message_buff[1] << 16;
    *p_net_token |= (uint32_t) _ll_ifc_message_buff[2] << 8;
    *p_net_token |= (uint32_t) _ll_ifc_message_buff[3];
    return LL_IFC_ACK;
}

int32_t ll_net_token_set(uint32_t net_token)
{
    if (net_token != 0xFFFFFFFF)
    {
        if (!(transport_mutex_grab()))
        {
            return LL_IFC_ERROR_HAL_CALL_FAILED;
        }

        _ll_ifc_message_buff[0] = (net_token >> 24) & 0xFF;
        _ll_ifc_message_buff[1] = (net_token >> 16) & 0xFF;
        _ll_ifc_message_buff[2] = (net_token >> 8) & 0xFF;
        _ll_ifc_message_buff[3] = (net_token) &0xFF;


        int32_t ret = hal_read_write(OP_NET_TOKEN_SET, &_ll_ifc_message_buff[0], 4, NULL, 0);
        transport_mutex_release();
        return (ret >= 0) ? LL_IFC_ACK : ret;
    }
    else
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
}

static int32_t ll_app_token_set(const uint8_t *app_token, uint8_t len)
{
    if (app_token == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (10 != len)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    memcpy(&_ll_ifc_message_buff[0], app_token, len);

    int32_t ret = hal_read_write(OP_APP_TOKEN_SET, &_ll_ifc_message_buff[0], 10, NULL, 0);

    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_app_token_get(uint8_t *app_token)
{
    if (app_token == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_APP_TOKEN_GET, NULL, 0, &_ll_ifc_message_buff[0], 10);

    memcpy(app_token, &_ll_ifc_message_buff[0], 10);

    transport_mutex_release();

    if (ret < 0)
    {
        return ret;
    }
    if (10 != ret)
    {
        return LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }

    return LL_IFC_ACK;
}

static int32_t ll_receive_mode_set(uint8_t rx_mode)
{
    if (rx_mode >= NUM_DOWNLINK_MODES)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    _ll_ifc_message_buff[0] = rx_mode;

    int32_t ret = hal_read_write(OP_RX_MODE_SET, &_ll_ifc_message_buff[0], 1, NULL, 0);
    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_receive_mode_get(uint8_t *rx_mode)
{
    if (rx_mode == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_RX_MODE_GET, NULL, 0, &_ll_ifc_message_buff[0], sizeof(*rx_mode));

    *rx_mode = _ll_ifc_message_buff[0];

    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_qos_request(uint8_t qos)
{
    if (qos > 15)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

     _ll_ifc_message_buff[0] = qos;

    int32_t ret = hal_read_write(OP_QOS_REQUEST, &_ll_ifc_message_buff[0], 1, NULL, 0);

    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_qos_get(uint8_t *qos)
{
    if (qos == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_QOS_GET, NULL, 0, &_ll_ifc_message_buff[0], sizeof(*qos));

    *qos = _ll_ifc_message_buff[0];

    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_scan_mode_set(enum ll_scan_mode scan_mode)
{

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    _ll_ifc_message_buff[0] = (uint8_t) scan_mode;

    int32_t ret = hal_read_write(OP_SCAN_MODE_SET, &_ll_ifc_message_buff[0], 1, NULL, 0);
    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_scan_mode_get(enum ll_scan_mode *scan_mode)
{
    uint8_t u8_scan_mode;

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_SCAN_MODE_GET, NULL, 0, &_ll_ifc_message_buff[0], 1);

    *scan_mode = _ll_ifc_message_buff[0];

    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_threshold_set(int16_t threshold)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    _ll_ifc_message_buff[0] = threshold & 0xff;
    _ll_ifc_message_buff[1] = threshold >> 8; 

    int32_t ret = hal_read_write(OP_SCAN_THRESHOLD_SET, &_ll_ifc_message_buff[0], 2, NULL, 0);
    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_threshold_get(int16_t *threshold)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_SCAN_THRESHOLD_GET, NULL, 0, &_ll_ifc_message_buff[0], 2);

    *threshold = ((uint16_t) _ll_ifc_message_buff[1] << 8) | _ll_ifc_message_buff[0];

    transport_mutex_release();
   
    return (ret >= 0) ? LL_IFC_ACK : ret;
}


static int32_t ll_scan_attempts_get(uint16_t *scan_attempts)
{

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_SCAN_ATTEMPTS_GET, NULL, 0, &_ll_ifc_message_buff[0], 2);
    
    *scan_attempts = ((uint16_t) _ll_ifc_message_buff[1] << 8) | _ll_ifc_message_buff[0];

    transport_mutex_release();
   
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_scan_attempts_left_get(uint16_t *scan_attempts)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_SCAN_ATTEMPTS_LEFT, NULL, 0, &_ll_ifc_message_buff[0], 2);
    *scan_attempts = ((uint16_t) _ll_ifc_message_buff[1] << 8) | _ll_ifc_message_buff[0];
    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}


int32_t ll_scan_attempts_set(uint16_t scan_attempts)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    _ll_ifc_message_buff[0] = scan_attempts & 0xff;
    _ll_ifc_message_buff[1] = scan_attempts >> 8;

    int32_t ret = hal_read_write(OP_SCAN_ATTEMPTS_SET, &_ll_ifc_message_buff[0], 2, NULL, 0);
    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_config_set(uint32_t net_token, const uint8_t app_token[APP_TOKEN_LEN],
                      enum ll_downlink_mode dl_mode, uint8_t qos)
{
    int32_t ret;

    ret = ll_net_token_set(net_token);
    if (LL_IFC_ACK != ret)
    {
        return ret;
    }

    ret = ll_app_token_set(app_token, APP_TOKEN_LEN);
    if (LL_IFC_ACK != ret)
    {
        return ret;
    }

    ret = ll_receive_mode_set(dl_mode);
    if (LL_IFC_ACK != ret)
    {
        return ret;
    }

    ret = ll_qos_request(qos);
    if (LL_IFC_ACK != ret)
    {
        return ret;
    }

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_config_get(uint32_t *net_token, uint8_t app_token[APP_TOKEN_LEN],
                      enum ll_downlink_mode *dl_mode, uint8_t *qos)
{
    int32_t ret;

    ret = ll_net_token_get(net_token);
    if (LL_IFC_ACK != ret)
    {
        return ret;
    }

    ret = ll_app_token_get(app_token);
    if (LL_IFC_ACK != ret)
    {
        return ret;
    }

    ret = ll_receive_mode_get((uint8_t *) dl_mode);
    if (LL_IFC_ACK != ret)
    {
        return ret;
    }

    ret = ll_qos_get(qos);
    if (LL_IFC_ACK != ret)
    {
        return ret;
    }

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_scan_config_set(enum ll_scan_mode scan_mode, int16_t threshold, uint16_t scan_attempts)
{
    if (scan_mode > LLABS_INFO_SCAN)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    int32_t ret = ll_scan_mode_set(scan_mode);
    if (ret < LL_IFC_ACK)
    {
        return ret;
    }

    ret = ll_threshold_set(threshold);
    if (ret < LL_IFC_ACK)
    {
        return ret;
    }

    ret = ll_scan_attempts_set(scan_attempts);
    if (ret < LL_IFC_ACK)
    {
        return ret;
    }

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_scan_config_get(enum ll_scan_mode *scan_mode, int16_t *threshold,
                           uint16_t *scan_attempts, uint16_t *scans_left)
{
    int32_t ret = ll_scan_mode_get(scan_mode);
    if (ret < LL_IFC_ACK)
    {
        return ret;
    }

    ret = ll_threshold_get(threshold);
    if (ret < LL_IFC_ACK)
    {
        return ret;
    }

    ret = ll_scan_attempts_get(scan_attempts);
    if (ret < LL_IFC_ACK)
    {
        return ret;
    }

    ret = ll_scan_attempts_left_get(scans_left);
    if (ret < LL_IFC_ACK)
    {
        return ret;
    }

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_connect_to_gw_channel(uint8_t channel)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    _ll_ifc_message_buff[0] = channel;

    int32_t ret = hal_read_write(OP_CONN_TO_GW_CH, &_ll_ifc_message_buff[0], 1, NULL, 0);
    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_control_messages_enabled_set(bool enable)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }
    _ll_ifc_message_buff[0] = enable;
    int32_t ret = hal_read_write(OP_CTRL_MSG_ENABLED_SET, &_ll_ifc_message_buff[0], 1, NULL, 0);
    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_control_messages_enabled_get(bool *enabled)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }
    int32_t ret = hal_read_write(OP_CTRL_MSG_ENABLED_GET, NULL, 0, &_ll_ifc_message_buff[0], 1);
    *enabled = _ll_ifc_message_buff[0];

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_cloud_gpio_activate_pin(uint8_t pin)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }
    _ll_ifc_message_buff[0] = pin;
    int32_t ret = hal_read_write(OP_GPIO_ENABLE_PIN, &_ll_ifc_message_buff[0], 1, NULL, 0);
    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_cloud_gpio_deactivate_pin(uint8_t pin)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }
    _ll_ifc_message_buff[0] = pin;
    int32_t ret = hal_read_write(OP_GPIO_DISABLE_PIN, &_ll_ifc_message_buff[0], 1, NULL, 0);

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_cloud_gpio_get_pin_status(uint8_t pin, bool *active)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }
    _ll_ifc_message_buff[0] = pin;
    int32_t ret = hal_read_write(OP_GPIO_PIN_STATUS, &_ll_ifc_message_buff[0], 1, NULL, 0);

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_cloud_gpio_set_pin_high(uint8_t pin)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }
    _ll_ifc_message_buff[0] = pin;
    int32_t ret = hal_read_write(OP_GPIO_SET_HIGH, &_ll_ifc_message_buff[0], 1, NULL, 0);
    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_cloud_gpio_set_pin_low(uint8_t pin)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }
    _ll_ifc_message_buff[0] = pin;
    int32_t ret = hal_read_write(OP_GPIO_SET_LOW, &_ll_ifc_message_buff[0], 1, NULL, 0);
    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_disconnect()
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }
    int32_t ret = hal_read_write(OP_DISCONNECT, NULL, 0, NULL, 0);
    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_poll_scan_result(llabs_gateway_scan_results_t *scan_result, uint8_t *num_gw)
{
    if (scan_result == NULL || num_gw == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    uint8_t buff[GW_SCAN_INFO_BUFF_SIZE];

    int32_t ret = hal_read_write(OP_GET_SCAN_INFO, NULL, 0, &_ll_ifc_message_buff[0], GW_SCAN_INFO_BUFF_SIZE);

    if (ret < LL_IFC_ACK)
    {
       goto LL_POLL_SCAN_ERROR;
    }

    ll_gw_scan_result_deserialize(&_ll_ifc_message_buff[0], scan_result, num_gw);
    
LL_POLL_SCAN_ERROR:
    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_get_gateway_scan_results(llabs_gateway_scan_results_t (*scan_results)[MAX_GW_SCAN_RESULTS],
                            uint8_t *num_gw)
{
    uint8_t gw=0;
    *num_gw = 0;

    do
    {
        llabs_gateway_scan_results_t scan_result;

        int32_t ret = ll_poll_scan_result(&scan_result, &gw);
        // if scanning will return a NACK_BUSY (6)
        // if done scanning, but no gateways will return NACK_NO_DATA but no paylaod
        // if done scanning, and gateway, will return LL_IFC_ACK and gw will be an index (0-?) 
        //  and decrement each time called until gw is zero.  Zero is a valid gateway index and last
        // one in the list.
        //
        if (ret != LL_IFC_ACK)
        {
            return ret;
        }

        // When num_gw is uninitialized, we need to set it with the total
        // amount of gateways.  The interface decrements num_gw to use as an index prior to 
        // sending so we need to add one for number.
        //
        if (*num_gw == 0)
        {
            *num_gw = gw + 1;
        }

        memcpy(&(*scan_results)[gw], &scan_result, sizeof(scan_result));

    } while (gw > 0);

    return LL_IFC_ACK;
}

int32_t ll_get_state(enum ll_state *state, enum ll_tx_state *tx_state, enum ll_rx_state *rx_state)
{
    int32_t ret = LL_IFC_ACK;

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    if (NULL != state)
    {
        uint8_t u8_state;
        ret = hal_read_write(OP_STATE, NULL, 0, &_ll_ifc_message_buff[0], 1);
        if (LL_IFC_ACK > ret)
        {
            goto GET_STATE_ERROR;
        }
        *state = (enum ll_state)(int8_t)_ll_ifc_message_buff[0];
    }

    if (NULL != tx_state)
    {
        uint8_t u8_tx_state;
        ret = hal_read_write(OP_TX_STATE, NULL, 0, &_ll_ifc_message_buff[0], 1);
        if (LL_IFC_ACK > ret)
        {
            goto GET_STATE_ERROR;
        }
        *tx_state = (enum ll_tx_state)(int8_t)_ll_ifc_message_buff[0];
    }

    if (NULL != rx_state)
    {
        uint8_t u8_rx_state;
        ret = hal_read_write(OP_RX_STATE, NULL, 0, &_ll_ifc_message_buff[0], 1);
        if (LL_IFC_ACK > ret)
        {
            goto GET_STATE_ERROR;
        }
        *rx_state = (enum ll_rx_state)(int8_t)_ll_ifc_message_buff[0];
    }

GET_STATE_ERROR:
    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_mailbox_request(void)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_MAILBOX_REQUEST, NULL, 0, NULL, 0);
    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_app_reg_get(uint8_t *is_registered)
{
    if (is_registered == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_APP_TOKEN_REG_GET, NULL, 0, &_ll_ifc_message_buff[0], 1);
    *is_registered = _ll_ifc_message_buff[0];
    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_encryption_key_exchange_request(void)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_CRYPTO_KEY_XCHG_REQ, NULL, 0, NULL, 0);
    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_net_info_get(llabs_network_info_t *p_net_info)
{
    if (p_net_info == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_NET_INFO_GET, NULL, 0,  &_ll_ifc_message_buff[0], NET_INFO_BUFF_SIZE);
    
    if (LL_IFC_ACK > ret)
    {
         // release mutex and return
    }
    else if (NET_INFO_BUFF_SIZE != ret)
    {
        ret = LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }
    else
    {
        ll_net_info_deserialize(&_ll_ifc_message_buff[0], p_net_info);
        ret = LL_IFC_ACK;
    }

    transport_mutex_release();

    return ret;
}

int32_t ll_stats_get(llabs_stats_t *s)
{
    if (s == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_STATS_GET, NULL, 0, &_ll_ifc_message_buff[0], STATS_SIZE);
    transport_mutex_release();

    if (LL_IFC_ACK > ret)
    {
         //  do nothing and return
    }
    else if (STATS_SIZE != ret)
    {
        ret = LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }
    else
    {
        ll_stats_deserialize(&_ll_ifc_message_buff[0], s);
        ret = LL_IFC_ACK;
    }
    return ret;
}

int32_t ll_retrieve_message(uint8_t *buf, uint16_t *size, uint8_t *port, int16_t *rssi,
                            uint8_t *snr)
{
    if (buf == NULL || size == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_MSG_RECV, NULL, 0, &_ll_ifc_message_buff[0], *size);
    
    if (ret < LL_IFC_ACK)
    {
        *size = 0;
    }
    else if ((ret & 0xff) <= 4)// Size is required
    {
        *size = 0;
        ret = LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }
    else
    {
        *size = (uint8_t)(ret & 0xFF) - 4;

        // Optional RSSI
        if (NULL != rssi)
        {
            *rssi = 0;
            *rssi = _ll_ifc_message_buff[0] + ((uint16_t) _ll_ifc_message_buff[1] << 8);
        }

        // Optional RSSI
        if (NULL != snr)
        {
            *snr = _ll_ifc_message_buff[2];
        }

        if (NULL != port)
        {
            *port = _ll_ifc_message_buff[3];
        }

        // get rid of snr and rssi in buffer
        memmove(buf, _ll_ifc_message_buff + 4, *size);

        ret = LL_IFC_ACK;
    }

    transport_mutex_release();
    return ret;
}

int32_t ll_dl_band_cfg_get(llabs_dl_band_cfg_t *p)
{
    if (p == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_DL_BAND_CFG_GET, NULL, 0, &_ll_ifc_message_buff[0], DL_BAND_CFG_SIZE);
    

    if (LL_IFC_ACK > ret)
    {
    }
    else if (DL_BAND_CFG_SIZE != ret)
    {
        ret = LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }
    else
    {
        ll_dl_band_cfg_deserialize(&_ll_ifc_message_buff[0], p);
        ret = LL_IFC_ACK;
    }

    transport_mutex_release();
    return ret;
}

int32_t ll_dl_band_cfg_set(const llabs_dl_band_cfg_t *p)
{
    if (p == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    ll_dl_band_cfg_serialize(p, &_ll_ifc_message_buff[0]);

    int32_t ret = hal_read_write(OP_DL_BAND_CFG_SET, &_ll_ifc_message_buff[0], DL_BAND_CFG_SIZE, NULL, 0);

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_rssi_offset_get(int8_t *rssi_offset)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_RSSI_OFFSET_GET, NULL, 0, &_ll_ifc_message_buff[0], 1);
    *rssi_offset = (int8_t)_ll_ifc_message_buff[0];

    transport_mutex_release();


    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_rssi_offset_set(int8_t rssi_offset)
{
    if (rssi_offset > 15 || rssi_offset < -15)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }
    _ll_ifc_message_buff[0] = rssi_offset;

    int32_t ret = hal_read_write(OP_RSSI_OFFSET_SET, &_ll_ifc_message_buff[0], 1, NULL, 0);

    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_connection_filter_get(uint8_t *p_f)
{
    if (NULL == p_f)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_CONN_FILT_GET, NULL, 0, &_ll_ifc_message_buff[0], 1);
    

    if (LL_IFC_ACK > ret)
    {
        
    }
    else if (sizeof(uint8_t) != ret)
    {
        ret = LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }
    else
    {
        *p_f = _ll_ifc_message_buff[0];
    }

    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_connection_filter_set(uint8_t f)
{
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }
    
    _ll_ifc_message_buff[0] = f;
    int32_t ret = hal_read_write(OP_CONN_FILT_SET, &_ll_ifc_message_buff[0], 1, NULL, 0);
    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_system_time_get(llabs_time_info_t *time_info)
{
    if (time_info == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    int32_t ret = hal_read_write(OP_SYSTEM_TIME_GET, NULL, 0,  &_ll_ifc_message_buff[0], STATS_SIZE);
    transport_mutex_release();

    if (LL_IFC_ACK > ret)
    {
    }
    else if (TIME_INFO_SIZE != ret)
    {
        ret = LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }
    else
    {
        ll_time_deserialize(_ll_ifc_message_buff, time_info);
        ret = LL_IFC_ACK;
    }
    return ret;
}

int32_t ll_system_time_sync(uint8_t sync_mode)
{
    if (sync_mode > 1)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }
    _ll_ifc_message_buff[0] = sync_mode;

    int32_t ret = hal_read_write(OP_SYSTEM_TIME_SYNC, &_ll_ifc_message_buff[0], sizeof(sync_mode), NULL, 0);
    transport_mutex_release();
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_message_send(uint8_t buf[], uint16_t len, bool ack, uint8_t port)
{
    if (buf == NULL || len <= 0 || len > 256)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (port > ll_ul_max_port)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    if (!(transport_mutex_grab()))
    {
        return LL_IFC_ERROR_HAL_CALL_FAILED;
    }

    memmove(&_ll_ifc_message_buff[2], buf, len);
    _ll_ifc_message_buff[0] = (uint8_t) ack;
    _ll_ifc_message_buff[1] = port;


    int32_t ret = hal_read_write(OP_MSG_SEND, &_ll_ifc_message_buff[0], len + 2, NULL, 0);
    transport_mutex_release();

    return (ret >= 0) ? LL_IFC_ACK : ret;
}
