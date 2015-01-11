
/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8821PwrSeq.c
	
Abstract:
	This file includes all kinds of Power Action event for RTL8821A and corresponding hardware configurtions which are released from HW SD.
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2011-08-08 Roger            Create.
	
--*/


#include "Hal8821APwrSeq.h"
#include <rtl8812a_hal.h>


/* 
    drivers should parse below arrays and do the corresponding actions
*/
//3 Power on  Array
struct wlan_pwr_cfg rtl8821A_power_on_flow[RTL8821A_TRANS_CARDEMU_TO_ACT_STEPS+RTL8821A_TRANS_END_STEPS]=
{
	RTL8821A_TRANS_CARDEMU_TO_ACT
	RTL8821A_TRANS_END
};

//3Radio off GPIO Array
struct wlan_pwr_cfg rtl8821A_radio_off_flow[RTL8821A_TRANS_ACT_TO_CARDEMU_STEPS+RTL8821A_TRANS_END_STEPS]=
{
	RTL8821A_TRANS_ACT_TO_CARDEMU
	RTL8821A_TRANS_END
};

//3Card Disable Array
struct wlan_pwr_cfg rtl8821A_card_disable_flow[RTL8821A_TRANS_ACT_TO_CARDEMU_STEPS+RTL8821A_TRANS_CARDEMU_TO_PDN_STEPS+RTL8821A_TRANS_END_STEPS]=
{
	RTL8821A_TRANS_ACT_TO_CARDEMU
	RTL8821A_TRANS_CARDEMU_TO_CARDDIS
	RTL8821A_TRANS_END
};

//3 Card Enable Array
struct wlan_pwr_cfg rtl8821A_card_enable_flow[RTL8821A_TRANS_CARDDIS_TO_CARDEMU_STEPS+RTL8821A_TRANS_CARDEMU_TO_ACT_STEPS+RTL8821A_TRANS_END_STEPS]=
{
	RTL8821A_TRANS_CARDDIS_TO_CARDEMU
	RTL8821A_TRANS_CARDEMU_TO_ACT		
	RTL8821A_TRANS_END
};

//3Suspend Array
struct wlan_pwr_cfg rtl8821A_suspend_flow[RTL8821A_TRANS_ACT_TO_CARDEMU_STEPS+RTL8821A_TRANS_CARDEMU_TO_SUS_STEPS+RTL8821A_TRANS_END_STEPS]=
{
	RTL8821A_TRANS_ACT_TO_CARDEMU
	RTL8821A_TRANS_CARDEMU_TO_SUS
	RTL8821A_TRANS_END
};

//3 Resume Array
struct wlan_pwr_cfg rtl8821A_resume_flow[RTL8821A_TRANS_ACT_TO_CARDEMU_STEPS+RTL8821A_TRANS_CARDEMU_TO_SUS_STEPS+RTL8821A_TRANS_END_STEPS]=
{
	RTL8821A_TRANS_SUS_TO_CARDEMU
	RTL8821A_TRANS_CARDEMU_TO_ACT
	RTL8821A_TRANS_END
};



//3HWPDN Array
struct wlan_pwr_cfg rtl8821A_hwpdn_flow[RTL8821A_TRANS_ACT_TO_CARDEMU_STEPS+RTL8821A_TRANS_CARDEMU_TO_PDN_STEPS+RTL8821A_TRANS_END_STEPS]=
{
	RTL8821A_TRANS_ACT_TO_CARDEMU
	RTL8821A_TRANS_CARDEMU_TO_PDN		
	RTL8821A_TRANS_END
};

//3 Enter LPS 
struct wlan_pwr_cfg rtl8821A_enter_lps_flow[RTL8821A_TRANS_ACT_TO_LPS_STEPS+RTL8821A_TRANS_END_STEPS]=
{
	//FW behavior
	RTL8821A_TRANS_ACT_TO_LPS	
	RTL8821A_TRANS_END
};

//3 Leave LPS 
struct wlan_pwr_cfg rtl8821A_leave_lps_flow[RTL8821A_TRANS_LPS_TO_ACT_STEPS+RTL8821A_TRANS_END_STEPS]=
{
	//FW behavior
	RTL8821A_TRANS_LPS_TO_ACT
	RTL8821A_TRANS_END
};
