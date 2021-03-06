#include "sw.h"

#include "dm.h"
#include "phy.h"
#include "reg.h"
#include "trx.h"
#include "hw.h"
#include "led.h"

#include <rtl8812a_hal.h>


void rtl8812_free_hal_data(struct rtl_priv *rtlpriv);
void hal_notch_filter_8812(struct rtl_priv *rtlpriv, bool enable);

static uint8_t rtw_init_default_value(struct rtl_priv *rtlpriv)
{
	struct rtl_hal_cfg *cfg = rtlpriv->cfg;
	uint8_t ret  = _SUCCESS;
	struct registry_priv *pregistrypriv = &rtlpriv->registrypriv;
	struct xmit_priv	*pxmitpriv = &rtlpriv->xmitpriv;
	struct mlme_priv *pmlmepriv = &rtlpriv->mlmepriv;
	struct security_priv *psecuritypriv = &rtlpriv->securitypriv;

	/* xmit_priv */
	pxmitpriv->vcs_setting = pregistrypriv->vrtl_carrier_sense;
	pxmitpriv->vcs = pregistrypriv->vcs_type;
	pxmitpriv->vcs_type = pregistrypriv->vcs_type;
	/* pxmitpriv->rts_thresh = pregistrypriv->rts_thresh; */
	pxmitpriv->frag_len = pregistrypriv->frag_thresh;

	/* recv_priv */

	/* mlme_priv */
	pmlmepriv->scan_interval = SCAN_INTERVAL;	/* 30*2 sec = 60sec */
	pmlmepriv->scan_mode = SCAN_ACTIVE;

	/*
	 * qos_priv
	 * pmlmepriv->qospriv.qos_option = pregistrypriv->wmm_enable;
	 */

	/* ht_priv */
#ifdef CONFIG_80211N_HT
	pmlmepriv->htpriv.ampdu_enable = _FALSE; /* set to disabled */
#endif

	/* security_priv */
	/* rtw_get_encrypt_decrypt_from_registrypriv(rtlpriv); */
	psecuritypriv->binstallGrpkey = _FAIL;
	psecuritypriv->sw_encrypt = cfg->mod_params->sw_crypto;
	psecuritypriv->sw_decrypt = cfg->mod_params->sw_crypto;

	psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Open; /* open system */
	psecuritypriv->dot11PrivacyAlgrthm = _NO_PRIVACY_;

	psecuritypriv->dot11PrivacyKeyIndex = 0;

	psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
	psecuritypriv->dot118021XGrpKeyid = 1;

	psecuritypriv->ndisauthtype = Ndis802_11AuthModeOpen;
	psecuritypriv->ndisencryptstatus = Ndis802_11WEPDisabled;


	/* pwrctrl_priv */


	/* registry_priv */
	rtw_init_registrypriv_dev_network(rtlpriv);
	rtw_update_registrypriv_dev_network(rtlpriv);


	/* hal_priv */
	rtw_hal_def_value_init(rtlpriv);

	/* misc. */
	rtlpriv->bReadPortCancel = _FALSE;
	rtlpriv->bWritePortCancel = _FALSE;
	rtlpriv->bLinkInfoDump = 0;
	rtlpriv->bNotifyChannelChange = 0;

	return ret;
}

static int rtl8821au_init_sw_vars(struct net_device *ndev)
{
	struct rtl_priv *rtlpriv = rtl_priv(ndev);

	uint8_t	ret8 = _SUCCESS;

	ret8 = rtw_init_default_value(rtlpriv);

	if ((rtw_init_cmd_priv(&rtlpriv->cmdpriv)) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}

	rtlpriv->cmdpriv.rtlpriv = rtlpriv;

	if ((rtw_init_evt_priv(&rtlpriv->evtpriv)) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}


	if (rtw_init_mlme_priv(rtlpriv) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}

	if (init_mlme_ext_priv(rtlpriv) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}

	if (_rtw_init_xmit_priv(&rtlpriv->xmitpriv, rtlpriv) == _FAIL) 	{
		printk("rtl8821au: Can't _rtw_init_xmit_priv\n");
		ret8 = _FAIL;
		goto exit;
	}

	if (_rtw_init_recv_priv(&rtlpriv->recvpriv, rtlpriv) == _FAIL) {
		printk("rtl8821au: Can't _rtw_init_recv_priv\n");
		ret8 = _FAIL;
		goto exit;
	}

	/*
	 * We don't need to memset rtlpriv->XXX to zero, because rtlpriv is allocated by rtw_zvmalloc().
	 * memset((unsigned char *)&rtlpriv->securitypriv, 0, sizeof (struct security_priv));
	 */

	/* _init_timer(&(rtlpriv->securitypriv.tkip_timer), rtlpriv->pifp, rtw_use_tkipkey_handler, rtlpriv); */

	if (_rtw_init_sta_priv(&rtlpriv->stapriv) == _FAIL) {
		printk("rtl8821au: Can't _rtw_init_sta_priv\n");
		ret8 = _FAIL;
		goto exit;
	}

	rtlpriv->stapriv.rtlpriv = rtlpriv;
	rtlpriv->setband = GHZ24_50;
	rtw_init_bcmc_stainfo(rtlpriv);

	rtw_init_pwrctrl_priv(rtlpriv);

	/* memset((uint8_t *)&rtlpriv->qospriv, 0, sizeof (struct qos_priv));//move to mlme_priv */


	rtw_hal_dm_init(rtlpriv);
	rtw_hal_sw_led_init(rtlpriv);

exit:

	return ret8;

}


static struct rtl_hal_ops rtl8821au_hal_ops = {
	/*
	 * New HAL functions with struct net_device  as first param
	 * this can be (hopefully)switched to struct ieee80211_hw
	 */

	.get_bbreg = 	rtl8821au_phy_query_bb_reg,
	.set_bbreg = 	rtl8821au_phy_set_bb_reg,
	.get_rfreg = 	rtl8821au_phy_query_rf_reg,
	.set_rfreg = 	rtl8821au_phy_set_rf_reg,

	.init_sw_leds = rtl8821au_init_sw_leds,
	.deinit_sw_leds = rtl8812au_deinit_sw_leds,
	.led_control	= rtl8821au_led_control,

	.init_sw_vars	= rtl8821au_init_sw_vars,

	/** ** */

	.fill_fake_txdesc	= rtl8821au_fill_fake_txdesc,

	/* Old HAL functions */

	.hal_init =	 	rtl8812au_hal_init,
	.hal_deinit = 		rtl8812au_hal_deinit,

	/* .free_hal_data = rtl8192c_free_hal_data, */

	.inirp_deinit =		rtl8812au_inirp_deinit,

	.init_xmit_priv =	rtl8812au_init_xmit_priv,
	.free_xmit_priv =	rtl8812au_free_xmit_priv,

	.init_recv_priv =	rtl8812au_init_recv_priv,
	.free_recv_priv =	rtl8812au_free_recv_priv,
	.init_default_value =	rtl8812au_init_default_value,
	.read_adapter_info =	_rtl8821au_read_adapter_info,

	/* .set_bwmode_handler = 	PHY_SetBWMode8192C; */
	/* .set_channel_handler = 	PHY_SwChnl8192C; */

	/* .hal_dm_watchdog = 	rtl8192c_HalDmWatchDog; */


	.set_hw_reg =	 	rtl8821au_set_hw_reg,
	.get_hw_reg = 		rtl8821au_get_hw_reg,
	.GetHalDefVarHandler = 	rtl8821au_get_hal_def_var,

	.SetBeaconRelatedRegistersHandler = 	rtl8821au_set_beacon_related_registers,

	/* .Add_RateATid = &rtl8192c_Add_RateATid, */

	.hal_xmit = 		rtl8812au_hal_xmit,
	.mgnt_xmit = 		rtl8812au_mgnt_xmit,
	.hal_xmitframe_enqueue = 	rtl8812au_hal_xmitframe_enqueue,

	.free_hal_data =	rtl8812_free_hal_data,

	.dm_init =		rtl8812_init_dm_priv,
	.dm_deinit =		rtl8812_deinit_dm_priv,

	.read_chip_version =	rtl8821au_read_chip_version,

	.set_bwmode_handler =	PHY_SetBWMode8812,
	.set_channel_handler =	PHY_SwChnl8812,
	.set_chnl_bw_handler =	PHY_SetSwChnlBWMode8812,

	.hal_dm_watchdog =	rtl8821au_dm_watchdog,

	.Add_RateATid =		rtl8812_Add_RateATid,

	.hal_notch_filter = hal_notch_filter_8812,
};

static struct rtl_hal_usbint_cfg rtl8821au_interface_cfg = {
#if 0
	/* rx */
	.in_ep_num = RTL92C_USB_BULK_IN_NUM,
	.rx_urb_num = RTL92C_NUM_RX_URBS,
	.rx_max_size = RTL92C_SIZE_MAX_RX_BUFFER,
	.usb_rx_hdl = rtl8192cu_rx_hdl,
	.usb_rx_segregate_hdl = NULL, /* rtl8192c_rx_segregate_hdl; */
	/* tx */
	.usb_tx_cleanup = rtl8192c_tx_cleanup,
	.usb_tx_post_hdl = rtl8192c_tx_post_hdl,
	.usb_tx_aggregate_hdl = rtl8192c_tx_aggregate_hdl,
	/* endpoint mapping */
	.usb_endpoint_mapping = rtl8192cu_endpoint_mapping,
	.usb_mq_to_hwq = rtl8192cu_mq_to_hwq,
#endif	
};

/* ULLI : shamelessly copied from rtlwifi */

static struct rtl_mod_params rtl8821au_mod_params = {
	.sw_crypto = false,
	.inactiveps = true,
	.swctrl_lps = false,
	.fwctrl_lps = true,
	.msi_support = true,
#if 0	/* Ulli : currently not defined */	
	.debug = DBG_EMERG,
#endif	
	.disable_watchdog = 0,
};

static struct rtl_hal_cfg rtl8821au_hal_cfg = {
	.name = "rtl8821au",
	.fw_name = "rtlwifi/rtl8821aufw.bin",	/* ULLI note two files */
	.ops = &rtl8821au_hal_ops,
	.mod_params = &rtl8821au_mod_params,
	.maps[SYS_ISO_CTRL] = REG_SYS_ISO_CTRL,
#if 0
	.maps[SYS_FUNC_EN] = REG_SYS_FUNC_EN,
	.maps[SYS_CLK] = REG_SYS_CLKR,
	.maps[MAC_RCR_AM] = AM,
	.maps[MAC_RCR_AB] = AB,
	.maps[MAC_RCR_ACRC32] = ACRC32,
	.maps[MAC_RCR_ACF] = ACF,
	.maps[MAC_RCR_AAP] = AAP,
#endif
	.maps[EFUSE_TEST] = REG_EFUSE_TEST,
	.maps[EFUSE_CTRL] = REG_EFUSE_CTRL,
	.maps[EFUSE_CLK] = 0,
	.maps[EFUSE_CLK_CTRL] = REG_EFUSE_CTRL,
	.maps[EFUSE_PWC_EV12V] = PWC_EV12V,
	.maps[EFUSE_FEN_ELDR] = FEN_ELDR,
	.maps[EFUSE_LOADER_CLK_EN] = LOADER_CLK_EN,
	.maps[EFUSE_ANA8M] = ANA8M,
	.maps[EFUSE_HWSET_MAX_SIZE] = EFUSE_MAP_LEN_JAGUAR,	/* ULLI : or HWSET_MAX_SIZE */
	.maps[EFUSE_MAX_SECTION_MAP] = EFUSE_MAX_SECTION_JAGUAR,	/* ULLI : or EFUSE_MAX_SECTION */
	.maps[EFUSE_REAL_CONTENT_SIZE] =  EFUSE_REAL_CONTENT_LEN_JAGUAR,	/* ULLI : EFUSE_REAL_CONTENT_LEN */
	.maps[EFUSE_OOB_PROTECT_BYTES_LEN] = EFUSE_OOB_PROTECT_BYTES_JAGUAR,	/* ULLI : EFUSE_OOB_PROTECT_BYTES */
	.maps[EFUSE_ACCESS] = REG_EFUSE_BURN_GNT_8812, /* ULLI : or REG_EFUSE_ACCESS as in rtlwifi */
};



extern int rtw_ht_enable;
extern int rtw_bw_mode;
extern int rtw_ampdu_enable;	/* for enable tx_ampdu */

static int rtw_drv_init(struct usb_interface *pusb_intf,const struct usb_device_id *pdid);
static void rtw_dev_remove(struct usb_interface *pusb_intf);

#define USB_VENDER_ID_REALTEK		0x0BDA


/* DID_USB_v916_20130116 */
static struct usb_device_id rtw_usb_id_tbl[] ={
	/* RTL8812AU */
	/*=== Realtek demoboard ===*/
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8812),.driver_info = RTL8812},/* Default ID */
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x881A),.driver_info = RTL8812},/* Default ID */
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x881B),.driver_info = RTL8812},/* Default ID */
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x881C),.driver_info = RTL8812},/* Default ID */
	/*=== Customer ID ===*/
	{USB_DEVICE(0x050D, 0x1109),.driver_info = RTL8812}, /* Belkin F9L1109 - SerComm */
	{USB_DEVICE(0x2001, 0x330E),.driver_info = RTL8812}, /* D-Link - ALPHA */
	{USB_DEVICE(0x7392, 0xA822),.driver_info = RTL8812}, /* Edimax - Edimax */
	{USB_DEVICE(0x0DF6, 0x0074),.driver_info = RTL8812}, /* Sitecom - Edimax */
	{USB_DEVICE(0x04BB, 0x0952),.driver_info = RTL8812}, /* I-O DATA - Edimax */
	{USB_DEVICE(0x0789, 0x016E),.driver_info = RTL8812}, /* Logitec - Edimax */
	{USB_DEVICE(0x0409, 0x0408),.driver_info = RTL8812}, /* NEC - */
	{USB_DEVICE(0x0B05, 0x17D2),.driver_info = RTL8812}, /* ASUS - Edimax */
	{USB_DEVICE(0x0E66, 0x0022),.driver_info = RTL8812}, /* HAWKING - Edimax */
	{USB_DEVICE(0x0586, 0x3426),.driver_info = RTL8812}, /* ZyXEL - */
	{USB_DEVICE(0x2001, 0x3313),.driver_info = RTL8812}, /* D-Link - ALPHA */
	{USB_DEVICE(0x1058, 0x0632),.driver_info = RTL8812}, /* WD - Cybertan*/
	{USB_DEVICE(0x1740, 0x0100),.driver_info = RTL8812}, /* EnGenius - EnGenius */
	{USB_DEVICE(0x2019, 0xAB30),.driver_info = RTL8812}, /* Planex - Abocom */
	{USB_DEVICE(0x07B8, 0x8812),.driver_info = RTL8812}, /* Abocom - Abocom */
	{USB_DEVICE(0x2001, 0x3315),.driver_info = RTL8812}, /* D-Link - Cameo */
	{USB_DEVICE(0x2001, 0x3316),.driver_info = RTL8812}, /* D-Link - Cameo */
	{USB_DEVICE(0x20F4, 0x805B),.driver_info = RTL8812}, /* TRENDnet - Cameo */
	{USB_DEVICE(0x13B1, 0x003F),.driver_info = RTL8812}, /* Linksys - SerComm */
	{USB_DEVICE(0x2357, 0x0101),.driver_info = RTL8812}, /* TP-Link - T4U */
	{USB_DEVICE(0x0BDA, 0x8812),.driver_info = RTL8812}, /* Alfa AWUS036AC */

	/* RTL8821AU */
        /*=== Realtek demoboard ===*/
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x0811),.driver_info = RTL8821},/* Default ID */
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x0821),.driver_info = RTL8821},/* Default ID */
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8822),.driver_info = RTL8821},/* Default ID */
	/*=== Customer ID ===*/
	{USB_DEVICE(0x7392, 0xA811),.driver_info = RTL8821}, /* Edimax - Edimax */
	{USB_DEVICE(0x7392, 0xA812),.driver_info = RTL8821}, /* Edimax - Edimax */
	{USB_DEVICE(0x2001, 0x3314),.driver_info = RTL8821}, /* D-Link - Cameo */
	{USB_DEVICE(0x0846, 0x9052),.driver_info = RTL8821}, /* Netgear - A6100 */
	{USB_DEVICE(0x0411, 0x0242),.driver_info = RTL8821}, /* BUFFALO - Edimax */
	{}	/* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, rtw_usb_id_tbl);

static int rtl8821au_probe(struct usb_interface *pusb_intf, const struct usb_device_id *pdid)
{
	return rtw_usb_probe(pusb_intf, pdid, &rtl8821au_hal_cfg);
}


static struct usb_driver rtl8821au_usb_drv = {
	.name = "rtl8821au",
	.probe = rtl8821au_probe,
	.disconnect = rtw_usb_disconnect,
	.id_table = rtw_usb_id_tbl,
#if 0
	.suspend =  rtl8821au_suspend,
	.resume = rtl8821au_resume,
  	.reset_resume   = rtl8821au_resume,
#endif
#ifdef CONFIG_AUTOSUSPEND
	.supports_autosuspend = 1,
#endif

};

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Realtek Wireless Lan Driver");
MODULE_AUTHOR("Hans Ulli Kroll <ulli.kroll@googlemail.com>");
MODULE_VERSION("git. based on v4.2.2_7502.20130517");

module_param_named(swenc, rtl8821au_mod_params.sw_crypto, bool, 0444);
module_param_named(ips, rtl8821au_mod_params.inactiveps, bool, 0444);

MODULE_PARM_DESC(swenc, "Set to 1 for software crypto (default 0)\n");
MODULE_PARM_DESC(ips, "Set to 0 to not use link power save (default 1)\n");

module_usb_driver(rtl8821au_usb_drv)
