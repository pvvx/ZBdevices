/*
 * cmd_parser.h
 * Author: pvvx
 */
#ifndef _CMD_PARSER_H_
#define _CMD_PARSER_H_

// A complete list of interface commands for different devices.
// Not all commands are supported by a specific device (!)
enum {
	CMD_ID_DEV_ID 	= 0x00, // Get dev id, version, services
	CMD_ID_DNAME    = 0x01, // Get/Set device name, "\0" - default: ATC_xxxx
	CMD_ID_GDEVS 	= 0x02, // Get address devises
	CMD_ID_I2C_SCAN = 0x03, // I2C scan
	CMD_ID_I2C_UTR	= 0x04, // Universal I2C/SMBUS read-write
	CMD_ID_SEN_ID	= 0x05,	// Get sensor ID
	CMD_ID_FLASH_ID	= 0x06,	// Get Flash JEDEC ID
	CMD_ID_DEV_MAC	= 0x10, // Get/Set MAC [+RandMAC], [size][mac[6][randmac[2]]]
	CMD_ID_MI_DNAME = 0x11, // Get/Set Mi key: DevNameId, [size]["\0"+miDevName]
	CMD_ID_MI_TBIND = 0x12, // Get/Set Mi keys: Token & Bind, [size][keys]
	CMD_ID_MI_CFG   = 0x13, // Mi cfg data, [size][data]
	CMD_ID_MI_KDEL  = 0x14, // Mi marked as deleted keys, [size][data]
	CMD_ID_MI_KALL  = 0x15, // Get all mi keys
	CMD_ID_MI_REST  = 0x16, // Restore prev mi token & bindkeys
	CMD_ID_MI_CLR	= 0x17, // Delete all mi keys
	CMD_ID_BKEY		= 0x18, // Get/Set beacon bindkey in EEP
	CMD_ID_COMFORT  = 0x20, // Get/Set comfort parameters
	CMD_ID_EXTDATA  = 0x22, // Get/Set show ext. data
	CMD_ID_UTC_TIME = 0x23, // Get/Set utc time (if USE_CLOCK = 1)
	CMD_ID_TADJUST  = 0x24, // Get/Set adjust time clock delta (in 1/16 us for 1 sec)
	CMD_ID_CFS  	= 0x25, // Get/Set sensor config
	CMD_ID_CFS_DEF 	= 0x26, // Set default sensor config
	CMD_ID_CFB20  	= 0x27, // Get/Set sensor MY18B20 config
	CMD_ID_CFB20_DEF = 0x28, // Set default sensor MY18B20 config
	CMD_ID_RH		= 0x29, // Get/Set sensor RH config
	CMD_ID_RH_CAL	= 0x2A, // Calibrate sensor RH
	CMD_ID_MEASURE  = 0x33, // Start/stop notify measures in connection mode
	CMD_ID_LOGGER   = 0x35, // Read memory measures
	CMD_ID_CLRLOG	= 0x36, // Clear memory measures
	CMD_ID_RDS      = 0x40, // Get/Set Reed switch config (DIY devices)
	CMD_ID_TRG      = 0x44, // Get/Set trg and Reed switch data config
	CMD_ID_TRG_OUT  = 0x45, // Get/Set trg out, Send Reed switch and trg data
	CMD_ID_HXC      = 0x49, // Get/Set HX71X config
	CMD_ID_CFG      = 0x55,	// Get/Set device config
	CMD_ID_CFG_DEF  = 0x56,	// Set default device config
	CMD_ID_ZB_CFG	= 0x57, // Get/Set zb device config
	CMD_ID_ZB_DEF	= 0x58, // Set default zb device config
	CMD_ID_LCD_DUMP = 0x60, // Get/Set lcd buf
	CMD_ID_LCD_FLG  = 0x61, // Start/Stop notify lcd dump and ...
	CMD_ID_ZB_NAME	= 0x68, // Get/Set zb device dev.name
	CMD_ID_ZB_MANU	= 0x69, // Get/Set zb device man.name
	CMD_ID_PINCODE  = 0x70, // Set new PinCode 0..999999
	CMD_ID_MTU		= 0x71, // Request Mtu Size Exchange (23..255)
	CMD_ID_REBOOT	= 0x72, // Set Reboot on disconnect
	CMD_ID_SET_OTA	= 0x73, // Extension BigOTA: Get/set address and size OTA, erase sectors

	// Debug commands (unsupported in different versions!):

	CMD_ID_OTAC		= 0xD1,	// OTA clear
	CMD_ID_WRFB		= 0xD3,	// Write Flash
	CMD_ID_RDFB		= 0xD4,	// Read Flash Block
	CMD_ID_ERFB		= 0xD5,	// Erase Flash Sector
	CMD_ID_CHGB		= 0xD7,	// Change boot
	CMD_ID_EEP_RW	= 0xDC,	// Get/set EEP
	CMD_ID_LR_RESET = 0xDD,	// Reset Long Range
	CMD_ID_DEBUG    = 0xDE	// Test/Debug

} CMD_ID_KEYS;


int cmd_parser(void * p);

#endif // _CMD_PARSER_H_
