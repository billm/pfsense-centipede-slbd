<?php
/* $Id$ */
/*
	guiconfig.inc
	by Scott Ullrich, Copyright 2004, All rights reserved.
	originally based on of m0n0wall (http://m0n0.ch/wall)

	Copyright (C) 2003-2004 Manuel Kasper <mk@neon1.net>.
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

/* Include authentication routines */
/* THIS MUST BE ABOVE ALL OTHER CODE */
require_once("authgui.inc");

/* make sure nothing is cached */
if (!$omit_nocacheheaders) {
	header("Expires: 0");
	header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
	header("Cache-Control: no-store, no-cache, must-revalidate");
	header("Cache-Control: post-check=0, pre-check=0", false);
	header("Pragma: no-cache");
}

/* parse the configuration and include all configuration functions */
require_once("config.inc");
require_once("functions.inc");

/* Pull in all the gui related display classes) */
foreach (scandir("/usr/local/www/classes/") as $file) {
	if (stristr($file, ".inc") !== false) {
		require_once("classes/{$file}");
	}
}
/*
 *   if user has selected a custom template, use it.
 *   otherwise default to pfsense tempalte
 */
if($config['theme'] <> "")
        $g['theme'] = $config['theme'];
else
        $g['theme'] = "pfsense";

/*
 *  If this device is an apple ipod/iphone
 *  switch the theme to one that works with it.
 */
$apple_ua = array("iPhone","iPod");
foreach($apple_ua as $useragent)
	if(strstr($_SERVER['HTTP_USER_AGENT'], $useragent))
		$g['theme'] = "pfsense";

$d_landirty_path = $g['varrun_path'] . "/lan.conf.dirty";
$d_pppoeuserdirty_path = $g['varrun_path'] . "/vpn-pppoe-users-edit.dirty";
$d_hostsdirty_path = $g['varrun_path'] . "/hosts.dirty";
$d_natconfdirty_path = $g['varrun_path'] . "/nat.conf.dirty";
$d_filterconfdirty_path = $g['varrun_path'] . "/filter.conf.dirty";
$d_ipsecconfdirty_path = $g['varrun_path'] . "/ipsec.conf.dirty";
$d_shaperconfdirty_path = $g['varrun_path'] . "/shaper.conf.dirty";
$d_pptpuserdirty_path = $g['varrun_path'] . "/pptpd.user.dirty";
$d_dnsmasqdirty_path = $g['varrun_path'] . "/dnsmasq.dirty";
$d_staticmapsdirty_path = $g['varrun_path'] . "/staticmaps.dirty";
$d_staticroutesdirty_path = $g['varrun_path'] . "/staticroutes.dirty";
$d_aliasesdirty_path = $g['varrun_path'] . "/aliases.dirty";
$d_proxyarpdirty_path = $g['varrun_path'] . "/proxyarp.dirty";
$d_fwupenabled_path = $g['varrun_path'] . "/fwup.enabled";
$d_firmwarelock_path = $g['varrun_path'] . "/firmware.lock";
$d_sysrebootreqd_path = $g['varrun_path'] . "/sysreboot.reqd";
$d_passthrumacsdirty_path = $g['varrun_path'] . "/passthrumacs.dirty";
$d_allowedipsdirty_path = $g['varrun_path'] . "/allowedips.dirty";
$d_ovpnclidirty_path = $g['varrun_path'] . "/ovpnclient.dirty";
$d_vipconfdirty_path = $g['varrun_path'] . "/vip.conf.dirty";
$d_sysctldirty_path = $g['varrun_path'] . "/sysctl.conf.dirty";
$d_vsconfdirty_path = $g['varrun_path'] . "/vs.conf.dirty";
$d_shaperconfdirty_path = $g['varrun_path'] . "/shaper.conf.dirty";

/* OpenVPN Directories */
$d_ovpnsrvdirty_path = "/tmp/ovpn-srv.dirty";
$d_ovpncrldirty_path = "/tmp/ovpn-crl.dirty";
$d_ovpnclidirty_path = "/tmp/ovpn-cli.dirty";
/* used by progress bar */
$lastseen = "-1";

$navlevelsep = ": ";	/* navigation level separator string */
$mandfldhtml = "";		/* display this before mandatory input fields */
$mandfldhtmlspc = "";	/* same as above, but with spacing */

if (file_exists($d_firmwarelock_path)) {
	if (!$d_isfwfile) {
		header("Location: system_firmware.php");
		exit;
	} else {
		return;
	}
}

$auth_server_types = array(
	'ldap' => "LDAP",
	'radius' => "Radius");

$ldap_urltypes = array(
	'TCP - Standard' => 389,
	'SSL - Encrypted' => 636);

$ldap_scopes = array(
	'one' => "One Level",
	'subtree' => "Entire Subtree");

$ldap_protvers = array(
	2,
	3);

$ldap_templates = array(

	'open' => array(
				'desc' => "OpenLDAP",
				'attr_user' => "cn",
				'attr_group' => "cn",
				'attr_member' => "member"),

	'msad' => array(
				'desc' => "Microsoft AD",
				'attr_user' => "samAccountNAme",
				'attr_group' => "cn",
				'attr_member' => "member"),

	'edir' => array(
				'desc' => "Novell eDirectory",
				'attr_user' => "cn",
				'attr_group' => "cn",
				'attr_member' => "uniqueMember"));

$radius_srvcs = array(
	'both' => "Authentication and Accounting",
	'auth' => "Authentication",
	'acct' => "Accounting");

$netbios_nodetypes = array(
	'0' => "none",
	'1' => "b-node",
	'2' => "p-node",
	'4' => "m-node",
	'5' => "h-node");

/* some well knows ports */
$wkports = array(
	3389 => "MS RDP",
	21 => "FTP",
	22 => "SSH",
	23 => "Telnet",
	25 => "SMTP",
	53 => "DNS",
	69 => "TFTP",
	80 => "HTTP",
	110 => "POP3",
	113 => "IDENT/AUTH",
	119 => "NNTP",
	123 => "NTP",
	137 => "NetBIOS-NS",
	138 => "NetBIOS-DGM",
	139 => "NetBIOS-SSN",
	143 => "IMAP",
	161 => "SNMP",
	162 => "SNMP-Trap",
	389 => "LDAP",
	443 => "HTTPS",
	445 => "MS DS",
	465 => "SMTP/S",
	500 => "isakmp",
	993 => "IMAP/S",
	995 => "POP3/S",
	1194 => "OpenVPN",
	1512 => "MS WINS",
	1701 => "L2TP",
	1723 => "PPTP",
	1755 => "MMS/TCP",
	1812 => "Radius",
	1813 => "Radius-accounting",
	1863 => "MSN",
	3000 => "HBCI",
	3478 => "STUN",
	4500 => "IPsec NAT-T",
	5004 => "RTP",
	5060 => "SIP",
	5190 => "ICQ",
	5900 => "VNC",
	5999 => "CVSup",
	7000 => "MMS/UDP");

$specialnets = array("wanip" => "WAN address", "lanip" => "LAN address", "lan" => "LAN net", "pptp" => "PPTP clients");

$spiflist = get_configured_interface_with_descr(true, true);
foreach ($spiflist as $ifgui => $ifdesc) {
	$specialnets[$ifgui] = $ifdesc . " net";
}

$medias = array("auto" => "autoselect", "100full" => "100BASE-TX full-duplex",
	"100half" => "100BASE-TX half-duplex", "10full" => "10BASE-T full-duplex",
	"10half" => "10BASE-T half-duplex");

/* platforms that support firmware updating */
$fwupplatforms = array('pfSense', 'net45xx', 'net48xx', 'generic-pc', 'embedded', 'wrap');

function do_input_validation($postdata, $reqdfields, $reqdfieldsn, $input_errors) {

	/* check for bad control characters */
	foreach ($postdata as $pn => $pd) {
		if (is_string($pd) && preg_match("/[\\x00-\\x08\\x0b\\x0c\\x0e-\\x1f]/", $pd)) {
			$input_errors[] = "The field '" . $pn . "' contains invalid characters.";
		}
	}

	for ($i = 0; $i < count($reqdfields); $i++) {
		if ($_POST[$reqdfields[$i]] == "") {
			$input_errors[] = "The field '" . $reqdfieldsn[$i] . "' is required.";
		}
	}
}

function print_input_errors($input_errors) {
	global $g;

	print <<<EOF
	<p>
	<table border="0" cellspacing="0" cellpadding="4" width="100%">
	<tr>
		<td class="inputerrorsleft">
			<img src="./themes/{$g['theme']}/images/icons/icon_error.gif">
		</td>
		<td class="inputerrorsright">
			<span class="errmsg"><p>
				The following input errors were detected:
				<ul>
EOF;
		foreach ($input_errors as $ierr) {
			echo "<li>" . htmlspecialchars($ierr) . "</li>";
		}

	print <<<EOF2
				</ul>
			</span>
		</td></tr>
	</table>
	</p>&nbsp;<br>
EOF2;
	
}

function exec_rc_script($scriptname) {

	global $d_sysrebootreqd_path;

	$execoutput = "";
	$retval = "";
	exec($scriptname . " >/dev/null 2>&1", $execoutput, $retval);
	return $retval;
}

function exec_rc_script_async($scriptname) {

	global $d_sysrebootreqd_path;
	$execoutput = "";
	$retval = "";

	exec("nohup " . $scriptname . " >/dev/null 2>&1 &", $execoutput, $retval);
	return $retval;
}

function verify_gzip_file($fname) {

    $returnvar = mwexec("/usr/bin/gzip -t " . escapeshellarg($fname));
	if ($returnvar != 0)
		return 0;
	else
		return 1;
}

function print_info_box_np($msg, $name="apply",$value="Apply changes") {
	global $g;
	
	if(stristr($msg, "apply") == true || stristr($msg, "save") || stristr($msg, "create")) {
		$savebutton = "<td class='infoboxsave'>";
		$savebutton .= "<input name=\"{$name}\" type=\"submit\" class=\"formbtn\" id=\"${name}\" value=\"{$value}\">";
		if($_POST['if']) 
			$savebutton .= "<input type='hidden' name='if' value='{$_POST['if']}'>";
		$savebutton.="</td>";
	}
	$nifty_redbox = "#990000";
	$nifty_blackbox = "#000000";
	
	$themename = $g['theme'];
	
	if(file_exists("/usr/local/www/themes/{$themename}/tabcontrols.php")) {
		$toeval = file_get_contents("/usr/local/www/themes/{$themename}/tabcontrols.php");
		eval($toeval);
	}
	
	if(file_exists("/usr/local/www/themes/{$themename}/infobox.php")) {
		$toeval = file_get_contents("/usr/local/www/themes/{$themename}/infobox.php");
		eval($toeval);
	}	
		
	echo <<<EOFnp
	<table class='infobox'>
		<tr>
			<td>
				<div class='infoboxnp' id='redbox'>
					<table class='infoboxnptable2'>
						<tr>
							<td class='infoboxnptd'>
								&nbsp;&nbsp;&nbsp;<img class='infoboxnpimg' src="/themes/{$g['theme']}/images/icons/icon_exclam.gif" >
							</td>
							<td class='infoboxnptd2'>
								<b>{$msg}</b>
							</td>
							{$savebutton}
						</tr>
					</table>
				</div>
			</td>
		</tr>
	</table>
	<script type="text/javascript">
		NiftyCheck();
		Rounded("div#redbox","all","#FFF","{$nifty_redbox}","smooth");
		Rounded("td#blackbox","all","#FFF","{$nifty_blackbox}","smooth");
	</script>
	<br/>
EOFnp;

}

function print_info_box($msg) {
	echo "<p>";
	print_info_box_np($msg);
	echo "</p>";
}

function get_std_save_message($ok) {
	global $d_sysrebootreqd_path;

	return "The changes have been applied successfully.  You can also <a href='status_filter_reload.php'>monitor</a> the filter reload progress.";
}

function pprint_address($adr) {
	global $specialnets;

	if (isset($adr['any'])) {
		$padr = "*";
	} else if ($adr['network']) {
		if (preg_match("/opt[0-999]ip/", $adr['network'])) {
			$padr = "Interface IP address";
		} else {
			$padr = $specialnets[$adr['network']];
		}
	} else {
		$padr = $adr['address'];
	}

	if (isset($adr['not']))
		$padr = "! " . $padr;

	return $padr;
}

function pprint_port($port) {
	global $wkports;

	$pport = "";

	if (!$port)
		return "*";
	else {
		$srcport = explode("-", $port);
		if ((!$srcport[1]) || ($srcport[0] == $srcport[1])) {
			$pport = $srcport[0];
			if ($wkports[$srcport[0]]) {
				$pport .= " (" . $wkports[$srcport[0]] . ")";
			}
		} else
			$pport .= $srcport[0] . " - " . $srcport[1];
	}

	return $pport;
}

function captiveportal_users_sort() {
	global $g, $config;

	if (!is_array($config['captiveportal']['user']))
                return;

	function cpusercmp($a, $b) {
		return strcasecmp($a['name'], $b['name']);
	}

	usort($config['captiveportal']['user'], "cpusercmp");
}

function admin_groups_sort() {
	global $g, $config;

	if (!is_array($config['system']['group']))
                return;

	function cpusercmp($a, $b) {
		return strcasecmp($a['name'], $b['name']);
	}

	usort($config['system']['group'], "cpusercmp");
}

function admin_users_sort() {
	global $g, $config;

	if (!is_array($config['system']['user']))
                return;

	function cpusercmp($a, $b) {
		return strcasecmp($a['name'], $b['name']);
	}

	usort($config['system']['user'], "cpusercmp");
}

/* sort by interface only, retain the original order of rules that apply to
   the same interface */
function filter_rules_sort() {
	global $config;

	/* mark each rule with the sequence number (to retain the order while sorting) */
	for ($i = 0; isset($config['filter']['rule'][$i]); $i++)
		$config['filter']['rule'][$i]['seq'] = $i;

	function filtercmp($a, $b) {
		if ($a['interface'] == $b['interface'])
			return $a['seq'] - $b['seq'];
		else
			return -strcmp($a['interface'], $b['interface']);
	}

	usort($config['filter']['rule'], "filtercmp");

	/* strip the sequence numbers again */
	for ($i = 0; isset($config['filter']['rule'][$i]); $i++)
		unset($config['filter']['rule'][$i]['seq']);
}

function nat_rules_sort() {
	global $config;

	if (!is_array($config['nat']['rule']))
                return;

	function natcmp($a, $b) {
		if ($a['external-address'] == $b['external-address']) {
			if ($a['protocol'] == $b['protocol']) {
				if ($a['external-port'] == $b['external-port']) {
					return 0;
				} else {
					return ($a['external-port'] - $b['external-port']);
				}
			} else {
				return strcmp($a['protocol'], $b['protocol']);
			}
		} else if (!$a['external-address'])
			return 1;
		else if (!$b['external-address'])
			return -1;
		else
			return ipcmp($a['external-address'], $b['external-address']);
	}

	usort($config['nat']['rule'], "natcmp");
}

function nat_1to1_rules_sort() {
	global $g, $config;

	if (!is_array($config['nat']['onetoone']))
                return;

	function nat1to1cmp($a, $b) {
		return ipcmp($a['external'], $b['external']);
	}

	usort($config['nat']['onetoone'], "nat1to1cmp");
}

function nat_server_rules_sort() {
	global $g, $config;

	if (!is_array($config['nat']['servernat']))
                return;

	function natservercmp($a, $b) {
		return ipcmp($a['ipaddr'], $b['ipaddr']);
	}

	usort($config['nat']['servernat'], "natservercmp");
}

function nat_out_rules_sort() {
	global $g, $config;

	function natoutcmp($a, $b) {
		return strcmp($a['source']['network'], $b['source']['network']);
	}

	usort($config['nat']['advancedoutbound']['rule'], "natoutcmp");
}

function pptpd_users_sort() {
	global $g, $config;

	if (!is_array($config['ppptpd']['user']))
                return;

	function usercmp($a, $b) {
		return strcasecmp($a['name'], $b['name']);
	}

	usort($config['pptpd']['user'], "usercmp");
}

function  l2tp_users_sort()  { 
        global  $g,  $config; 
 
	if (!is_array($config['l2tp']['user']))
		return;

        function  usercmp($a,  $b)  { 
                return  strcasecmp($a['name'],  $b['name']); 
        } 
	 
        usort($config['l2tp']['user'],  "usercmp");  
}

function openvpn_users_sort() {
	global $g, $config;

	if (!is_array($config['openvpn']['user']))
                return;

	function usercmp($a, $b) {
		return strcasecmp($a['name'], $b['name']);
	}

	usort($config['openvpn']['user'], "usercmp");
}

function pppoe_users_sort() {
	global $g, $config;

	if (!is_array($config['pppoe']['user']))
                return;

	function usercmp($a, $b) {
		return strcasecmp($a['name'], $b['name']);
	}

	usort($config['pppoe']['user'], "usercmp");
}

function staticroutes_sort() {
	global $g, $config;

	if (!is_array($config['staticroutes']['route']))
                return;

	function staticroutecmp($a, $b) {
		return strcmp($a['network'], $b['network']);
	}

	usort($config['staticroutes']['route'], "staticroutecmp");
}

function hosts_sort() {
	global $g, $config;

	if (!is_array($config['dnsmasq']['hosts']))
                return;

	function hostcmp($a, $b) {
		return strcasecmp($a['host'], $b['host']);
	}

	usort($config['dnsmasq']['hosts'], "hostcmp");
}

function staticmaps_sort($ifgui) {
	global $g, $config;

	function staticmapcmp($a, $b) {
		return ipcmp($a['ipaddr'], $b['ipaddr']);
	}

	usort($config['dhcpd'][$ifgui]['staticmap'], "staticmapcmp");
}

function aliases_sort() {
	global $g, $config;

	function aliascmp($a, $b) {
		return strcmp($a['name'], $b['name']);
	}

	if($config['aliases'])
		usort($config['aliases']['alias'], "aliascmp");
}

function schedule_sort(){
	global $g, $config;

	if (!is_array($config['schedules']['schedule']))
                return;

	function schedulecmp($a, $b) {
		return strcmp($a['name'], $b['name']);
	}

	usort($config['schedules']['schedule'], "schedulecmp");

}

function ipsec_mobilekey_sort() {
	global $g, $config;

	function mobilekeycmp($a, $b) {
		return strcmp($a['ident'][0], $b['ident'][0]);
	}

	usort($config['ipsec']['mobilekey'], "mobilekeycmp");
}

function proxyarp_sort() {
	global $g, $config;

	function proxyarpcmp($a, $b) {
		if (isset($a['network']))
			list($ast,$asn) = explode("/", $a['network']);
		else if (isset($a['range'])) {
			$ast = $a['range']['from'];
			$asn = 32;
		}
		if (isset($b['network']))
			list($bst,$bsn) = explode("/", $b['network']);
		else if (isset($b['range'])) {
			$bst = $b['range']['from'];
			$bsn = 32;
		}
		if (ipcmp($ast, $bst) == 0)
			return ($asn - $bsn);
		else
			return ipcmp($ast, $bst);
	}

	usort($config['proxyarp']['proxyarpnet'], "proxyarpcmp");
}

function passthrumacs_sort() {
	global $g, $config;

	function passthrumacscmp($a, $b) {
		return strcmp($a['mac'], $b['mac']);
	}

	usort($config['captiveportal']['passthrumac'],"passthrumacscmp");
}

function cpelements_sort() {
	global $g, $config;

	function cpelementscmp($a, $b) {
		return strcasecmp($a['name'], $b['name']);
	}

	usort($config['captiveportal']['element'],"cpelementscmp");
}

function allowedips_sort() {
	global $g, $config;

	function allowedipscmp($a, $b) {
		return strcmp($a['ip'], $b['ip']);
	}

	usort($config['captiveportal']['allowedip'],"allowedipscmp");
}

function wol_sort() {
	global $g, $config;

	function wolcmp($a, $b) {
		return strcmp($a['descr'], $b['descr']);
	}

	usort($config['wol']['wolentry'], "wolcmp");
}

function gentitle($title) {
	global $navlevelsep;
	if(!is_array($title))
		return $title;
	else
		return join($navlevelsep, $title); 
}

function genhtmltitle($title) {
        global $config;
        return gentitle($title);
}

/* update the changedesc and changecount(er) variables */
function update_changedesc($update) {
	global $changedesc;
	global $changecount;

	$changedesc .= " {$update}";
	$changecount++;
}

function dump_clog($logfile, $tail, $withorig = true, $grepfor = "", $grepinvert = "") {
	global $g, $config;
    $sor = isset($config['syslog']['reverse']) ? "-r" : "";
    $logarr = "";
	$grepline = "  ";
	if(is_array($grepfor))
		foreach($grepfor as $agrep)
			$grepline .= " | grep \"$agrep\"";
	if(is_array($grepinvert))
		foreach($grepinvert as $agrep)
			$grepline .= " | grep -v \"$agrep\"";
	if($config['system']['disablesyslogclog']) 
		exec("cat {$logfile}{$grepline} | grep -v \"\033\" | /usr/bin/tail {$sor} -n {$tail}", $logarr);
	else 
		exec("/usr/sbin/clog {$logfile}{$grepline} | grep -v \"CLOG\" | grep -v \"\033\" | /usr/bin/tail {$sor} -n {$tail}", $logarr);
    /* comment out debug code
    echo "<!-- /usr/sbin/clog {$logfile}{$grepline}| grep -v \"CLOG\" | grep -v \"\033\" | /usr/bin/tail {$sor} -n {$tail} -->";
    */
    foreach ($logarr as $logent) {
            $logent = preg_split("/\s+/", $logent, 6);
            echo "<tr valign=\"top\">\n";
            if ($withorig) {
            		$entry_date_time = htmlspecialchars(join(" ", array_slice($logent, 0, 3)));
                    echo "<td class=\"listlr\" nowrap>" . $entry_date_time  . "</td>\n";
                    echo "<td class=\"listr\">" . htmlspecialchars($logent[4] . " " . $logent[5]) . "</td>\n";
            } else {
                    echo "<td class=\"listlr\" colspan=\"2\">" . htmlspecialchars($logent[5]) . "</td>\n";
            }
            echo "</tr>\n";
    }
}

function return_clog($logfile, $tail, $withorig = true, $grepfor = "", $grepinvert = "", $grepreverse = false) {
	global $g, $config;
	$sor = (isset($config['syslog']['reverse']) || $grepreverse) ? "-r" : "";
	$logarr = "";
	$grepline = "  ";
	if(is_array($grepfor))
		foreach($grepfor as $agrep)
			$grepline .= " | grep \"$agrep\"";
	if(is_array($grepinvert))
		foreach($grepinvert as $agrep)
			$grepline .= " | grep -v \"$agrep\"";
	/* comment out debug code
	echo "<!-- /usr/sbin/clog {$logfile}{$grepline}| grep -v \"CLOG\" | grep -v \"\033\" | /usr/bin/tail {$sor} -n {$tail} -->";
	*/
	if($config['system']['disablesyslogclog']) 
		exec("cat {$logfile}{$grepline} | grep -v \"\033\" | /usr/bin/tail {$sor} -n {$tail}", $logarr);
	else 
		exec("/usr/sbin/clog {$logfile}{$grepline} | grep -v \"CLOG\" | grep -v \"\033\" | /usr/bin/tail {$sor} -n {$tail}", $logarr);
	return($logarr);
}

/* Check if variable has changed, update and log if it has
 * returns true if var changed
 * varname = variable name in plain text
 * orig = original value
 * new = new value
 */
function update_if_changed($varname, & $orig, $new) {
	if (is_array($orig) && is_array($new)) {
		$a_diff = array_diff($orig, $new);
		foreach ($a_diff as $diff) {
			update_changedesc("removed {$varname}: \"{$diff}\"");
		}
		$a_diff = array_diff($new, $orig);
		foreach ($a_diff as $diff) {
			update_changedesc("added {$varname}: \"{$diff}\"");
		}
		$orig = $new;
		return true;

	} else {
		if ($orig != $new) {
			update_changedesc("{$varname}: \"{$orig}\" -> \"{$new}\"");
			$orig = $new;
			return true;
		}
	}
	return false;
}

function address_to_pconfig($adr, &$padr, &$pmask, &$pnot, &$pbeginport, &$pendport) {

        if (isset($adr['any']))
                $padr = "any";
        else if ($adr['network'])
                $padr = $adr['network'];
        else if ($adr['address']) {
                list($padr, $pmask) = explode("/", $adr['address']);
                if (!$pmask)
                        $pmask = 32;
        }

        if (isset($adr['not']))
                $pnot = 1;
        else
                $pnot = 0;

        if ($adr['port']) {
                list($pbeginport, $pendport) = explode("-", $adr['port']);
                if (!$pendport)
                        $pendport = $pbeginport;
        } else {
                if(alias_expand($pbeginport) <> "" || alias_expand($pendport) <> "") {
                        /* Item is a port alias */
                } else {
                        $pbeginport = "any";
                        $pendport = "any";
                }
        }
}

function pconfig_to_address(&$adr, $padr, $pmask, $pnot=false, $pbeginport=0, $pendport=0) {

        $adr = array();

        if ($padr == "any")
                $adr['any'] = true;
        else if (is_specialnet($padr))
                $adr['network'] = $padr;
        else {
                $adr['address'] = $padr;
                if ($pmask != 32)
                        $adr['address'] .= "/" . $pmask;
        }

        if ($pnot)
                $adr['not'] = true;
        else
                unset($adr['not']);

        if (($pbeginport != 0) && ($pbeginport != "any")) {
                if ($pbeginport != $pendport)
                        $adr['port'] = $pbeginport . "-" . $pendport;
                else
                        $adr['port'] = $pbeginport;
        }

        if(alias_expand($pbeginport)) {
                $adr['port'] = $pbeginport;
        }
}

function is_specialnet($net) {
        global $specialsrcdst;
		if(!$net) 
			return false;
        if (in_array($net, $specialsrcdst) || strstr($net, "opt"))
                return true;
        else
                return false;
}

function ipsec_ca_sort() {
        global $g, $config;

        function ipseccacmp($a, $b) {
                return strcmp($a['ident'], $b['ident']);
        }

        usort($config['ipsec']['cacert'], "ipseccacmp");
}

//function to create widget tabs when called
function display_widget_tabs(& $tab_array) {	
	echo "<div id='tabs'>";
	$tabscounter = 0;
	foreach ($tab_array as $ta) {
	$dashpos = strpos($ta[2],'-');
	$tabname = $ta[2] . "-tab";
	$tabclass = substr($ta[2],0,$dashpos);
	$tabclass = $tabclass . "-class";
		if ($ta[1] == true) {
			$tabActive = "table-cell";
			$tabNonActive = "none";
		} 
		else {
			$tabActive = "none";
			$tabNonActive = "table-cell";
		}
		echo "<div id='{$ta[2]}-active' class='{$tabclass}-tabactive' style='display:{$tabActive}; background-color:#EEEEEE; color:black;'>";
		echo "<B>&nbsp;&nbsp;&nbsp;{$ta[0]}";
		echo "&nbsp;&nbsp;&nbsp;</B>";
		echo "</div>";
		
		echo "<div id='{$ta[2]}-deactive' class='{$tabclass}-tabdeactive' style='display:{$tabNonActive}; background-color:#777777; color:white; cursor: pointer;' onClick=\"return changeTabDIV('{$ta[2]}')\">";
		echo "<B>&nbsp;&nbsp;&nbsp;{$ta[0]}";
		echo "&nbsp;&nbsp;&nbsp;</B>";
		echo "</div>";
	}
	
	echo "<script type=\"text/javascript\">";
	echo "NiftyCheck();\n";
	echo "Rounded(\"div.{$tabclass}-tabactive\",\"top\",\"#CCCCCC\",\"#EEEEEE\",\"smooth\");\n";
	echo "Rounded(\"div.{$tabclass}-tabdeactive\",\"top\",\"#CCCCCC\",\"#777777\",\"smooth\");\n";
	echo "</script>";
	echo "</div>";
}


// Return inline javascript file or CSS to minimizie 
// request count going back to server.
function outputJavaScriptFileInline($javascript) {
	if(file_exists($javascript)) {
		echo "\n<script type=\"text/javascript\">\n";
		include($javascript);
		echo "\n</script>\n";
	} else {
		echo "\n\n<!-- Could not location file:  {$javascript} -->\n\n";
	}
}



function outputCSSPrintFileInline($css) {
	if(file_exists($css)) {
		echo "\n<style media=\"print\" type=\"text/css\">\n";
		include($css);
		echo "\n</style>\n";
	} else {
		echo "\n\n<!-- Could not location file:  {$css} -->\n\n";
	}
}


function outputCSSFileInline($css) {
	if(file_exists($css)) {
		echo "\n<style type=\"text/css\">\n";
		include($css);
		echo "\n</style>\n";
	} else {
		echo "\n\n<!-- Could not location file:  {$css} -->\n\n";
	}
}

$rfc2616 = array(
	100 => "100 Continue",
	101 => "101 Switching Protocols",
	200 => "200 OK",
	201 => "201 Created",
	202 => "202 Accepted",
	203 => "203 Non-Authoritative Information",
	204 => "204 No Content",
	205 => "205 Reset Content",
	206 => "206 Partial Content",
	300 => "300 Multiple Choices",
	301 => "301 Moved Permanently",
	302 => "302 Found",
	303 => "303 See Other",
	304 => "304 Not Modified",
	305 => "305 Use Proxy",
	306 => "306 (Unused)",
	307 => "307 Temporary Redirect",
	400 => "400 Bad Request",
	401 => "401 Unauthorized",
	402 => "402 Payment Required",
	403 => "403 Forbidden",
	404 => "404 Not Found",
	405 => "405 Method Not Allowed",
	406 => "406 Not Acceptable",
	407 => "407 Proxy Authentication Required",
	408 => "408 Request Timeout",
	409 => "409 Conflict",
	410 => "410 Gone",
	411 => "411 Length Required",
	412 => "412 Precondition Failed",
	413 => "413 Request Entity Too Large",
	414 => "414 Request-URI Too Long",
	415 => "415 Unsupported Media Type",
	416 => "416 Requested Range Not Satisfiable",
	417 => "417 Expectation Failed",
	500 => "500 Internal Server Error",
	501 => "501 Not Implemented",
	502 => "502 Bad Gateway",
	503 => "503 Service Unavailable",
	504 => "504 Gateway Timeout",
	505 => "505 HTTP Version Not Supported"
);

function is_rfc2616_code($code) {
	global $rfc2616;
	if (isset($rfc2616[$code]))
		return true;
	else
		return false;
}

function print_rfc2616_select($tag, $current){
	global $rfc2616;

	/* Default to 200 OK if not set */
	if ($current == "")
		$current = 200;

	echo "<select id=\"{$tag}\" name=\"{$tag}\">\n";	
	foreach($rfc2616 as $code => $message) {
		if ($code == $current) {
			$sel = " selected";
		} else {
			$sel = "";
		}
		echo "<option value=\"{$code}\"{$sel}>{$message}</option>\n";
	}
}

// Useful debugging function, much cleaner than print_r
function echo_array($array,$return_me=false){
    if(is_array($array) == false){
        $return = "The provided variable is not an array.";
    }else{
        foreach($array as $name=>$value){
            if(is_array($value)){
                $return .= "";
                $return .= "['<b>$name</b>'] {<div style='margin-left:10px;'>\n";
                $return .= echo_array($value,true);
                $return .= "</div>}";
                $return .= "\n\n";
            }else{
                if(is_string($value)){
                    $value = "\"$value\"";
                }
                $return .= "['<b>$name</b>'] = $value\n\n";
            }
        }
    }
    if($return_me == true){
        return $return;
    }else{
        echo "<pre>".$return."</pre>";
    }
}

function add_package_tabs($tabgroup, & $tab_array) {
	global $config, $g;

	if(!is_array($config['installedpackages']))
		return;
	if(!is_array($config['installedpackages']['tab']))
		return;

	foreach($config['installedpackages']['tab'] as $tab) {
		if ($tab['group'] !== $group)
			continue;
		$tab_entry = array();
		if($tab['name']) {
			$tab_entry[] = $tab['name'];
			$tab_entry[] = false;
			$tab_entry[] = $tab['url'];
			$tab_array[] = $tab_entry;
		}
	}
}

?>
