#include "ll_ifc_symphony.h"
#include "ll_ifc_private.h"
#include "ifc_struct_defs.h"
#include <string.h> // memmove

uint8_t _uplink_message_buff[256 + 2];
uint8_t *uplink_message_buff = _uplink_message_buff + 2;
uint8_t ll_ul_max_port = 127;

const llabs_dl_band_cfg_t DL_BAN_FCC = {902000000, 928000000, 386703, 3, 0};  // USA / Mexico
const llabs_dl_band_cfg_t DL_BAN_BRA = {916000000, 928000000, 386703, 3, 0};  // Brazil
const llabs_dl_band_cfg_t DL_BAN_AUS = {918000000, 926000000, 386703, 3, 0};  // Australia
const llabs_dl_band_cfg_t DL_BAN_NZL = {921000000, 928000000, 386703, 3, 0};  // New Zealand
const llabs_dl_band_cfg_t DL_BAN_ETSI = {869100000, 871000000, 386703, 1, 0}; // Europe

static int32_t ll_net_token_get(uint32_t *p_net_token)
{
    uint8_t buff[4];
    if (p_net_token == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    int32_t ret = hal_read_write(OP_NET_TOKEN_GET, NULL, 0, buff, 4);
    if (ret < 0)
    {
        return ret;
    }

    if (4 != ret)
    {
        return LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }

    *p_net_token = 0;
    *p_net_token |= (uint32_t) buff[0] << 24;
    *p_net_token |= (uint32_t) buff[1] << 16;
    *p_net_token |= (uint32_t) buff[2] << 8;
    *p_net_token |= (uint32_t) buff[3];
    return LL_IFC_ACK;
}

static int32_t ll_net_token_set(uint32_t net_token)
{
    if (net_token != 0xFFFFFFFF)
    {
        uint8_t buff[4];
        buff[0] = (net_token >> 24) & 0xFF;
        buff[1] = (net_token >> 16) & 0xFF;
        buff[2] = (net_token >> 8) & 0xFF;
        buff[3] = (net_token) &0xFF;
        int32_t ret = hal_read_write(OP_NET_TOKEN_SET, buff, 4, NULL, 0);
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
    int32_t ret = hal_read_write(OP_APP_TOKEN_SET, (uint8_t *) app_token, 10, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_app_token_get(uint8_t *app_token)
{
    if (app_token == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    int32_t ret = hal_read_write(OP_APP_TOKEN_GET, NULL, 0, app_token, 10);
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
    int32_t ret = hal_read_write(OP_RX_MODE_SET, &rx_mode, 1, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_receive_mode_get(uint8_t *rx_mode)
{
    if (rx_mode == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    int32_t ret = hal_read_write(OP_RX_MODE_GET, NULL, 0, rx_mode, sizeof(*rx_mode));
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_qos_request(uint8_t qos)
{
    if (qos > 15)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    int32_t ret = hal_read_write(OP_QOS_REQUEST, &qos, 1, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_qos_get(uint8_t *qos)
{
    if (qos == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    int32_t ret = hal_read_write(OP_QOS_GET, NULL, 0, qos, sizeof(*qos));
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_scan_mode_set(enum ll_scan_mode scan_mode)
{
    uint8_t u8_scan_mode = (uint8_t) scan_mode;
    int32_t ret = hal_read_write(OP_SCAN_MODE_SET, &u8_scan_mode, 1, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_scan_mode_get(enum ll_scan_mode *scan_mode)
{
    uint8_t u8_scan_mode;
    int32_t ret = hal_read_write(OP_SCAN_MODE_GET, NULL, 0, &u8_scan_mode, 1);
    *scan_mode = u8_scan_mode;
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_threshold_set(int16_t threshold)
{
    uint8_t buf[2];
    buf[0] = threshold & 0xff;
    buf[1] = threshold >> 8;

    int32_t ret = hal_read_write(OP_SCAN_THRESHOLD_SET, buf, 2, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_threshold_get(int16_t *threshold)
{
    uint8_t buf[2];
    int32_t ret = hal_read_write(OP_SCAN_THRESHOLD_GET, NULL, 0, buf, 2);
    *threshold = ((uint16_t) buf[1] << 8) | buf[0];
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_scan_attempts_set(uint16_t scan_attempts)
{
    uint8_t buf[2];
    buf[0] = scan_attempts & 0xff;
    buf[1] = scan_attempts >> 8;
    int32_t ret = hal_read_write(OP_SCAN_ATTEMPTS_SET, buf, 2, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_scan_attempts_get(uint16_t *scan_attempts)
{
    uint8_t buf[2];
    int32_t ret = hal_read_write(OP_SCAN_ATTEMPTS_GET, NULL, 0, buf, 2);
    *scan_attempts = ((uint16_t) buf[1] << 8) | buf[0];
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

static int32_t ll_scan_attempts_left_get(uint16_t *scan_attempts)
{
    uint8_t buf[2];
    int32_t ret = hal_read_write(OP_SCAN_ATTEMPTS_LEFT, NULL, 0, buf, 2);
    *scan_attempts = ((uint16_t) buf[1] << 8) | buf[0];
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
    int32_t ret = hal_read_write(OP_CONN_TO_GW_CH, &channel, 1, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_control_messages_enabled_set(bool enable)
{
    int32_t ret = hal_read_write(OP_CTRL_MSG_ENABLED_SET, (uint8_t*)&enable, 1, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_control_messages_enabled_get(bool *enabled)
{
    int32_t ret = hal_read_write(OP_CTRL_MSG_ENABLED_GET, NULL, 0, (uint8_t*)enabled, 1);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_cloud_gpio_activate_pin(uint8_t pin)
{
    int32_t ret = hal_read_write(OP_GPIO_ENABLE_PIN, (uint8_t*)&pin, 1, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_cloud_gpio_deactivate_pin(uint8_t pin)
{
    int32_t ret = hal_read_write(OP_GPIO_DISABLE_PIN, (uint8_t*)&pin, 1, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_cloud_gpio_get_pin_status(uint8_t pin, bool *active)
{
    int32_t ret = hal_read_write(OP_GPIO_PIN_STATUS, (uint8_t*)&pin, 1, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_cloud_gpio_set_pin_high(uint8_t pin)
{
    int32_t ret = hal_read_write(OP_GPIO_SET_HIGH, (uint8_t*)&pin, 1, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_cloud_gpio_set_pin_low(uint8_t pin)
{
    int32_t ret = hal_read_write(OP_GPIO_SET_LOW, (uint8_t*)&pin, 1, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_disconnect()
{
    int32_t ret = hal_read_write(OP_DISCONNECT, NULL, 0, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_poll_scan_result(llabs_gateway_scan_results_t *scan_result, uint8_t *num_gw)
{
    if (scan_result == NULL || num_gw == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    uint8_t buff[GW_SCAN_INFO_BUFF_SIZE];

    int32_t ret = hal_read_write(OP_GET_SCAN_INFO, NULL, 0, buff, GW_SCAN_INFO_BUFF_SIZE);
    if (ret < LL_IFC_ACK)
    {
        return ret;
    }

    ll_gw_scan_result_deserialize(buff, scan_result, num_gw);

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t
ll_get_gateway_scan_results(llabs_gateway_scan_results_t (*scan_results)[MAX_GW_SCAN_RESULTS],
                            uint8_t *num_gw)
{
    uint8_t gw;
    *num_gw = 0;

    do
    {
        llabs_gateway_scan_results_t scan_result;

        int32_t ret = ll_poll_scan_result(&scan_result, &gw);
        if (ret != LL_IFC_ACK)
        {
            return ret;
        }

        // When num_gw is uninitialized, we need to set it with the total
        // amount of gateways
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

    if (NULL != state)
    {
        uint8_t u8_state;
        ret = hal_read_write(OP_STATE, NULL, 0, &u8_state, 1);
        if (LL_IFC_ACK > ret)
        {
            return ret;
        }
        *state = (enum ll_state)(int8_t) u8_state;
    }

    if (NULL != tx_state)
    {
        uint8_t u8_tx_state;
        ret = hal_read_write(OP_TX_STATE, NULL, 0, &u8_tx_state, 1);
        if (LL_IFC_ACK > ret)
        {
            return ret;
        }
        *tx_state = (enum ll_tx_state)(int8_t) u8_tx_state;
    }

    if (NULL != rx_state)
    {
        uint8_t u8_rx_state;
        ret = hal_read_write(OP_RX_STATE, NULL, 0, &u8_rx_state, 1);
        if (LL_IFC_ACK > ret)
        {
            return ret;
        }
        *rx_state = (enum ll_rx_state)(int8_t) u8_rx_state;
    }

    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_mailbox_request(void)
{
    int32_t ret = hal_read_write(OP_MAILBOX_REQUEST, NULL, 0, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_app_reg_get(uint8_t *is_registered)
{
    if (is_registered == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    int32_t ret = hal_read_write(OP_APP_TOKEN_REG_GET, NULL, 0, is_registered, 1);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_encryption_key_exchange_request(void)
{
    int32_t ret = hal_read_write(OP_CRYPTO_KEY_XCHG_REQ, NULL, 0, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_net_info_get(llabs_network_info_t *p_net_info)
{
    uint8_t buff[NET_INFO_BUFF_SIZE];
    if (p_net_info == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    int32_t ret = hal_read_write(OP_NET_INFO_GET, NULL, 0, buff, NET_INFO_BUFF_SIZE);
    if (LL_IFC_ACK > ret)
    {
        return ret;
    }
    if (NET_INFO_BUFF_SIZE != ret)
    {
        return LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }

    ll_net_info_deserialize(buff, p_net_info);
    return LL_IFC_ACK;
}

int32_t ll_stats_get(llabs_stats_t *s)
{
    uint8_t buff[STATS_SIZE];
    if (s == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    int32_t ret = hal_read_write(OP_STATS_GET, NULL, 0, buff, STATS_SIZE);
    if (LL_IFC_ACK > ret)
    {
        return ret;
    }
    if (STATS_SIZE != ret)
    {
        return LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }

    ll_stats_deserialize(buff, s);
    return LL_IFC_ACK;
}

int32_t ll_retrieve_message(uint8_t *buf, uint16_t *size, uint8_t *port, int16_t *rssi,
                            uint8_t *snr)
{
    int32_t rw_response;

    if (buf == NULL || size == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    rw_response = hal_read_write(OP_MSG_RECV, NULL, 0, buf, *size);

    if (rw_response < LL_IFC_ACK)
    {
        *size = 0;
        return rw_response;
    }

    // Size is required
    if ((rw_response & 0xff) <= 4)
    {
        *size = 0;
        return LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }
    *size = (uint8_t)(rw_response & 0xFF) - 4;

    // Optional RSSI
    if (NULL != rssi)
    {
        *rssi = 0;
        *rssi = buf[0] + ((uint16_t) buf[1] << 8);
    }

    // Optional RSSI
    if (NULL != snr)
    {
        *snr = buf[2];
    }

    if (NULL != port)
    {
        *port = buf[3];
    }

    // get rid of snr and rssi in buffer
    memmove(buf, buf + 4, *size);

    return LL_IFC_ACK;
}

int32_t ll_dl_band_cfg_get(llabs_dl_band_cfg_t *p)
{
    uint8_t buff[DL_BAND_CFG_SIZE];
    if (p == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    int32_t ret = hal_read_write(OP_DL_BAND_CFG_GET, NULL, 0, buff, DL_BAND_CFG_SIZE);
    if (LL_IFC_ACK > ret)
    {
        return ret;
    }
    if (DL_BAND_CFG_SIZE != ret)
    {
        return LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }

    ll_dl_band_cfg_deserialize(buff, p);
    return LL_IFC_ACK;
}

int32_t ll_dl_band_cfg_set(const llabs_dl_band_cfg_t *p)
{
    uint8_t buff[DL_BAND_CFG_SIZE];
    if (p == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    ll_dl_band_cfg_serialize(p, buff);
    int32_t ret = hal_read_write(OP_DL_BAND_CFG_SET, buff, DL_BAND_CFG_SIZE, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_rssi_offset_get(int8_t *rssi_offset)
{
    uint8_t raw = 0;
    int32_t ret = hal_read_write(OP_RSSI_OFFSET_GET, NULL, 0, &raw, 1);
    *rssi_offset = (int8_t)raw;
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_rssi_offset_set(int8_t rssi_offset)
{
    if (rssi_offset > 15 || rssi_offset < -15)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }

    int32_t ret = hal_read_write(OP_RSSI_OFFSET_SET, (uint8_t*)&rssi_offset, 1, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_connection_filter_get(uint8_t *p_f)
{
    if (NULL == p_f)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    uint8_t f;
    int32_t ret = hal_read_write(OP_CONN_FILT_GET, NULL, 0, &f, 1);
    if (LL_IFC_ACK > ret)
    {
        return ret;
    }
    if (sizeof(f) != ret)
    {
        return LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }

    *p_f = f;
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_connection_filter_set(uint8_t f)
{
    int32_t ret = hal_read_write(OP_CONN_FILT_SET, &f, 1, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}

int32_t ll_system_time_get(llabs_time_info_t *time_info)
{
    uint8_t buff[TIME_INFO_SIZE];
    if (time_info == NULL)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    int32_t ret = hal_read_write(OP_SYSTEM_TIME_GET, NULL, 0, buff, STATS_SIZE);
    if (LL_IFC_ACK > ret)
    {
        return ret;
    }
    if (TIME_INFO_SIZE != ret)
    {
        return LL_IFC_ERROR_INCORRECT_RESPONSE_LENGTH;
    }

    ll_time_deserialize(buff, time_info);
    return LL_IFC_ACK;
}

int32_t ll_system_time_sync(uint8_t sync_mode)
{
    if (sync_mode > 1)
    {
        return LL_IFC_ERROR_INCORRECT_PARAMETER;
    }
    int32_t ret = hal_read_write(OP_SYSTEM_TIME_SYNC, &sync_mode, sizeof(sync_mode), NULL, 0);
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

    memmove(uplink_message_buff, buf, len);
    _uplink_message_buff[0] = (uint8_t) ack;
    _uplink_message_buff[1] = port;
    int32_t ret = hal_read_write(OP_MSG_SEND, _uplink_message_buff, len + 2, NULL, 0);
    return (ret >= 0) ? LL_IFC_ACK : ret;
}
