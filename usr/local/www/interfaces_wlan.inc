<?
/* $Id$ */
/*
	interfaces_wlan.inc
	part of m0n0wall (http://m0n0.ch/wall)

	Copyright (C) 2003-2005 Manuel Kasper <mk@neon1.net>.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice,
	   this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/

function wireless_config_init(&$optcfg) {
	global $pconfig;

	$pconfig['standard'] = $optcfg['wireless']['standard'];
	$pconfig['mode'] = $optcfg['wireless']['mode'];
	$pconfig['protmode'] = $optcfg['wireless']['protmode'];
	$pconfig['ssid'] = $optcfg['wireless']['ssid'];
	$pconfig['channel'] = $optcfg['wireless']['channel'];
	$pconfig['txpower'] = $optcfg['wireless']['txpower'];
	$pconfig['distance'] = $optcfg['wireless']['distance'];
	$pconfig['wme_enable'] = isset($optcfg['wireless']['wme']['enable']);
	$pconfig['pureg_enable'] = isset($optcfg['wireless']['pureg']['enable']);
	$pconfig['apbridge_enable'] = isset($optcfg['wireless']['apbridge']['enable']);
	$pconfig['authmode'] = $optcfg['wireless']['authmode'];
	$pconfig['hidessid_enable'] = isset($optcfg['wireless']['hidessid']['enable']);
	if (is_array($optcfg['wireless']['wpa'])) {
		$pconfig['debug_mode'] = $optcfg['wireless']['wpa']['debug_mode'];
		$pconfig['macaddr_acl'] = $optcfg['wireless']['wpa']['macaddr_acl'];
		$pconfig['mac_acl_enable'] = isset($optcfg['wireless']['wpa']['mac_acl_enable']);
		$pconfig['auth_algs'] = $optcfg['wireless']['wpa']['auth_algs'];
		$pconfig['wpa_mode'] = $optcfg['wireless']['wpa']['wpa_mode'];
		$pconfig['wpa_key_mgmt'] = $optcfg['wireless']['wpa']['wpa_key_mgmt'];
		$pconfig['wpa_pairwise'] = $optcfg['wireless']['wpa']['wpa_pairwise'];
		$pconfig['wpa_group_rekey'] = $optcfg['wireless']['wpa']['wpa_group_rekey'];
		$pconfig['wpa_gmk_rekey'] = $optcfg['wireless']['wpa']['wpa_gmk_rekey'];
		$pconfig['wpa_strict_rekey'] = isset($optcfg['wireless']['wpa']['wpa_strict_rekey']);
		$pconfig['passphrase'] = $optcfg['wireless']['wpa']['passphrase'];
		$pconfig['ieee8021x_enable'] = isset($optcfg['wireless']['wpa']['ieee8021x']['enable']);
		$pconfig['ext_wpa_sw'] = $optcfg['wireless']['wpa']['ext_wpa_sw'];
		$pconfig['wpa_enable'] = isset($optcfg['wireless']['wpa']['enable']);
	}
	$pconfig['wep_enable'] = isset($optcfg['wireless']['wep']['enable']);
	$pconfig['mac_acl'] = $optcfg['wireless']['mac_acl'];

	if (is_array($optcfg['wireless']['wep']) &&
		is_array($optcfg['wireless']['wep']['key'])) {
		$i = 1;
		foreach ($optcfg['wireless']['wep']['key'] as $wepkey) {
			$pconfig['key' . $i] = $wepkey['value'];
			if (isset($wepkey['txkey']))
				$pconfig['txkey'] = $i;
			$i++;
		}
		if (!isset($wepkey['txkey']))
			$pconfig['txkey'] = 1;
	}
}

function wireless_config_post(&$optcfg) {
	global $pconfig;

	$input_errors = "";
	unset($input_errors);

	/* input validation */
	if ($_POST['enable']) {
		$reqdfields = explode(" ", "mode ssid");
		$reqdfieldsn = explode(",", "Mode,SSID");
		do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	}

	/* loop through keys and enforce size */
	for ($i = 1; $i <= 4; $i++) {
		if ($_POST['key' . $i]) {
			/* 64 bit */
			if(strlen($_POST['key' . $i]) == 5)
				continue;
			if(strlen($_POST['key' . $i]) == 10) {
				/* hex key */
				if(stristr($_POST['key' . $i], "0x") == false) {
					$_POST['key' . $i] = "0x" . $_POST['key' . $i];
				}
				continue;
			}
			if(strlen($_POST['key' . $i]) == 12) {
				/* hex key */
				if(stristr($_POST['key' . $i], "0x") == false) {
					$_POST['key' . $i] = "0x" . $_POST['key' . $i];
				}
				continue;
			}
			/* 128 bit */
			if(strlen($_POST['key' . $i]) == 13)
				continue;
			if(strlen($_POST['key' . $i]) == 26) {
				/* hex key */
				if(stristr($_POST['key' . $i], "0x") == false) {
					$_POST['key' . $i] = "0x" . $_POST['key' . $i];
				}
				continue;
			}
			if(strlen($_POST['key' . $i]) == 28)
				continue;
			$input_errors[] =  "Invalid wep key size.   Sizes should be 40 (64) bit keys or 104 (128) bit.";
			break;
		}
	}

	if (!$input_errors) {

		$optcfg['wireless']['standard'] = $_POST['standard'];
		$optcfg['wireless']['mode'] = $_POST['mode'];
		$optcfg['wireless']['protmode'] = $_POST['protmode'];
		$optcfg['wireless']['ssid'] = $_POST['ssid'];
		$optcfg['wireless']['channel'] = $_POST['channel'];
		$optcfg['wireless']['authmode'] = $_POST['authmode'];
		$optcfg['wireless']['txpower'] = $_POST['txpower'];
		$optcfg['wireless']['distance'] = $_POST['distance'];
		if (!is_array($optcfg['wireless']['wpa']))
			$optcfg['wireless']['wpa'] = array();
		$optcfg['wireless']['wpa']['macaddr_acl'] = $_POST['macaddr_acl'];
		$optcfg['wireless']['wpa']['auth_algs'] = $_POST['auth_algs'];
		$optcfg['wireless']['wpa']['wpa_mode'] = $_POST['wpa_mode'];
		$optcfg['wireless']['wpa']['wpa_key_mgmt'] = $_POST['wpa_key_mgmt'];
		$optcfg['wireless']['wpa']['wpa_pairwise'] = $_POST['wpa_pairwise'];
		$optcfg['wireless']['wpa']['wpa_group_rekey'] = $_POST['wpa_group_rekey'];
		$optcfg['wireless']['wpa']['wpa_gmk_rekey'] = $_POST['wpa_gmk_rekey'];
		$optcfg['wireless']['wpa']['passphrase'] = $_POST['passphrase'];
		$optcfg['wireless']['wpa']['ext_wpa_sw'] = $_POST['ext_wpa_sw'];

		if($_POST['hidessid_enable'] == "yes")
			$optcfg['wireless']['hidessid']['enable'] = true;
		else
			unset($optcfg['wireless']['hidessid']['enable']);

		if($_POST['mac_acl_enable'] == "yes")
			$optcfg['wireless']['wpa']['mac_acl_enable'] = true;
		else
			unset($optcfg['wireless']['wpa']['mac_acl_enable']);

		if($_POST['ieee8021x_enable'] == "yes")
			$optcfg['wireless']['wpa']['ieee8021x']['enable'] = true;
		else
			unset($optcfg['wireless']['wpa']['ieee8021x']['enable']);

		if($_POST['wpa_strict_rekey'] == "yes")
			$optcfg['wireless']['wpa']['wpa_strict_rekey'] = true;
		else
			unset($optcfg['wireless']['wpa']['wpa_strict_rekey']);

		if($_POST['debug_mode'] == "yes")
			$optcfg['wireless']['wpa']['debug_mode'] = true;
		else
			unset($optcfg['wireless']['wpa']['debug_mode']);

		if($_POST['wpa_enable'] == "yes")
			$optcfg['wireless']['wpa']['enable'] = $_POST['wpa_enable'] = true;
		else
			unset($optcfg['wireless']['wpa']['enable']);

		if($_POST['wep_enable'] == "yes") {
			if (!is_array($optcfg['wireless']['wep']))
                        	$optcfg['wireless']['wep'] = array();
			$optcfg['wireless']['wep']['enable'] = $_POST['wep_enable'] = true;
		} else
			unset($optcfg['wireless']['wep']['enable']);


		if($_POST['wme_enable'] == "yes") {
			if (!is_array($optcfg['wireless']['wme']))
                        	$optcfg['wireless']['wme'] = array();
			$optcfg['wireless']['wme']['enable'] = $_POST['wme_enable'] = true;
		} else
			unset($optcfg['wireless']['wme']['enable']);


		if($_POST['pureg_enable'] == "yes") {
			if (!is_array($optcfg['wireless']['pureg']))
                        	$optcfg['wireless']['pureg'] = array();
			$optcfg['wireless']['pureg']['enable'] = $_POST['pureg_enable'] = true;
		} else
			unset($optcfg['wireless']['pureg']['enable']);


		if($_POST['apbridge_enable'] == "yes") {
			if (!is_array($optcfg['wireless']['apbridge']))
                        	$optcfg['wireless']['apbridge'] = array();
			$optcfg['wireless']['apbridge']['enable'] = $_POST['apbridge_enable'] = true;
		} else
			unset($optcfg['wireless']['apbridge']['enable']);


		if($_POST['standard'] == "11a Turbo") {
			if (!is_array($optcfg['wireless']['turbo']))
                        	$optcfg['wireless']['turbo'] = array();
			$optcfg['wireless']['turbo']['enable'] = true;
		} else
			unset($optcfg['wireless']['turbo']['enable']);

		$optcfg['wireless']['wep']['key'] = array();

		for ($i = 1; $i <= 4; $i++) {
			if ($_POST['key' . $i]) {
				$newkey = array();
				$newkey['value'] = $_POST['key' . $i];
				if ($_POST['txkey'] == $i)
					$newkey['txkey'] = true;
				$optcfg['wireless']['wep']['key'][] = $newkey;
			}
		}
	}

	return $input_errors;
}

$curif = convert_friendly_interface_to_real_interface_name($if);
$wl_modes = get_wireless_modes($curif);

function wireless_config_print(&$optcfg) {
	global $pconfig, $wl_modes, $g;
?>

<script language="JavaScript">
	function openwindow(url) {
		var oWin = window.open(url,"pfSensePop","width=620,height=400,top=150,left=150");
		if (oWin==null || typeof(oWin)=="undefined") {
			return false;
		} else {
			return true;
		}
	}
</script>
                <tr>
                  <td colspan="2" valign="top" height="16"></td>
		</tr>

                <tr>
                  <td colspan="2" valign="top" class="listtopic">Wireless configuration</td>
		</tr>
		<tr>
			<td valign="top" class="vncellreq">Standard</td>
			<td class="vtable">
			<select name="standard" class="formselect" id="standard">
				<?php
				foreach($wl_modes as $wl_standard => $wl_channels) {
					PRINT "<option ";
					if ($pconfig['standard'] == "$wl_standard") {
						PRINT "selected ";
					}
					PRINT "value=\"$wl_standard\">802.$wl_standard</option>\n";
				}
				?>
				</select>
			</td>
		</tr>
		<tr>
			<td valign="top" class="vncellreq">Mode</td>
			<td class="vtable">
			<select name="mode" class="formselect" id="mode">
				<option <? if ($pconfig['mode'] == 'bss') echo "selected";?> value="bss">Infrastructure (BSS)</option>
				<option <? if ($pconfig['mode'] == 'adhoc') echo "selected";?> value="adhoc">Ad-hoc (IBSS)</option>
				<option <? if ($pconfig['mode'] == 'hostap') echo "selected";?> value="hostap">Access Point</option>
			</select>
			</td>
		</tr>
		<tr>
			<td valign="top" class="vncellreq">802.11g OFDM Protection Mode</td>
			<td class="vtable">
			<select name="protmode" class="formselect" id="protmode">
				<option <? if ($pconfig['protmode'] == 'off') echo "selected";?> value="off">Protection mode off</option>
				<option <? if ($pconfig['protmode'] == 'cts') echo "selected";?> value="cts">Protection mode CTS to self</option>
				<option <? if ($pconfig['protmode'] == 'rtscts') echo "selected";?> value="rtscts">Protection mode RTS and CTS</option>
			</select>
			<br/>
			For IEEE 802.11g, use the specified technique for protecting OFDM frames in a mixed 11b/11g network.
			<br/>
			</td>
		</tr>
		<tr>
			<td valign="top" class="vncellreq">SSID</td>
			<td class="vtable"><input name="ssid" type="text" class="formfld unknown" id="ssid" size="20" value="<?=htmlspecialchars($pconfig['ssid']);?>">
			</td>
                </tr>
		<tr>
			<td valign="top" class="vncell">802.11g only</td>
			<td class="vtable"><input name="pureg_enable" type="checkbox" value="yes"  class="formfld" id="pureg_enable" <? if ($pconfig['pureg_enable']) echo "checked";?>>
			<br/>When operating as an access point in 802.11g mode allow only 11g-capable stations to associate (11b-only stations are not permitted to associate).
			</td>
		</tr>
		<tr>
			<td valign="top" class="vncell">Allow intra-BSS communication</td>
			<td class="vtable"><input name="apbridge_enable" type="checkbox" value="yes"  class="formfld" id="apbridge_enable" <? if ($pconfig['apbridge_enable']) echo "checked";?>>
			<br/>
			When operating as an access point, enable this if you want to pass packets between wireless clients directly.
			<br/>
			Disabling the internal bridging is useful when traffic is to be processed with packet filtering.
			</td>
		</tr>
		<tr>
			<td valign="top" class="vncell">Enable WME</td>
			<td class="vtable"><input name="wme_enable" type="checkbox" class="formfld" id="wme_enable" value="yes" <? if ($pconfig['wme_enable']) echo "checked";?>>
			<br/>Setting this option will force the card to use WME (wireless QoS).
			</td>
		</tr>
		<tr>
			<td valign="top" class="vncell">Enable Hide SSID</td>
			<td class="vtable"><input name="hidessid_enable" type="checkbox" class="formfld" id="hidessid_enable" value="yes" <? if ($pconfig['hidessid_enable']) echo "checked";?>>
			<br/>
			Setting this option will force the card to NOT broadcast its SSID
			<br/>
			(this might create problems for some clients). </td>
		</tr>
                <tr>
			<td valign="top" class="vncellreq">Transmit power</td>
			<td class="vtable">
			<select name="txpower" class="formselect" id="txpower">
			<?
				for($x = 99; $x > 0; $x--) {
					if($pconfig["txpower"] == $x)
						$SELECTED = " SELECTED";
					else
						$SELECTED = "";
					echo "<option {$SELECTED}>{$x}</option>\n";
				}
			?>
			</select><br/>
			Note: Typically only a few discreet power settings are available and the driver will use the setting closest to the specified value.  Not all adaptors support changing the transmit power setting.
			</td>
		</tr>
                <tr>
			<td valign="top" class="vncellreq">Channel</td>
			<td class="vtable"><select name="channel" class="formselect" id="channel">
				<option <? if ($pconfig['channel'] == 0) echo "selected"; ?> value="0">Auto</option>
				<?php
				foreach($wl_modes as $wl_standard => $wl_channels) {
					if($wl_standard == "11g") { $wl_standard = "11b/g"; }
					foreach($wl_channels as $wl_channel) {
						PRINT "<option ";
						if ($pconfig['channel'] == "$wl_channel") {
							PRINT "selected ";
						}
						PRINT "value=\"$wl_channel\">$wl_standard - $wl_channel</option>\n";
					}
				}
				?>
			</select>
			<br/>
			Note: Not all channels may be supported by your card
		</tr>
		<tr>
			<td valign="top" class="vncell">Distance setting</td>
			<td class="vtable"><input name="distance" type="text" class="formfld unknown" id="distance" size="5" value="<?=htmlspecialchars($pconfig['distance']);?>">
			<br/>
			Note: This field can be used to tune ACK/CTS timers to fit the distance between AP and Client<br/>
			(measured in Meters and works only for Atheros based cards !)
			</td>
		</tr>
		<tr>
			<td valign="top" class="vncell">WEP</td>
			<td class="vtable"> <input name="wep_enable" type="checkbox" id="wep_enable" value="yes" <? if ($pconfig['wep_enable']) echo "checked"; ?>>
			<strong>Enable WEP</strong>
			<table border="0" cellspacing="0" cellpadding="0">
		<tr>
                        <td>&nbsp;</td>
                        <td>&nbsp;</td>
                        <td>&nbsp;TX key&nbsp;</td>
		</tr>
		<tr>
                        <td>Key 1:&nbsp;&nbsp;</td>
                        <td> <input name="key1" type="text" class="formfld unknown" id="key1" size="30" value="<?=htmlspecialchars($pconfig['key1']);?>"></td>
                        <td align="center"> <input name="txkey" type="radio" value="1" <? if ($pconfig['txkey'] == 1) echo "checked";?>></td>
		</tr>
		<tr>
                        <td>Key 2:&nbsp;&nbsp;</td>
                        <td> <input name="key2" type="text" class="formfld unknown" id="key2" size="30" value="<?=htmlspecialchars($pconfig['key2']);?>"></td>
                        <td align="center"> <input name="txkey" type="radio" value="2" <? if ($pconfig['txkey'] == 2) echo "checked";?>></td>
		</tr>
		<tr>
                        <td>Key 3:&nbsp;&nbsp;</td>
                        <td> <input name="key3" type="text" class="formfld unknown" id="key3" size="30" value="<?=htmlspecialchars($pconfig['key3']);?>"></td>
                        <td align="center"> <input name="txkey" type="radio" value="3" <? if ($pconfig['txkey'] == 3) echo "checked";?>></td>
		</tr>
		<tr>
                        <td>Key 4:&nbsp;&nbsp;</td>
                        <td> <input name="key4" type="text" class="formfld unknown" id="key4" size="30" value="<?=htmlspecialchars($pconfig['key4']);?>"></td>
                        <td align="center"> <input name="txkey" type="radio" value="4" <? if ($pconfig['txkey'] == 4) echo "checked";?>></td>
		</tr>
			</table>
			<br/>
			40 (64) bit keys may be entered as 5 ASCII characters or 10 hex digits preceded by '0x'.<br/>
			104 (128) bit keys may be entered as 13 ASCII characters or 26 hex digits preceded by '0x'.
		   	</td>
                </tr>
                <tr>
			<td valign="top" class="vncell">WPA</td>
			<td class="vtable"><input name="wpa_enable" type="checkbox" class="formfld" id="wpa_enable" value="yes" <? if ($pconfig['wpa_enable']) echo "checked"; ?>>
			<strong>Enable WPA</strong>
			<br/><br/>
			<table border="0" cellspacing="0" cellpadding="0">
			<tr>
                        <td>&nbsp;</td>
                        <td>&nbsp;WPA Pre Shared Key&nbsp;</td>
			</tr>
			<tr>
			<td>PSK:&nbsp;&nbsp;</td>
			<td><input name="passphrase" type="text" class="formfld unknown" id="passphrase" size="66" value="<?=htmlspecialchars($pconfig['passphrase']);?>"></td>
			</tr>
			</table>
			<br/>Passphrase must be from 8 to 63 chars.
			</td>
	        </tr>
		<tr>
			<td valign="top" class="vncell">WPA Mode</td>
			<td class="vtable"><select name="wpa_mode" class="formselect" id="wpa_mode">
			<option <? if ($pconfig['wpa_mode'] == '1') echo "selected";?> value="1">WPA</option>
			<option <? if ($pconfig['wpa_mode'] == '2') echo "selected";?> value="2">WPA2</option>
			<option <? if ($pconfig['wpa_mode'] == '3') echo "selected";?> value="3">Both</option>
			</select>
			</td>
		</tr>
		<tr>
			<td valign="top" class="vncell">WPA Key Management Mode</td>
			<td class="vtable"><select name="wpa_key_mgmt" class="formselect" id="wpa_key_mgmt">
			<option <? if ($pconfig['wpa_key_mgmt'] == 'WPA-PSK') echo "selected";?> value="WPA-PSK">Pre Shared Key</option>
			<option <? if ($pconfig['wpa_key_mgmt'] == 'WPA-EAP') echo "selected";?> value="WPA-EAP">Extensible Authentication Protocol</option>
			<option <? if ($pconfig['wpa_key_mgmt'] == 'WPA-PSK WPA-EAP') echo "selected";?> value="WPA-PSK WPA-EAP">Both</option>
 			</select>
			</td>
		</tr>
		<? /*
		<tr>
			<td valign="top" class="vncell">Enable MAC Filtering</td>
			<td class="vtable"><input name="mac_acl_enable" type="checkbox" value="yes" class="formfld" id="mac_acl_enable" <? if ($pconfig['mac_acl_enable']) echo "checked"; ?>>
			Setting this option will enable the use of a mac filterlist to allow/deny association based on mac address
			<br/><br/>
			<select name="macaddr_acl" class="formselect" id="macaddr_acl">
                        <option <? if ($pconfig['macaddr_acl'] == '0') echo "selected";?> value="0">Allow</option>
			<option <? if ($pconfig['macaddr_acl'] == '1') echo "selected";?> value="1">Deny</option>
			<option <? if ($pconfig['macaddr_acl'] == '2') echo "selected";?> value="2">Radius</option>
			</select>
			<br/><br/>
			Setting this to "Allow" will allow all clients in not in deny list, while "Deny" will deny all clients not in allow list.
			Radius will cause allow and deny list to be searched and then query radius.</br>
			</td>
		</tr>
		*/ ?>
		<tr>
			<td valign="top" class="vncell">Authentication</td>
			<td class="vtable"><select name="auth_algs" class="formselect" id="auth_algs">
			<option <? if ($pconfig['auth_algs'] == '1') echo "selected";?> value="1">Open System Authentication</option>
			<option <? if ($pconfig['auth_algs'] == '2') echo "selected";?> value="2">Shared Key Authentication</option>
			<option <? if ($pconfig['auth_algs'] == '3') echo "selected";?> value="3">Both</option>
			</select>
			<br/>Note: Shared Key Authentication requires WEP.</br>
			</td>
		</tr>
		<tr>
			<td valign="top" class="vncell">WPA Pairwise</td>
			<td class="vtable"><select name="wpa_pairwise" class="formselect" id="wpa_pairwise">
			<option <? if ($pconfig['wpa_pairwise'] == 'CCMP TKIP') echo "selected";?> value="CCMP TKIP">Both</option>
			<option <? if ($pconfig['wpa_pairwise'] == 'CCMP') echo "selected";?> value="CCMP">AES</option>
			<option <? if ($pconfig['wpa_pairwise'] == 'TKIP') echo "selected";?> value="TKIP">TKIP</option>
			</select>
			</td>
		</tr>
		<tr>
			<td valign="top" class="vncell">Key Rotation</td>
			<td class="vtable"><input name="wpa_group_rekey" type="text" class="formfld unknown" id="wpa_group_rekey" size="30" value="<? echo $pconfig['wpa_group_rekey'] ? $pconfig['wpa_group_rekey'] : "60";?>">
			<br/>Allowed values are 1-9999 but should not be longer than Master Key Regeneration time.
			</td>
		</tr>
		<tr>
			<td valign="top" class="vncell">Master Key Regeneration</td>
			<td class="vtable"><input name="wpa_gmk_rekey" type="text" class="formfld" id="wpa_gmk_rekey" size="30" value="<? echo $pconfig['wpa_gmk_rekey'] ? $pconfig['wpa_gmk_rekey'] : "3600";?>">
			<br/>Allowed values are 1-9999 but should not be shorter than Key Rotation time.
			</td>
		</tr>
		<tr>
			<td valign="top" class="vncell">Strict Key Regeneration</td>
			<td class="vtable"><input name="wpa_strict_rekey" type="checkbox" value="yes"  class="formfld" id="wpa_strict_rekey" <? if ($pconfig['wpa_strict_rekey']) echo "checked"; ?>>
			<br/>Setting this option will force the AP to rekey whenever a client disassociates.
			</td>
		</tr>
		<tr>
			<td valign="top" class="vncell">Enable IEEE802.1X</td>
			<td class="vtable"><input name="ieee8021x" type="checkbox" value="yes"  class="formfld" id="ieee8021x" <? if ($pconfig['ieee8021x']) echo "checked";?>>
			<br/>Setting this option will enable 802.1x authentication.
			</td>
		</tr>
<? } ?>
