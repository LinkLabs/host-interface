#ifndef __LL_IFC_SYMPHONY_H
#define __LL_IFC_SYMPHONY_H

#include <stdint.h>
#include <stdbool.h>
#include "ll_ifc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup Link_Labs_Interface_Library
 * @{
 */

/**
 * @defgroup Symphony_Interface Symphony Link
 *
 * @brief Communicate with a Symphony Link network.
 *
 * Symphony Link mode allows the external host to communicate with Symphony
 * Link networks, a dynamic IEEE 802.15.4 based LoRa wireless system built
 * specifically for low power, wide-area connectivity.
 *
 * @{
 */

/**
 * @brief These are the different scanning modes for Link Labs Symphony
 * Modules.
 *
 * A Quick Scan will attempt to connect to the first gateway or repeater it sees. The
 * intent behind this scan is to keep the scanning process as quick and
 * simple as possible resulting in less power consumption of the scanning
 * features.
 *
 * A Normal Scan is the default scan that Link Labs Symphony Modules have
 * always used. This scan looks for a balance between power consumption,
 * results and performance. The normal scan will look at every gateway within
 * it's range and assess which gateway has the best connection credentials.
 * Once all of the channels have been scanned, it will connect to the gateway or
 * repeater with the best RSSI and SNR.
 *
 * An Info Scan can be use to gather a list of scan results. This scan mode
 * gathers much more data about each gateway and repeater on the air; then
 * saves all of that information into an array of scan_results that the user
 * can use. The intent behind this scan was to provide the users with a "WiFi-like"
 * connection system with symphony. So once you have your scan results and you
 * find the gateway you want to connect to, you can specify that to the module
 * and it will directly connect you to it. You can also re-purpose this scan in
 * many ways to see what gateways your module can see for RSSI analysis and much more.
 * This scan will not attempt to connect to any gateway, but will wait for the user
 * to either manually connect to a gateway, or set the scan_config in order to scan
 * regularly.
 */
enum ll_scan_mode {
    LLABS_QUICK_SCAN_AND_CONNECT,   ///< Quick Scan resulting in a connection attempt.
    LLABS_NORMAL_SCAN_AND_CONNECT,  ///< Normal Scan resulting in a connection attempt.
    LLABS_INFO_SCAN                 ///< Full Scan resulting in a list of local gateways.
};

enum ll_downlink_mode {
    LL_DL_OFF = 0,          ///< 0x00
    LL_DL_ALWAYS_ON = 1,    ///< 0x01
    LL_DL_MAILBOX = 2,      ///< 0x02
};

enum ll_state {
    LL_STATE_IDLE_CONNECTED = 1,        ///< 0x01
    LL_STATE_IDLE_DISCONNECTED = 2,     ///< 0x02
    LL_STATE_INITIALIZING = 3,          ///< 0x03
    LL_STATE_ERROR = -1,                ///< 0xFF
};

enum ll_tx_state {
    LL_TX_STATE_TRANSMITTING = 1,       ///< 0x01
    LL_TX_STATE_SUCCESS = 2,            ///< 0x02
    LL_TX_STATE_ERROR = -1,             ///< 0xFF
};

enum ll_rx_state {
    LL_RX_STATE_NO_MSG = 0,             ///< 0x00
    LL_RX_STATE_RECEIVED_MSG = 1,       ///< 0x01
};

/**
 * @brief An exposed buffer that will be used internally, but can also be used
 *   to hold uplink messages for 'll_message_send' to avoid double-buffering.
 *   The buffer has size 256 bytes.
 */
extern uint8_t *uplink_message_buff;

extern const llabs_dl_band_cfg_t DL_BAN_FCC;  ///< USA / Mexico DL Band Configuration
extern const llabs_dl_band_cfg_t DL_BAN_BRA;  ///< Brazil DL Band Configuration
extern const llabs_dl_band_cfg_t DL_BAN_AUS;  ///< Australia DL Band Configuration
extern const llabs_dl_band_cfg_t DL_BAN_NZL;  ///< New Zealand DL Band Configuration
extern const llabs_dl_band_cfg_t DL_BAN_ETSI; ///< Europe (ETSI) DL Band Configuration
/**
 * @brief
 *   Set the scan attemps on the module
 *
 * @details
 *   Sets the number of times a scan will restart upon a 
 *   failed scan before going to disconnect idle state.
 *
 * @param[in] scan_attempts
 *   The number of times the scan will restart upon a failed scan.
 *   0 = Infinite Scan Attempts; Range (1, 65535)
 *
 * @return
 *   0 - success, negative otherwise.
 */
int32_t ll_scan_attempts_set(uint16_t scan_attempts);

/**
 * @brief
 *   Set the current configuration of the module.
 *
 * @details
 *   Sets the configuration values for the module. The module
 *   will only respond to values that have changed. For example,
 *   if the app token is set to the app token that the module already
 *   has, it will not try to re-register the app token.
 *
 *   The user should call this function whenever the module is powered on.
 *
 * @param[in] net_token
 *   The network token set for this module. This determines what gateways the
 *   module will connect to. Use `OPEN_NET_TOKEN` if you don't have a particular
 *   network token.
 *
 * @param[in] app_token
 *   The app token set for this module. This is what registers your uplink
 *   messages with your Conductor account.
 *
 * @param[in] dl_mode
 *   The downlink mode for this module.
 *
 * @param[in] qos
 *   The quality of service level [0-15] for this module. The higher the
 *   quality of service, the faster uplink messages will be transmitted.
 *   Note that too many modules with a high quality of service may adversely
 *   affect network capacity.
 *
 * @return
 *   0 - success, negative otherwise.
 */
int32_t ll_config_set(uint32_t net_token, const uint8_t app_token[APP_TOKEN_LEN],
                      enum ll_downlink_mode dl_mode, uint8_t qos);

/**
 * @brief
 *   Get the current configuration of the module.
 *
 * @details
 *   Returns the configuration set by the user (or defaults
 *   if the user didn't set config yet). The config parameters
 *   may not be in effect yet. For example, this function will return
 *   the downlink mode that the user requested even though
 *   the module may not have entered that downlink mode yet.
 *
 * @param[out] net_token
 *   The network token set for this module.
 *
 * @param[out] app_token
 *   The app token set for this module.
 *
 * @param[out] dl_mode
 *   The downlink mode requested for this module.
 *
 * @param[out] qos
 *   The quality of service level [0-15] requested for this module.
 *
 * @return
 *   0 - success, negative otherwise.
 */
int32_t ll_config_get(uint32_t *net_token, uint8_t app_token[APP_TOKEN_LEN],
                      enum ll_downlink_mode * dl_mode, uint8_t *qos);

/**
 * @brief
 *   Set the current scan configuration of the module.
 *
 * @details
 *   Sets the scan configuration values for the module.
 *   Calling this function will put the module in scanning mode
 *   automatically.
 *
 * @param[in] scan_mode
 *   The type of scan that should be done.
 *
 * @param[in] threshold
 *   The minimum RSSI threshold for the module to accept a gateway or repeater connection.
 *   0 = No RSSI threshold;
 *
 * @param[in] scan_attempts
 *   The number of times the scan will restart upon a failed scan.
 *   0 = Infinite Scan Attempts; Range (1, 65535)
 *
 * @return
 *   0, on success, negative otherwise
 */
int32_t ll_scan_config_set(enum ll_scan_mode scan_mode, int16_t threshold, uint16_t scan_attempts);

/**
 * @brief
 *   Get the current scan configuration of the module.
 *
 * @details
 *   Returns the configuration of the module set by the user
 *   (or the defualts if the user didn't get the scan config
 *   yet).
 *
 * @param[out] scan_type
 *  Pointer to the scan type of the module.
 *
 * @param[out] threshold
 *  Pointer to the minimum RSSI threshold to connect to a gateway.
 *  0 = No RSSI threshold;
 *
 * @param[out] scan_attempts
 *  Pointer to the number of times the scan will restart upon a failed scan.
 *  0 = Infinite Scan Attempts; Range (1, 65535)
 *
 * @param[out] scans_left
 *  Pointer to how many scan attempts the module still has to do before becoming idle.
 *
 * @return
 *   0, on success, negative otherwise
 */
int32_t ll_scan_config_get(enum ll_scan_mode *scan_mode, int16_t *threshold, uint16_t *scan_attempts, uint16_t *scans_left);
/**
 * @brief
 *   Get the current scan mode of the module.
 *
 * @details
 *   Returns the scan mode (NORMAL, INFO, QUICK) of the module set by the user
 *   (or the defualts if the user didn't get the scan mode
 *   yet).
 *
 * @param[out] scan_mode
 *  Pointer to the scan mode of the module.
 *
 * @return
 *   0, on success, negative otherwise
 */
int32_t ll_scan_mode_get(enum ll_scan_mode *scan_mode);

/**
 * @brief
 *   Set the current scan mode of the module.
 *
 * @details
 *   Set the scan mode (NORMAL, INFO, QUICK) of the module 
 *
 * @param[out] scan_mode
 *  scan mode of the module.
 *
 * @return
 *   0, on success, negative otherwise
 */
int32_t ll_scan_mode_set(enum ll_scan_mode scan_mode);


/**
 * @brief
 *   Provides the user with a list of available access points and allows the user to decide
 *   which one to connect to.
 *
 * @details
 *  You can only retrieve a scan list when the gateway is in a "INFO SCAN" mode, this can be
 *  set in the scan_config, and setting the new values will trigger a new scan.
 *  This method should be constantly polled at any interval during and after the scan, while
 *  the scan is executing, you will receive a nack - "busy try again" until the results are given.
 *
 * @param[out] scan_results
 *  This is an array of network information that contains the results of the scan.
 *
 * @param[out] num_gw
 *  The number of gateways scanned.
 *
 * @return
 *   0, on success; NACK_BUSY_TRY_AGAIN, While scan is still going, negative otherwise
 */
int32_t ll_get_gateway_scan_results(llabs_gateway_scan_results_t (*scan_results)[MAX_GW_SCAN_RESULTS], uint8_t* num_gw);

/**
 * @brief
 *   Provides an extensible way to get gateway scan results, when you call this function
 *   (given the module has scan results stored), you will receive a scan result and the
 *   number of scan results left for you to poll.
 *
 * @details
 *  You can only retrieve a scan list when the gateway is in a "INFO SCAN" mode, this can be
 *  set in the scan_config, and setting the new values will trigger a new scan.
 *  This method should be constantly polled at any interval during and after the scan, while
 *  the scan is executing, you will receive a nack - "busy try again" until the results are given.
 *
 * @param[out] scan_results
 *  The scan results of the given gateway.
 *
 * @param[out] num_gw
 *  The current gateway number as well as how many gateways are left to poll.
 *
 * @return
 *   0, on success; NACK_BUSY_TRY_AGAIN, While scan is still going, negative otherwise
 */
int32_t ll_poll_scan_result(llabs_gateway_scan_results_t *scan_result, uint8_t *num_gw);

/**
 * @brief
 *   Connect to a gateway based on the Channel it is on.
 *
 * @details
 *   This function will set the modules downlink channel to that of the
 *   requested channel and attempt to sync with any gateway on that channel.
 *   This function call does not require any scan results to be effective.
 *   Will only take effect when in IDLE_NO_GW mode.
 *
 * @param[in] channel
 *   The channel of the gateway that should be connected to. Must be between
 *   0 and 45.
 *
 * @return
 *   0, on success, negative otherwise
 */
int32_t ll_connect_to_gw_channel(uint8_t channel);

/**
 * @brief
 *   Disconnects the module from any current gateway.
 *
 * @details
 *   The state that the module is left in depends on the scan_config,
 *   since this call could either put the gateway in IDLE_NO_GW or can
 *   just start a new scan. Will only take effect when in IDLE_GW mode.
 *
 * @return
 *   0, on success, negative otherwise
 */
int32_t ll_disconnect(void);

/**
 * @brief
 *   Enable or Disable the Control Message Feature.
 *
 * @details
 *   The control message feature allows the module to receive the following
 *   messages from conductor and bypass the host application completely.
 *       * Module Reset
 *       * Module Info Scan & Report
 *       * Connect to Gateway Channel
 *       * Get Module Stats
 *       * Cloud GPIO Functions
 *
 *   These functions are good for recovering modules that have a locked up host
 *   and are also good for deployment operations with site analysis. Using the
 *   info scan and report function with the connect to gateway channel commands
 *   can give a server operator a lot of control over their deployment and fix
 *   any possible connectivity issues. The module stats command can also give
 *   a server administrator an idea of how the module is doing on a system level.
 *   Cloud GPIO functions have an infinite number of possibilities that can be used
 *   and configured how ever the host application sees fit.
 *
 *   When the control message feature is enabled the module will automatically send
 *   out a message very similar to the mailbox request message. This message will be
 *   sent at a constant 15 minute interval to request any control messages that might
 *   be pending. In Downlink Always On mode the module will still send out this message
 *   in order to give the server a "heartbeat" or a constant ping to keep updating both
 *   the server's and the module's idea of connectivity between them. When the module is
 *   set for Downlink Always On mode the gateway will not hold the control message for the
 *   control message request, but instead will send it down ASAP because the module should
 *   be listening for it at that point.
 *
 * @param[in] enable
 *   If you want to enable the control message feature or not.
 *
 * @return
 *   0, on success, negative otherwise
 */
int32_t ll_control_messages_enabled_set(bool enable);

/**
 * @brief
 *   Get if the Control Message Feature is Enabled or Disabled.
 *
 * @param[out] enabled
 *   If the control message feature is enabled or not.
 *
 * @return
 *   0, on success, negative otherwise
 */
int32_t ll_control_messages_enabled_get(bool *enabled);

/**
 * @brief
 *   This function will activate the specified GPIO pin, by default all pins are disabled but
 *   as soon as a pin is enabled, that setting is persisted through module resets / reboots.
 *
 *   Activating disabled pins will increase battery usage considerably.
 *
 * @details
 *   These GPIO pins are accessible from the cloud via control messages that enable the following
 *   commands to interface with the GPIO...
 *      * Activate Pin ( This Function )
 *      * Deactivate Pin
 *      * Get Status
 *      * Write to Pin
 *
 *   The module will also send a notification to the cloud when the GPIO input pin is triggered
 *   to enable external callbacks from the cloud. If you want to use the GPIO feature by itself
 *   and not send out the triggered read control message notification, then you can completely
 *   disable the control message feature with `ll_control_messages_enabled_set(false);`.
 *
 * @param[in] pin
 *   Corresponds to the Virtual GPIO pin layout...
 *
 *  --------------------------------------------------------------------------------------------
 *  | Virtual Pin # | Module Pin # | Function                                                  |
 *  |---------------|--------------|-----------------------------------------------------------|
 *  |  Pin 0        |  Pin 12      |  Output Pin                                               |
 *  |  Pin 1        |  Pin 23      |  Output Pin                                               |
 *  |  Pin 2        |  Pin 29      |  Input Pin [ Triggers Callback on Falling Edge ]          |
 *  |  Pin 3        |  Pin 31      |  Input Pin [ Triggers Callback on Rising  Edge ]          |
 *  --------------------------------------------------------------------------------------------
 *
 * @return
 *   0, on success, negative otherwise
 */
int32_t ll_cloud_gpio_activate_pin(uint8_t pin);

/**
 * @brief
 *   This function will deactivate the specified GPIO pin, by default all pins are disabled but
 *   as soon as a pin is enabled, that setting is persisted through module resets / reboots.
 *
 *   Disabling active pins will decrease battery usage considerably.
 *
 * @details
 *   These GPIO pins are accessible from the cloud via control messages that enable the following
 *   commands to interface with the GPIO...
 *      * Activate Pin
 *      * Deactivate Pin ( This Function )
 *      * Get Status
 *      * Write to Pin
 *
 *   The module will also send a notification to the cloud when the GPIO input pin is triggered
 *   to enable external callbacks from the cloud. If you want to use the GPIO feature by itself
 *   and not send out the triggered read control message notification, then you can completely
 *   disable the control message feature with `ll_control_messages_enabled_set(false);`.
 *
 * @param[in] pin
 *   Corresponds to the Virtual GPIO pin layout...
 *
 *  --------------------------------------------------------------------------------------------
 *  | Virtual Pin # | Module Pin # | Function                                                  |
 *  |---------------|--------------|-----------------------------------------------------------|
 *  |  Pin 0        |  Pin 12      |  Output Pin                                               |
 *  |  Pin 1        |  Pin 23      |  Output Pin                                               |
 *  |  Pin 2        |  Pin 29      |  Input Pin [ Triggers Callback on Falling Edge ]          |
 *  |  Pin 3        |  Pin 31      |  Input Pin [ Triggers Callback on Rising  Edge ]          |
 *  --------------------------------------------------------------------------------------------
 *
 * @return
 *   0, on success, negative otherwise
 */
int32_t ll_cloud_gpio_deactivate_pin(uint8_t pin);

/**
 * @brief
 *   Get the status of the requested GPIO pin to see if it is activated or not.
 *
 * @details
 *   These GPIO pins are accessible from the cloud via control messages that enable the following
 *   commands to interface with the GPIO...
 *      * Activate Pin
 *      * Deactivate Pin
 *      * Get Status ( This Function )
 *      * Write to Pin
 *
 *   The module will also send a notification to the cloud when the GPIO input pin is triggered
 *   to enable external callbacks from the cloud. If you want to use the GPIO feature by itself
 *   and not send out the triggered read control message notification, then you can completely
 *   disable the control message feature with `ll_control_messages_enabled_set(false);`.
 *
 * @param[in] pin
 *   Corresponds to the Virtual GPIO pin layout...
 *
 *  --------------------------------------------------------------------------------------------
 *  | Virtual Pin # | Module Pin # | Function                                                  |
 *  |---------------|--------------|-----------------------------------------------------------|
 *  |  Pin 0        |  Pin 12      |  Output Pin                                               |
 *  |  Pin 1        |  Pin 23      |  Output Pin                                               |
 *  |  Pin 2        |  Pin 29      |  Input Pin [ Triggers Callback on Falling Edge ]          |
 *  |  Pin 3        |  Pin 31      |  Input Pin [ Triggers Callback on Rising  Edge ]          |
 *  --------------------------------------------------------------------------------------------
 *
 * @param[out] active
 *   Whether or not the pin is active or not.
 *
 * @return
 *   0, on success, negative otherwise
 */
int32_t ll_cloud_gpio_get_pin_status(uint8_t pin, bool *active);

/**
 * @brief
 *   Sets the requested GPIO pin to output HIGH. This function only works for output pins.
 *
 * @details
 *   These GPIO pins are accessible from the cloud via control messages that enable the following
 *   commands to interface with the GPIO...
 *      * Activate Pin
 *      * Deactivate Pin
 *      * Get Status
 *      * Write to Pin ( This Function )
 *
 *   The module will also send a notification to the cloud when the GPIO input pin is triggered
 *   to enable external callbacks from the cloud. If you want to use the GPIO feature by itself
 *   and not send out the triggered read control message notification, then you can completely
 *   disable the control message feature with `ll_control_messages_enabled_set(false);`.
 *
 * @param[in] pin
 *   Corresponds to the Virtual GPIO pin layout...
 *
 *  --------------------------------------------------------------------------------------------
 *  | Virtual Pin # | Module Pin # | Function                                                  |
 *  |---------------|--------------|-----------------------------------------------------------|
 *  |  Pin 0        |  Pin 12      |  Output Pin                                               |
 *  |  Pin 1        |  Pin 23      |  Output Pin                                               |
 *  |  Pin 2        |  Pin 29      |  Input Pin [ Triggers Callback on Falling Edge ]          |
 *  |  Pin 3        |  Pin 31      |  Input Pin [ Triggers Callback on Rising  Edge ]          |
 *  --------------------------------------------------------------------------------------------
 *
 * @return
 *   0, on success, negative otherwise
 */
int32_t ll_cloud_gpio_set_pin_high(uint8_t pin);

/**
 * @brief
 *   Sets the requested GPIO pin to output LOW. This function only works for output pins.
 *
 * @details
 *   These GPIO pins are accessible from the cloud via control messages that enable the following
 *   commands to interface with the GPIO...
 *      * Activate Pin
 *      * Deactivate Pin
 *      * Get Status
 *      * Write to Pin ( This Function )
 *
 *   The module will also send a notification to the cloud when the GPIO input pin is triggered
 *   to enable external callbacks from the cloud. If you want to use the GPIO feature by itself
 *   and not send out the triggered read control message notification, then you can completely
 *   disable the control message feature with `ll_control_messages_enabled_set(false);`.
 *
 * @param[in] pin
 *   Corresponds to the Virtual GPIO pin layout...
 *
 *  --------------------------------------------------------------------------------------------
 *  | Virtual Pin # | Module Pin # | Function                                                  |
 *  |---------------|--------------|-----------------------------------------------------------|
 *  |  Pin 0        |  Pin 12      |  Output Pin                                               |
 *  |  Pin 1        |  Pin 23      |  Output Pin                                               |
 *  |  Pin 2        |  Pin 29      |  Input Pin [ Triggers Callback on Falling Edge ]          |
 *  |  Pin 3        |  Pin 31      |  Input Pin [ Triggers Callback on Rising  Edge ]          |
 *  --------------------------------------------------------------------------------------------
 *
 * @return
 *   0, on success, negative otherwise
 */
int32_t ll_cloud_gpio_set_pin_low(uint8_t pin);

/**
 * @brief
 *   Gets the state of the module.
 *
 * @details
 *   Returns the state of the module as three separate states: The general
 *   connection state, the state of the current uplink message, and the
 *   state of the current downlink message.
 *
 * @param[out] state
 *   The state of the connection. If the state is `LL_STATE_ERROR`, then this
 *   invalidates any of the other state variables.
 *
 * @param[out] tx_state
 *   The state of the transmission. If no messages have been sent by the user since the
 *   last reboot, then this variable is invalid. Otherwise, it returns the result of
 *   the last transmission (or LL_TX_STATE_TRANSMITTING if the message is in progress).
 *
 * @param[out] rx_state
 *   The state of the received message. Either the module has a downlink message ready for
 *   the user to pull out with `ll_retrieve_message`, or not. Once the user pulls out the
 *   message using `ll_retrieve_message`, this state will be reset.
 *
 * @return
 *   0 - success, negative otherwise.
 */
int32_t ll_get_state(enum ll_state * state, enum ll_tx_state * tx_state, enum ll_rx_state * rx_state);

/**
 *@brief
 *  Request.
 *
 *@return
 *   0 - success, negative otherwise (Fails if module is not in MAILBOX mode).
 */
int32_t ll_mailbox_request(void);

/**
 *@brief
 *  Get the end node's application token's registration status.
 *
 *@param[out] is_registered
 *  1=>registered, 0=>otherwise
 *
 *@return
 *   0 - success, negative otherwise.
 */
int32_t ll_app_reg_get(uint8_t *is_registered);

/**
 *@brief
 *  Request a new key exchange between end node and gateway.
 *
 *@return
 *   0 - success, negative otherwise.
 */
int32_t ll_encryption_key_exchange_request(void);

/**
 * @brief
 *   Get the Network Info.
 *
 * @param[out] net_info
 *   Network Info:
 *   All multi byte values are sent over in little-endian mode.
 *   Byte [0-3]   : uint32_t network id (node)
 *   Byte [4-7]   : uint32_t network id (gateway)
 *   Byte [8]     : int8_t gateway channel
 *   Byte [9-12]  : uint32_t gateway frequency
 *   Byte [13-16] : uint32_t Seconds elapsed since last beacon Rx'd
 *   Byte [17-18] : int16_t Downlink RSSI [dBm]
 *   Byte [19]    : uint8_t Downlink SNR [dB]
 *   Byte [20-23] : uint32_t Connection Status (0=Unknown, 1=Disconnected, 2=Connected)
 *
 * @return
 *   0 - success, negative otherwise.
 */
int32_t ll_net_info_get(llabs_network_info_t *net_info);

/**
 * @brief
 *   Get the Network Communication Statistics.
 *
 * @param[out] s
 *   The stats struct sent by the module.
 *
 * @return
 *   0 - success, negative otherwise.
 */
int32_t ll_stats_get(llabs_stats_t *s);

/**
 * @brief
 *   Get the Downlink Band Configuration.
 *
 * @param[out] p_dl_band_cfg
 *   The band cfg struct sent by the module.
 *
 * @return
 *   0 - success, negative otherwise.
 */
int32_t ll_dl_band_cfg_get(llabs_dl_band_cfg_t *p_dl_band_cfg);


/**
 * @brief
 *   Set the Downlink Band Configuration.
 *
 * @param[in] p_dl_band_cfg
 *   The band cfg struct sent by the module.
 *
 * @return
 *   0 - success, negative otherwise.
 */
int32_t ll_dl_band_cfg_set(const llabs_dl_band_cfg_t *p_dl_band_cfg);

/**
 * @brief
 *    Get what the RSSI offset calibration is.
 *
 * @param[out] rssi_offset
 *   the rssi offset value.
 *
 * @ return
 *   0 - success, negative otherwise.
 */
int32_t ll_rssi_offset_get(int8_t *rssi_offset);

/**
 * @brief
 *   Calibrate the module's rssi value with an offset.
 *
 * @note
 *   The range for an RSSI offset is (-15, 15).
 *
 * @param[out] rssi_offset
 *   the rssi offset value.
 *
 * @ return
 *   0 - success, negative otherwise.
 */
int32_t ll_rssi_offset_set(int8_t rssi_offset);

/**
 * @brief
 *   Get the Connection Filter value.
 *
 * @param[out] p_f
 *   The current value is written to *p_f.
 *   0 = Allow connections to any available devices
 *   1 = Allow connection to Gateways only
 *   2 = Allow connection to Repeaters only
 *
 * @return
 *   0 - success, negative otherwise.
 */
int32_t ll_connection_filter_get(uint8_t* p_f);

/**
 * @brief
 *   Set the Connection Filter value.
 *
 * @param[in] f
 *   Same param ll_connection_filter_get().
 *
 * @return
 *   0 - success, negative otherwise.
 */
int32_t ll_connection_filter_set(uint8_t f);

/**
 * @brief
 *   Get the system time as number of seconds since the
 *   UNIX epoch 00:00:00 UTC on 1 January 1970
 *   and the time the module last synchronized time with the gateway.
 *
 * @param[out]
 *   The time_info struct sent by the module.
 *
 * @return
 *   0 - success, negative otherwise.
 */
int32_t ll_system_time_get(llabs_time_info_t *time_info);

/**
 * @brief
 *   Force the module to synchronize system time from the gateway.
 *
 * @note
 *   worse-case delay for synchronization is the Info Block period
 *   which defaults to 8 seconds.
 *
 * @param[in] sync_mode
 *   0 - Time sync only when requested.
 *   1 - Time sync opportunistically.
 *
 * @return
 *   0 - success, negative otherwise.
 */
int32_t ll_system_time_sync(uint8_t sync_mode);

/**
 * @brief
 *   Request Symphony MAC to send an acknowledged uplink message.
 *
 * @details
 *   When the message finishes transmission the module will return to the idle state.
 *   Internally the function copies the message into a larger buffer. If you want to
 *   avoid having two buffers you can use the larger buffer (uplink_message_buff)
 *   to hold the message then call this function with that buffer.
 *
 * @param[in] buf
 *   byte array containing the data payload.
 *
 * @param[in] len
 *   length of the input buffer in bytes.
 *
 * @param[in] ack
 *   Whether or not to request an ACK from the gateway.
 *
 * @param[in] port
 *   The port number to send to. Valid user ports are 0 through 127.
 *   Ports 128 through 255 are reserved for predefined protocols.
 *
 * @return
 *   positive number of bytes queued,
 *   negative if an error.
 */
int32_t ll_message_send(uint8_t buf[], uint16_t len, bool ack, uint8_t port);

/**
 * @brief
 *   Retrieves a downlink message received by the module.
 *
 * @details
 *   This function should be called when the `rx_state` variable from the `get_state`
 *   function is `LL_RX_STATE_RECEIVED_MSG'.
 *
 * @param[out] buf
 *   The buffer into which the received message will be placed. This buffer must
 *   be at least 4 bytes larger than the maximum message size expected
 *   (4 bytes will be used to get the RSSI, SNR, and port).
 *
 * @param[in,out] size
 *   The size of the output buffer. If the message is successfully retrieved,
 *   this will be set to the size of the message (without the RSSI, SNR, and port).
 *
 * @param[out] port
 *   The port number associated with the message.
 *
 * @param[out] rssi
 *   The rssi of the received message.
 *
 * @param[out] snr
 *   The snr of the received message.
 *
 * @return
 *   0 - success, negative otherwise.
 */
int32_t ll_retrieve_message(uint8_t *buf, uint16_t *size, uint8_t *port, int16_t *rssi, uint8_t *snr);

/** @} (end defgroup Symphony_Interface) */

/** @} (end addtogroup Link_Labs_Interface_Library) */


#ifdef __cplusplus
}
#endif

#endif
