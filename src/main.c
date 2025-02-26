/********************************************************************************************************
 * @file    concurrent_main.c
 *
 * @brief   This is the source file for concurrent_main
 *
 * @author  Zigbee Group
 * @date    2021
 *
 * @par     Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
 *******************************************************************************************************/
#include "tl_common.h"
#include "zb_common.h"
#include "stack/ble/ble_config.h"
#include "stack/ble/ble_common.h"
#include "stack/ble/ble.h"
#include "zigbee_ble_switch.h"
#include "zcl_include.h"
#include "device.h"
#include "ext_ota.h"
#include "app_ui.h"
#include "ble_cfg.h"

#if ZIGBEE_TUYA_OTA
int flash_main(void)
#else
int main(void)
#endif
{
	u8 isRetention = (drv_platform_init() == SYSTEM_DEEP_RETENTION) ? 1 : 0;

	os_init(isRetention);

	reg_clk_en0 = FLD_CLK0_SPI_EN
#if UART_PRINTF_MODE
			| FLD_CLK0_UART_EN
#endif
			| FLD_CLK0_SWIRE_EN;

#if PA_ENABLE
	rf_paInit(PA_TX, PA_RX);
#endif

	user_zb_init(isRetention);
	ble_radio_init();
	user_ble_init(isRetention);

	if(CURRENT_SLOT_GET() == DUALMODE_SLOT_ZIGBEE){
		switch_to_zb_context();
	}

	drv_enable_irq();

#if (MODULE_WATCHDOG_ENABLE)
	drv_wd_setInterval(600);
    drv_wd_start();
#endif

    while(1) {
#if (MODULE_WATCHDOG_ENABLE)
		drv_wd_clear();
#endif

#if USE_BLE_OTA
		if(!ble_attr.ota_is_working)
#endif
			sensors_task();
		concurrent_mode_main_loop();
		task_keys();
#if PM_ENABLE
		app_pm_task();
#endif
	}
	return 0;
}

