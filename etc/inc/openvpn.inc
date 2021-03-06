<?php

/* $Id$ */
/*
	$RCSfile$
	
	Copyright (C) 2008 Scott Ullrich <sullrich@gmail.com>
	All rights reserved.
	
	Copyright (C) 2006  Fernando Lemos
	All rights reserved.

	This file was rewritten from scratch by Fernando Lemos but
	*MIGHT* contain code previously written by:

	Copyright (C) 2005 Peter Allgeyer <allgeyer_AT_web.de>
	All rights reserved.

	Copyright (C) 2004 Peter Curran (peter@closeconsultants.com).
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notices,
	   this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	   notices, this list of conditions and the following disclaimer in the
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
/* DISABLE_PHP_LINT_CHECKING */

require_once('config.inc');
require_once('globals.inc');
require_once('pfsense-utils.inc');
require_once('util.inc');

$openvpn_prots = array("UDP", "TCP");

/* 
 * The User Auth mode below is disabled because
 * OpenVPN erroneously requires that we provide
 * a CA configuration parameter. In this mode,
 * clients don't send a certificate so there is
 * no need for a CA. If we require that admins
 * provide one in the pfSense UI due to a bogus
 * requirement imposed by OpenVPN, it could be
 * considered very confusing ( I know I was ).
 *
 * -mgrooms
 */

$openvpn_dh_lengths = array(
	1024, 2048, 4096 );

$openvpn_server_modes = array(
	'p2p_tls' => "Peer to Peer ( SSL/TLS )",
	'p2p_shared_key' => "Peer to Peer ( Shared Key )",
	'server_tls' => "Remote Access ( SSL/TLS )",
//	'server_user' => "Remote Access ( User Auth )",
	'server_tls_user' => "Remote Access ( SSL/TLS + User Auth )");

$openvpn_client_modes = array(
	'p2p_tls' => "Peer to Peer ( SSL/TLS )",
	'p2p_shared_key' => "Peer to Peer ( Shared Key )" );

function openvpn_create_key() {

	$fp = popen("/usr/local/sbin/openvpn --genkey --secret /dev/stdout 2>/dev/null", "r");
	if (!$fp)
		return false;

	$rslt = stream_get_contents($fp);
	pclose($fp);

	return $rslt;
}

function openvpn_create_dhparams($bits) {

	$fp = popen("/usr/bin/openssl dhparam {$bits} 2>/dev/null", "r");
	if (!$fp)
		return false;

	$rslt = stream_get_contents($fp);
	pclose($fp);

	return $rslt;
}

function openvpn_vpnid_used($vpnid) {
	global $config;

	if (is_array($config['openvpn']['openvpn-server']))
		foreach ($config['openvpn']['openvpn-server'] as & $settings)
			if ($vpnid == $settings['vpnid'])
				return true;

	if (is_array($config['openvpn']['openvpn-client']))
		foreach ($config['openvpn']['openvpn-client'] as & $settings)
			if ($vpnid == $settings['vpnid'])
				return true;

	return false;
}

function openvpn_vpnid_next() {

	$vpnid = 1;
	while(openvpn_vpnid_used($vpnid))
		$vpnid++;

	return $vpnid;
}

function openvpn_port_used($prot, $port) {
	global $config;

	if (is_array($config['openvpn']['openvpn-server']))
		foreach ($config['openvpn']['openvpn-server'] as & $settings)
			if ($port == $settings['local_port'] &&
				$prot == $settings['protocol'])
				return $settings['vpnid'];

	if (is_array($config['openvpn']['openvpn-client']))
		foreach ($config['openvpn']['openvpn-client'] as & $settings)
			if ($port == $settings['local_port'] &&
				$prot == $settings['protocol'])
				return $settings['vpnid'];

	return 0;
}

function openvpn_port_next($prot) {

	$port = 1194;
	while(openvpn_port_used($prot, $port))
		$port++;

	return $port;
}

function openvpn_get_cipherlist() {

	$ciphers = array();
	$cipher_out = shell_exec('openvpn --show-ciphers | grep "default key" | awk \'{print $1, "(" $2 "-" $3 ")";}\'');
	$cipher_lines = explode("\n", trim($cipher_out));
	sort($cipher_lines);
	foreach ($cipher_lines as $line) {
		$words = explode(' ', $line);
		$ciphers[$words[0]] = "{$words[0]} {$words[1]}";
	}

	return $ciphers;
}

function openvpn_validate_host($value, $name) {
	$value = trim($value);
	if (empty($value) || !(is_domain($value) && is_ipaddr($value)))
		return "The field '$name' must contain a valid IP address or domain name.";
	return false;
}

function openvpn_validate_port($value, $name) {
	$value = trim($value);
	if (empty($value) || !(is_numeric($value) && ($value > 0) && ($value < 65535)))
		return "The field '$name' must contain a valid port, ranging from 0 to 65535.";
	return false;
}

function openvpn_validate_cidr($value, $name) {
	$value = trim($value);
	if (!empty($value)) {
		list($ip, $mask) = explode('/', $value);
		if (!is_ipaddr($ip) or !is_numeric($mask) or ($mask > 32) or ($mask < 0))
			return "The field '$name' must contain a valid CIDR range.";
	}
	return false;
}

function openvpn_add_dhcpopts(& $settings, & $conf) {

	if (!empty($settings['dns_domain'])) 
		$conf .= "push \"dhcp-option DOMAIN {$settings['dns_domain']}\"\n";

	if (!empty($settings['dns_server1']))
		$conf .= "push \"dhcp-option DNS {$settings['dns_server1']}\"\n";
	if (!empty($settings['dns_server2']))
		$conf .= "push \"dhcp-option DNS {$settings['dns_server2']}\"\n";
	if (!empty($settings['dns_server3']))
		$conf .= "push \"dhcp-option DNS {$settings['dns_server3']}\"\n";
	if (!empty($settings['dns_server4']))
		$conf .= "push \"dhcp-option DNS {$settings['dns_server4']}\"\n";

	if (!empty($settings['ntp_server1']))
		$conf .= "push \"dhcp-option NTP {$settings['dhcp_ntp']}\"\n";
	if (!empty($settings['ntp_server2']))
		$conf .= "push \"dhcp-option NTP {$settings['dhcp_ntp']}\"\n";

	if ($settings['netbios_enable']) {

		if (!empty($settings['dhcp_nbttype']) && ($settings['dhcp_nbttype'] != 0))
			$conf .= "push \"dhcp-option NBT {$settings['dhcp_nbttype']}\"\n";
		if (!empty($settings['dhcp_nbtscope'])) 
			$conf .= "push \"dhcp-option NBS {$settings['dhcp_nbtscope']}\"\n";

		if (!empty($settings['wins_server1']))
			$conf .= "push \"dhcp-option WINS {$settings['wins_server1']}\"\n";
		if (!empty($settings['wins_server2']))
			$conf .= "push \"dhcp-option WINS {$settings['wins_server2']}\"\n";

		if (!empty($settings['nbdd_server1']))
			$conf .= "push \"dhcp-option NBDD {$settings['nbdd_server1']}\"\n";
	}

	if ($settings['gwredir']) 
		$conf .= "push \"redirect-gateway def1\"\n";
}

function openvpn_add_custom(& $settings, & $conf) {

	if ($settings['custom_options']) {

		$options = explode(';', $settings['custom_options']);

		if (is_array($options)) {
			foreach ($options as $option)
				$conf .= "$option\n";
		} else
			$conf .= "{$settings['custom_options']}\n";
	}
}

function openvpn_add_keyfile(& $data, & $conf, $mode_id, $directive) {
	global $g;

	$fpath = $g['varetc_path']."/openvpn/{$mode_id}.{$directive}";
	file_put_contents($fpath, base64_decode($data));
	chown($fpath, 'nobody');
	chgrp($fpath, 'nobody');

	$conf .= "{$directive} {$fpath}\n";
}

function openvpn_reconfigure($mode,& $settings) {
	global $g, $config;

	if (empty($settings))
		return;
	if ($settings['disable']) 
		return;

	/*
	 * NOTE: Deleting tap devices causes spontaneous reboots. Instead,
	 * we use a vpnid number which is allocated for a particular client
	 * or server configuration. ( see openvpn_vpnid_next() )
	 */

	$vpnid = $settings['vpnid'];
	$mode_id = $mode.$vpnid;

	$tunname = "tun{$vpnid}";
	if ($mode == "server")
		$devname = "ovpns{$vpnid}";
	else
		$devname = "ovpnc{$vpnid}";

	/* is our device already configured */
	if (mwexec("/sbin/ifconfig {$devname}")) {

		/* create the tap device if required */
		if (!file_exists("/dev/{$tunname}"))
			exec("/sbin/ifconfig {$tunname} create");

		/* rename the device */
		mwexec("/sbin/ifconfig {$tunname} name {$devname}");

		/* add the device to the openvpn group */
		mwexec("/sbin/ifconfig {$devname} group openvpn");
	}

	$pfile = $g['varrun_path'] . "/openvpn_{$mode_id}.pid";
	$proto = ($settings['protocol'] == 'UDP' ? 'udp' : "tcp-{$mode}");
	$cipher = $settings['crypto'];

	$interface = $settings['interface'];
	if (!$interface)
		$interface = 'WAN';

	$iface = convert_friendly_interface_to_real_interface_name($interface);
	$lines = explode(' ', trim(shell_exec("ifconfig {$iface} | grep inet | grep -v inet6")));
	$iface_ip = $lines[1];

	$conf  = "dev {$devname}\n";
	$conf .= "dev-type tun\n";
	$conf .= "dev-node /dev/{$tunname}\n";
	$conf .= "writepid {$pfile}\n";
	$conf .= "#user nobody\n";
	$conf .= "#group nobody\n";
	$conf .= "daemon\n";
	$conf .= "keepalive 10 60\n";
	$conf .= "ping-timer-rem\n";
	$conf .= "persist-tun\n";
	$conf .= "persist-key\n";
	$conf .= "proto {$proto}\n";
	$conf .= "cipher {$cipher}\n";
	$conf .= "up /etc/rc.filter_configure\n";
	$conf .= "down /etc/rc.filter_configure\n";
	$conf .= "local {$iface_ip}\n";

	// server specific settings

	if ($mode == 'server') {

		list($ip, $mask) = explode('/', $settings['tunnel_network']);
		$mask = gen_subnet_mask($mask);

		// configure tls modes
		switch($settings['mode']) {
			case 'p2p_tls':
			case 'server_tls':
			case 'server_tls_user':
				$conf .= "tls-server\n";
				break;
		}

		// configure p2p/server modes
		switch($settings['mode']) {
			case 'p2p_tls':
			case 'p2p_shared_key':
				$baselong = ip2long($ip) & ip2long($mask);
				$ip1 = long2ip($baselong + 1);
				$ip2 = long2ip($baselong + 2);
				$conf .= "ifconfig $ip1 $ip2\n";
				break;
			case 'server_tls':
			case 'server_user':
			case 'server_tls_user':
				$conf .= "server {$ip} {$mask}\n";
				$conf .= "client-config-dir {$g['varetc_path']}/openvpn-csc\n";
				break;
		}

		// configure user auth modes
		switch($settings['mode']) {
			case 'server_user':
				$conf .= "client-cert-not-required\n";
			case 'server_tls_user':
				$conf .= "username-as-common-name\n";
				$conf .= "auth-user-pass-verify /etc/inc/openvpn.auth-user.php via-env\n";
				break;
		}

		// The local port to listen on
		$conf .= "lport {$settings['local_port']}\n";

		// The management port to listen on
		$conf .= "management 127.0.0.1 {$settings['local_port']}\n";

		if ($settings['maxclients'])
			$conf .= "max-clients {$settings['maxclients']}\n";

		// Can we push routes
		if ($settings['local_network']) {
			list($ip, $mask) = explode('/', $settings['local_network']);
			$mask = gen_subnet_mask($mask);
			$conf .= "push \"route $ip $mask\"\n";
		}

		// Configure client dhcp options
		switch($settings['mode']) {
			case 'server_tls':
			case 'server_user':
			case 'server_tls_user':
				openvpn_add_dhcpopts($settings, $conf);
				break;
		}
	}

	// client specific settings

	if ($mode == 'client') {

		// configure p2p mode
		switch($settings['mode']) {
			case 'p2p_tls':
				$conf .= "tls-client\n";
			case 'shared_key':
				$conf .= "client\n";
				break;
		}

		// The port we'll listen at
		if ($settings['local_port'])
			$conf .= "lport {$settings['local_port']}\n";
		else
			$conf .= "nobind\n";

		// The remote server
		$conf .= "remote {$settings['server_addr']} {$settings['server_port']}\n";

		if (!empty($settings['use_shaper']))
			$conf .= "shaper {$settings['use_shaper']}\n";

		if (!empty($settings['tunnel_network'])) {
			list($ip, $mask) = explode('/', $settings['tunnel_network']);
			$mask = gen_subnet_mask($mask);
			$baselong = ip2long($ip) & ip2long($mask);
			$ip1 = long2ip($baselong + 1);
			$ip2 = long2ip($baselong + 2);
			$conf .= "ifconfig $ip2 $ip1\n";
		}

		if ($settings['proxy_addr'])
			$conf .= "http-proxy {$settings['proxy_addr']} {$settings['proxy_port']}\n";
	}

	// Add a remote network route if set
	if ($settings['remote_network']) {
		list($ip, $mask) = explode('/', $settings['remote_network']);
		$mask = gen_subnet_mask($mask);
		$conf .= "route $ip $mask\n";
	}

	// Write the settings for the keys
	switch($settings['mode']) {
		case 'p2p_shared_key':
			openvpn_add_keyfile($settings['shared_key'], $conf, $mode_id, "secret");
			break;
		case 'p2p_tls':
		case 'server_tls':
		case 'server_tls_user':
			$ca = lookup_ca($settings['caref']);
			openvpn_add_keyfile($ca['crt'], $conf, $mode_id, "ca");
		case 'server_user':
			$cert = lookup_cert($settings['certref']);
			openvpn_add_keyfile($cert['crt'], $conf, $mode_id, "cert");
			openvpn_add_keyfile($cert['prv'], $conf, $mode_id, "key");
			if ($mode == 'server')
				$conf .= "dh {$g['etc_path']}/dh-parameters.{$settings['dh_length']}\n";
			if ($settings['crl'])
				openvpn_add_keyfile($settings['crl'], $conf, $mode_id, "crl-verify");
			if ($settings['tls'])
				openvpn_add_keyfile($settings['tls'], $conf, $mode_id, "tls-auth");
			break;
	}

	if ($settings['compression'])
		$conf .= "comp-lzo\n";

	if ($settings['passtos'])
		$conf .= "passtos\n";

	if ($settings['resolve_retry'])
		$conf .= "resolv-retry infinite\n";

	if ($settings['dynamic_ip']) {
		$conf .= "persist-remote-ip\n";
		$conf .= "float\n";
	}

	openvpn_add_custom($settings, $conf);

	$fpath = $g['varetc_path']."/openvpn/{$mode_id}.conf";
	file_put_contents($fpath, $conf);
	chown($fpath, 'nobody');
	chgrp($fpath, 'nobody');
}

function openvpn_restart($mode, & $settings) {
	global $g, $config;

	$vpnid = $settings['vpnid'];
	$mode_id = $mode.$vpnid;

	/* kill the process if running */
	$pfile = $g['varrun_path']."/openvpn_{$mode_id}.pid";
	if (file_exists($pfile)) {

		/* read the pid file */
		$pid = rtrim(file_get_contents($pfile));
		unlink($pfile);

		/* send a term signal to the process */
		posix_kill($pid, SIGTERM);

		/* wait until the process exits */
		while(posix_kill($pid, 0))
			usleep(250000);
	}

	if ($settings['disable'])
		return;

	/* start the new process */
	$fpath = $g['varetc_path']."/openvpn/{$mode_id}.conf";
	mwexec_bg("nohup openvpn --config {$fpath}");
	touch("{$g['tmp_path']}/filter_dirty");
}

function openvpn_delete($mode, & $settings) {
	global $g, $config;

	$vpnid = $settings['vpnid'];
	$mode_id = $mode.$vpnid;

	$tunname = "tun{$vpnid}";
	if ($mode == "server")
		$devname = "ovpns{$vpnid}";
	else
		$devname = "ovpnc{$vpnid}";

	/* kill the process if running */
	$pfile = "{$g['varrun_path']}/openvpn_{$mode_id}.pid";
	if (file_exists($pfile)) {

		/* read the pid file */
		$pid = trim(file_get_contents($pfile));
		unlink($pfile);

		/* send a term signal to the process */
		posix_kill($pid, SIGTERM);
	}

	/* remove the device from the openvpn group */
	mwexec("/sbin/ifconfig {$devname} -group openvpn");

	/* restore the original adapter name */
	mwexec("/sbin/ifconfig {$devname} name {$tunname}");

	/* remove the configuration files */
	mwexec("/bin/rm {$g['varetc_path']}/openvpn/{$mode_id}.*");
}

function openvpn_resync_csc(& $settings) {
	global $g, $config;

	$fpath = $g['varetc_path']."/openvpn-csc/".$settings['common_name'];

	if ($settings['disable']) {
		unlink_if_exists($fpath);
		return;
	}

	$conf = '';
	if ($settings['block'])
		$conf .= "disable\n";

	if ($settings['push_reset'])
		$conf .= "push-reset\n";

	if (!empty($settings['tunnel_network'])) {
		list($ip, $mask) = explode('/', $settings['tunnel_network']);
		$baselong = ip2long($ip) & gen_subnet_mask_long($mask);
		$ip1 = long2ip($baselong + 1);
		$ip2 = long2ip($baselong + 2);
		$conf .= "ifconfig-push {$ip1} {$ip2}\n";
	}

	openvpn_add_dhcpopts($settings, $conf);

	if ($settings['gwredir'])
		$conf .= "push \"redirect-gateway def1\"\n";

	openvpn_add_custom($settings, $conf);

	file_put_contents($fpath, $conf);
	chown($fpath, 'nobody');
	chgrp($fpath, 'nobody');
}

function openvpn_delete_csc(& $settings) {
	global $g, $config;

	$fpath = $g['varetc_path']."/openvpn-csc/".$settings['common_name'];
	unlink_if_exists($fpath);
}

// Resync the configuration and restart the VPN
function openvpn_resync($mode, & $settings) {
	openvpn_reconfigure($mode, $settings);
	openvpn_restart($mode, $settings);
}

// Resync and restart all VPNs
function openvpn_resync_all() {
	global $g, $config;

	// delay our setup until the system
	// has a chance to init our paths
	if (!file_exists($g['varetc_path']."/openvpn") ||
		!file_exists($g['varetc_path']."/openvpn-csc"))
		return;

	if (!is_array($config['openvpn']))
		$config['openvpn'] = array();

/*
	if (!$config['openvpn']['dh-parameters']) {
		echo "Configuring OpenVPN Parameters ...\n";
		$dh_parameters = openvpn_create_dhparams(1024);
		$dh_parameters = base64_encode($dh_parameters);
		$config['openvpn']['dh-parameters'] = $dh_parameters;
		write_config("OpenVPN DH parameters");
	}

	$path_ovdh = $g['varetc_path']."/openvpn/dh-parameters";
	if (!file_exists($path_ovdh)) {
		$dh_parameters = $config['openvpn']['dh-parameters'];
		$dh_parameters = base64_decode($dh_parameters);
		file_put_contents($path_ovdh, $dh_parameters);
	}
*/

	if (is_array($config['openvpn']['openvpn-server']))
		foreach ($config['openvpn']['openvpn-server'] as & $settings)
			openvpn_resync('server', $settings);

	if (is_array($config['openvpn']['openvpn-client']))
		foreach ($config['openvpn']['openvpn-client'] as & $settings)
			openvpn_resync('client', $settings);

	if (is_array($config['openvpn']['openvpn-csc']))
		foreach ($config['openvpn']['openvpn-csc'] as & $settings)
			openvpn_resync_csc($settings);

	/* give speedy machines time to settle */
	sleep(5);

	/* reload the filter policy */
	filter_configure();
}

?>
