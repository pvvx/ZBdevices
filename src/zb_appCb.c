/********************************************************************************************************
 * @file    zb_appCb.c
 *
 * @brief   This is the source file for zb_appCb
 *
 * @author  Zigbee Group
 * @date    2021
 *
 * @par     Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *			All rights reserved.
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/


/**********************************************************************
 * INCLUDES
 */
#include "tl_common.h"
#include "zb_api.h"
#include "zcl_include.h"
#include "bdb.h"
#include "ota.h"
#include "device.h"
#include "app_ui.h"
#include "lcd.h"
#include "zigbee_ble_switch.h"
#include "stack/ble/ble.h"
#include "ble_cfg.h"
/**********************************************************************
 * LOCAL CONSTANTS
 */


/**********************************************************************
 * TYPEDEFS
 */


/**********************************************************************
 * LOCAL FUNCTIONS
 */
void zbdemo_bdbInitCb(u8 status, u8 joinedNetwork);
void zbdemo_bdbCommissioningCb(u8 status, void *arg);
void zbdemo_bdbIdentifyCb(u8 endpoint, u16 srcAddr, u16 identifyTime);


/**********************************************************************
 * LOCAL VARIABLES
 */
bdb_appCb_t g_zbDemoBdbCb =
{
	zbdemo_bdbInitCb,
	zbdemo_bdbCommissioningCb,
	zbdemo_bdbIdentifyCb,
	NULL
};

#ifdef ZCL_OTA
ota_callBack_t sensorDevice_otaCb =
{
	sensorDevice_otaProcessMsgHandler,
};
#endif

/**********************************************************************
 * FUNCTIONS
 */
s32 sensorDevice_bdbNetworkSteerStart(void *arg){

	bdb_networkSteerStart();

	g_sensorAppCtx.timerSteerEvt = NULL;
	return -1;
}

#if REJOIN_FAILURE_TIMER

#define REJOIN_FAILURE_COUNT	55 // 55 sec

inline void sensorDevice_rejoin_faillure_timer_set(void) {
	u32 period;
	if(g_sensorAppCtx.timerSteerEvt) {
		TL_ZB_TIMER_CANCEL(&g_sensorAppCtx.timerSteerEvt);
	}
	if(++g_sensorAppCtx.rejoin_cnt > 200) // max 240 sec
		g_sensorAppCtx.rejoin_cnt = 200;
	if(g_sensorAppCtx.rejoin_cnt < 7)
		period = 9 << 10; // ~9 sec
	else if(g_sensorAppCtx.rejoin_cnt < REJOIN_FAILURE_COUNT)
		period = REJOIN_FAILURE_COUNT << 10; // ~50 sec
	else
		period = g_sensorAppCtx.rejoin_cnt << 10; // * 1024, 50..180 sec
	g_sensorAppCtx.timerSteerEvt = TL_ZB_TIMER_SCHEDULE(sensorDevice_bdbNetworkSteerStart, NULL, period);
}

s32 sensorDevice_rejoinBackoff(void *arg){

	if(zb_isDeviceFactoryNew()){
		g_sensorAppCtx.timerRejoinBackoffEvt = NULL;
		return -1;
	}

    zb_rejoinReqWithBackOff(zb_apsChannelMaskGet(), g_bdbAttrs.scanDuration);
    return 0;
}

#endif

/*********************************************************************
 * @fn      zbdemo_bdbInitCb
 *
 * @brief   application callback for bdb initiation
 *
 * @param   status - the status of bdb init BDB_INIT_STATUS_SUCCESS or BDB_INIT_STATUS_FAILURE
 *
 * @param   joinedNetwork  - 1: node is on a network, 0: node isn't on a network
 *
 * @return  None
 */
void zbdemo_bdbInitCb(u8 status, u8 joinedNetwork){
	if(status == BDB_INIT_STATUS_SUCCESS){
		/*
		 * for non-factory-new device:
		 * 		load zcl data from NV, start poll rate, start ota query, bdb_networkSteerStart
		 *
		 * for factory-new device:
		 * 		steer a network
		 *
		 */
		if(joinedNetwork){
			zb_setPollRate(DEFAULT_POLL_RATE);

#ifdef ZCL_OTA
			ota_queryStart(15 * 60);
#endif

#ifdef ZCL_POLL_CTRL
			sensorDevice_zclCheckInStart();
#endif
		} else {
			u16 jitter = 0;
			do {
				jitter = zb_random() % 0x0fff;
			} while(jitter == 0);

			if(g_sensorAppCtx.timerSteerEvt){
				TL_ZB_TIMER_CANCEL(&g_sensorAppCtx.timerSteerEvt);
			}
			///time_soff = 0;
			g_sensorAppCtx.timerSteerEvt = TL_ZB_TIMER_SCHEDULE(sensorDevice_bdbNetworkSteerStart, NULL, jitter);

			g_dualModeInfo.bleStart = 1;
		}
	}
#if REJOIN_FAILURE_TIMER
	else
	{
		if(joinedNetwork){
			zb_rejoinReqWithBackOff(zb_apsChannelMaskGet(), g_bdbAttrs.scanDuration);
		}
		g_dualModeInfo.bleStart = 1;

	}
#endif
//	g_sensorAppCtx.rejoin_cnt = REJOIN_FAILURE_COUNT;
}

/*********************************************************************
 * @fn      zbdemo_bdbCommissioningCb
 *
 * @brief   application callback for bdb commissioning
 *
 * @param   status - the status of bdb commissioning
 *
 * @param   arg
 *
 * @return  None
 */
void zbdemo_bdbCommissioningCb(u8 status, void *arg){
	switch(status){
		case BDB_COMMISSION_STA_SUCCESS:

			ble_task_stop();	// отключение BLE
			g_sensorAppCtx.rejoin_cnt = REJOIN_FAILURE_COUNT;

			light_blink_start(7, 500, 500);
			zb_setPollRate(DEFAULT_POLL_RATE);

			if(g_sensorAppCtx.timerSteerEvt){
				TL_ZB_TIMER_CANCEL(&g_sensorAppCtx.timerSteerEvt);
			}
#if REJOIN_FAILURE_TIMER
			if(g_sensorAppCtx.timerRejoinBackoffEvt){
				TL_ZB_TIMER_CANCEL(&g_sensorAppCtx.timerRejoinBackoffEvt);
			}
#endif
#ifdef ZCL_POLL_CTRL
		    sensorDevice_zclCheckInStart();
#endif
#ifdef ZCL_OTA
			ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
#endif
#if	USE_DISPLAY
			show_connected_symbol(true);
#endif
			break;
		case BDB_COMMISSION_STA_IN_PROGRESS:
			break;
		case BDB_COMMISSION_STA_NOT_AA_CAPABLE:
			break;
		case BDB_COMMISSION_STA_NO_NETWORK:
		case BDB_COMMISSION_STA_TCLK_EX_FAILURE:
		case BDB_COMMISSION_STA_TARGET_FAILURE:
			{
#if REJOIN_FAILURE_TIMER
				sensorDevice_rejoin_faillure_timer_set();
#else
				u16 jitter = 0;
				do{
					jitter = zb_random() % 0x0fff;
				}while(jitter == 0);

				if(g_sensorAppCtx.timerSteerEvt){
					TL_ZB_TIMER_CANCEL(&g_sensorAppCtx.timerSteerEvt);
				}
				g_sensorAppCtx.timerSteerEvt = TL_ZB_TIMER_SCHEDULE(sensorDevice_bdbNetworkSteerStart, NULL, jitter);
#endif
#if	USE_DISPLAY
				show_connected_symbol(false);
#endif
				g_dualModeInfo.bleStart = 1;
			}
			break;
		case BDB_COMMISSION_STA_FORMATION_FAILURE:
			break;
		case BDB_COMMISSION_STA_NO_IDENTIFY_QUERY_RESPONSE:
			break;
		case BDB_COMMISSION_STA_BINDING_TABLE_FULL:
			break;
		case BDB_COMMISSION_STA_NOT_PERMITTED:
			break;
		case BDB_COMMISSION_STA_NO_SCAN_RESPONSE:
		case BDB_COMMISSION_STA_PARENT_LOST:
#if REJOIN_FAILURE_TIMER
			sensorDevice_rejoinBackoff(NULL);
#else
			zb_rejoinReqWithBackOff(zb_apsChannelMaskGet(), g_bdbAttrs.scanDuration);
#endif
#if	USE_DISPLAY
			show_connected_symbol(false);
#endif
			g_dualModeInfo.bleStart = 1;
			break;
		case BDB_COMMISSION_STA_REJOIN_FAILURE:
			if(!zb_isDeviceFactoryNew()){
#if REJOIN_FAILURE_TIMER
                // sleep for 6 minutes before reconnect if rejoin failed
                g_sensorAppCtx.timerRejoinBackoffEvt = TL_ZB_TIMER_SCHEDULE(sensorDevice_rejoinBackoff, NULL, 360000);
#else
				zb_rejoinReqWithBackOff(zb_apsChannelMaskGet(), g_bdbAttrs.scanDuration);
#endif
			}
#if	USE_DISPLAY
			show_connected_symbol(false);
#endif
			g_dualModeInfo.bleStart = 1;
			break;
		default:
			break;
	}
}


extern void sensorDevice_zclIdentifyCmdHandler(u8 endpoint, u16 srcAddr, u16 identifyTime);
void zbdemo_bdbIdentifyCb(u8 endpoint, u16 srcAddr, u16 identifyTime){
	sensorDevice_zclIdentifyCmdHandler(endpoint, srcAddr, identifyTime);
}

#ifdef ZCL_OTA
void sensorDevice_otaProcessMsgHandler(u8 evt, u8 status)
{
	if(evt == OTA_EVT_START){
		if(status == ZCL_STA_SUCCESS){
			zb_setPollRate(QUEUE_POLL_RATE);
		}else{

		}
	}else if(evt == OTA_EVT_COMPLETE){
		zb_setPollRate(DEFAULT_POLL_RATE);

		if(status == ZCL_STA_SUCCESS){
			ota_mcuReboot();
		}else{
			ota_queryStart(OTA_PERIODIC_QUERY_INTERVAL);
		}
	}else if(evt == OTA_EVT_IMAGE_DONE){
		zb_setPollRate(DEFAULT_POLL_RATE);
	}
}
#endif

/*********************************************************************
 * @fn      sensorDevice_leaveCnfHandler
 *
 * @brief   Handler for ZDO Leave Confirm message.
 *
 * @param   pRsp - parameter of leave confirm
 *
 * @return  None
 */
void sensorDevice_leaveCnfHandler(nlme_leave_cnf_t *pLeaveCnf)
{
    if(pLeaveCnf->status == SUCCESS){
#if REJOIN_FAILURE_TIMER
		if(g_sensorAppCtx.timerRejoinBackoffEvt){
			TL_ZB_TIMER_CANCEL(&g_sensorAppCtx.timerRejoinBackoffEvt);
		}
#endif
    	//zb_resetDevice();
    }
}

/*********************************************************************
 * @fn      sensorDevice_leaveIndHandler
 *
 * @brief   Handler for ZDO leave indication message.
 *
 * @param   pInd - parameter of leave indication
 *
 * @return  None
 */
void sensorDevice_leaveIndHandler(nlme_leave_ind_t *pLeaveInd)
{
    //printf("sensorDevice_leaveIndHandler, rejoin = %d\n", pLeaveInd->rejoin);
    //printfArray(pLeaveInd->device_address, 8);
}
