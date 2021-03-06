<?php
/*
	ipsec.inc
	Copyright (C) 2007 Scott Ullrich
	Copyright (C) 2008 Shrew Soft Inc
	All rights reserved.

	Parts of this code was originally based on vpn_ipsec_sad.php
	Copyright (C) 2003-2004 Manuel Kasper

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

/* IPsec defines */
$my_identifier_list = array(
	'myaddress' => array( 'desc' => 'My IP address', 'mobile' => true ),
	'address' => array( 'desc' => 'IP address', 'mobile' => true ),
	'fqdn' => array( 'desc' => 'Distinguished name', 'mobile' => true ),
	'user_fqdn' => array( 'desc' => 'User distinguished name', 'mobile' => true ),
	'asn1dn' => array( 'desc' => 'ASN.1 distinguished Name', 'mobile' => true ),
	'keyid tag' => array( 'desc' => 'KeyID tag', 'mobile' => true ),
	'dyn_dns' => array( 'desc' => 'Dynamic DNS', 'mobile' => true ));

$peer_identifier_list = array(
	'peeraddress' => array( 'desc' => 'Peer IP address', 'mobile' => false ),
	'address' => array( 'desc' => 'IP address', 'mobile' => false ),
	'fqdn' => array( 'desc' => 'Distinguished name', 'mobile' => true ),
	'user_fqdn' => array( 'desc' => 'User distinguished name', 'mobile' => true ),
	'asn1dn' => array( 'desc' => 'ASN.1 distinguished Name', 'mobile' => true ),
	'keyid tag' => array( 'desc' =>'KeyID tag', 'mobile' => true ));

$p1_ealgos = array(
	'aes' => array( 'name' => 'AES', 'keysel' => array( 'lo' => 128, 'hi' => 256, 'step' => 64 ) ),
	'blowfish' => array( 'name' => 'Blowfish', 'keysel' => array( 'lo' => 128, 'hi' => 256, 'step' => 8 ) ),
	'3des' => array( 'name' => '3DES' ),
	'cast128' => array( 'name' => 'CAST128' ),
	'des' => array( 'name' => 'DES' ));

$p2_ealgos = array(
	'aes' => array( 'name' => 'AES', 'keysel' => array( 'lo' => 128, 'hi' => 256, 'step' => 64 ) ),
	'blowfish' => array( 'name' => 'Blowfish', 'keysel' => array( 'lo' => 128, 'hi' => 256, 'step' => 8 ) ),
	'3des' => array( 'name' => '3DES' ),
	'cast128' => array( 'name' => 'CAST128' ),
	'des' => array( 'name' => 'DES' ));

$p1_halgos = array(
	'sha1' => 'SHA1',
	'md5' => 'MD5');

$p2_halgos = array(
	'hmac_sha1' => 'SHA1',
	'hmac_md5' => 'MD5');

$p1_authentication_methods = array(
	'hybrid_rsa_server' => array( 'name' => 'Hybrid RSA + Xauth', 'mobile' => true ),
	'xauth_rsa_server' => array( 'name' => 'Mutual RSA + Xauth', 'mobile' => true ),
	'xauth_psk_server' => array( 'name' => 'Mutual PSK + Xauth', 'mobile' => true ),
	'rsasig' => array( 'name' => 'Mutual RSA', 'mobile' => false ),
	'pre_shared_key' => array( 'name' => 'Mutual PSK', 'mobile' => false ) );

$p2_protos = array(
	'esp' => 'ESP',
	'ah' => 'AH');

$p2_pfskeygroups = array(
	'0' => 'off',
	'1' => '1',
	'2' => '2',
	'5' => '5');

/*
 * ikeid management functions
 */

function ipsec_ikeid_used($ikeid) {
	global $config;

	foreach ($config['ipsec']['phase1'] as $ph1ent)
		if( $ikeid == $ph1ent['ikeid'] )
			return true;

	return false;
}

function ipsec_ikeid_next() {

	$ikeid = 1;
	while(ipsec_ikeid_used($ikeid))
		$ikeid++;

	return $ikeid;
}

/*
 * Return phase1 local address
 */
function ipsec_get_phase1_src(& $ph1ent) {

	if ($ph1ent['interface'])
		$if = $ph1ent['interface'];
	else
		$if = "WAN";

	$realinterface = convert_friendly_interface_to_real_interface_name($if);
	$interfaceip = find_interface_ip($realinterface);

	return $interfaceip;
}

/*
 * Return phase1 local address
 */
function ipsec_get_phase1_dst(& $ph1ent) {

	$rg = $ph1ent['remote-gateway'];
	if (!is_ipaddr($rg))
		return resolve_retry($rg);

	if(!is_ipaddr($rg))
		return false;

	return $rg;
}

/*
 * Return phase2 idinfo in cidr format
 */
function ipsec_idinfo_to_cidr(& $idinfo,$addrbits = false) {
	global $config;

	switch ($idinfo['type'])
	{
		case "address":
			if ($addrbits)
				return $idinfo['address']."/32";
			else
				return $idinfo['address'];
		case "network":
			return $idinfo['address']."/".$idinfo['netbits'];
		case "mobile":
			return "0.0.0.0/0";
        default:
			$address = $config['interfaces']['lan']['ipaddr'];
			$netbits = $config['interfaces'][$idinfo['type']]['subnet'];
			$address = gen_subnet($address,$netbits);
			return $address."/".$netbits;
    }
}

/*
 * Return phase2 idinfo in address/netmask format
 */
function ipsec_idinfo_to_subnet(& $idinfo,$addrbits = false) {
	global $config;

	switch ($idinfo['type'])
	{
		case "address":
			if ($addrbits)
				return $idinfo['address']."/255.255.255.255";
			else
				return $idinfo['address'];
		case "network":
			return $idinfo['address']."/".gen_subnet_mask($idinfo['netbits']);
		case "mobile":
			return "0.0.0.0/0";
        default:
			$address = $config['interfaces']['lan']['ipaddr'];
			$netbits = $config['interfaces'][$idinfo['type']]['subnet'];
			$address = gen_subnet($address,$netbits);
			$netbits = gen_subnet_mask($netbits);
			return $address."/".netbits;
    }
}

/*
 *  Return phase2 idinfo in text format
 */
function ipsec_idinfo_to_text(& $idinfo) {

    switch ($idinfo['type'])
    {
        case "address":
            return $idinfo['address'];
        case "network":
            return $idinfo['address']."/".$idinfo['netbits'];
		case "mobile":
			return "Mobile Client";
        default:
            return strtoupper($idinfo['type']);
    }
}

/*
 * Return phase1 association for phase2
 */
function ipsec_lookup_phase1(& $ph2ent,& $ph1ent)
{
    global $config;
    $a_phase1 = $config['ipsec']['phase1'];

    if (is_array($a_phase1) && count($a_phase1)) {
        foreach ($a_phase1 as $ph1tmp) {
            if ($ph1tmp['ikeid'] == $ph2ent['ikeid']) {
                $ph1ent = $ph1tmp;
                return $ph1ent;
            }
        }
    }

    return false;
}

/*
 * Check phase1 communications status
 */
function ipsec_phase1_status(& $ph1ent) {

	$loc_ip = get_ipsec_tunnel_src($ph1ent);
	$rmt_ip = $ph1ent['remote-gateway'];

	if(ipsec_lookup_ipsakmp_sa($loc_ip,$rmt_ip))
		return true;

	return false;
}

/*
 * Check phase2 communications status
 */
function ipsec_phase2_status(& $spd,& $sad,& $ph1ent,& $ph2ent) {

	$loc_ip = ipsec_get_phase1_src($ph1ent);
	$rmt_ip = ipsec_get_phase1_dst($ph1ent);

	$loc_id = ipsec_idinfo_to_cidr($ph2ent['localid'],true);
	$rmt_id = ipsec_idinfo_to_cidr($ph2ent['remoteid'],true);

	/* check for established SA in both directions */
	if( ipsec_lookup_ipsec_sa($spd,$sad,"out",$loc_ip,$rmt_ip,$loc_id,$rmt_id) &&
		ipsec_lookup_ipsec_sa($spd,$sad,"in",$rmt_ip,$loc_ip,$rmt_id,$loc_id))
		return true;

	return false;
}

/*
 * Return ISAKMP SA details
 */
function ipsec_lookup_isakmp_sa($in_srcip,$in_dstip) {
	/* TODO : use racconctl to lookup iskamp SA */
	return NULL;
}

/*
 * Return IPsec SA details
 */
function ipsec_lookup_ipsec_sa(& $spd,& $sad,$dir,$in_srcip,$in_dstip,$in_srcid,$in_dstid) {

	/* match the phase1/2 to an SP */

	foreach($spd as $sp) {

		/* match direction */

		if($dir != $sp['dir'])
			continue;

		/* match IPs */

		if($in_srcip != $sp['src'])
			continue;
		if($in_dstip != $sp['dst'])
			continue;

		/* add netbits for address IDs */

		$sp_srcid = $sp['srcid'];
		$sp_dstid = $sp['dstid'];

		if (!strstr($sp_srcid,"/"))
			$sp_srcid .= '/32';
		if (!strstr($sp_dstid,"/"))
			$sp_dstid .= '/32';

		/* match IDs */

		if($in_srcid != $sp_srcid)
			continue;
		if($in_dstid != $sp_dstid)
			continue;

		/* match the SP to a unique SA by reqid */

		foreach($sad as $sa) {

			/* match REQIDs */

			if($sa[reqid] != $sp[reqid])
				continue;

			/* sanitize for NAT-T ports */

			$sa_srcip = $sa['src'];
			$sa_dstip = $sa['dst'];

			if (strstr($sa_srcip,"["))
				$sa_srcip = substr($sa_srcip,0,strcspn($sa_srcip,"["));
			if (strstr($sa_dstip,"["))
				$sa_dstip = substr($sa_dstip,0,strcspn($sa_dstip,"["));

			/* match IPs */

			if($in_srcip != $sa_srcip)
				continue;
			if($in_dstip != $sa_dstip)
				continue;

			return $sa;
		}
	}

	return NULL;
}

/*
 * Return dump of SPD table
 */
function ipsec_dump_spd()
{
	$fd = @popen("/usr/local/sbin/setkey -DP", "r");
	$spd = array();
	if ($fd) {
		while (!feof($fd)) {
			$line = chop(fgets($fd));
			if (!$line)
				continue;
			if ($line == "No SPD entries.")
				break;
			if ($line[0] != "\t") {
				if (is_array($cursp))
					$spd[] = $cursp;
				$cursp = array();
				$linea = explode(" ", $line);
				$cursp['srcid'] = substr($linea[0], 0, strpos($linea[0], "["));
				$cursp['dstid'] = substr($linea[1], 0, strpos($linea[1], "["));
				$i = 0;
			} else if (is_array($cursp)) {
				$linea = explode(" ", trim($line));
				switch($i)
				{
					case 1:
						if ($linea[1] == "none")	/* don't show default anti-lockout rule */
							unset($cursp);
						else
							$cursp['dir'] = $linea[0];
						break;
					case 2:
						$upperspec = explode("/", $linea[0]);
						$cursp['proto'] = $upperspec[0];
						list($cursp['src'], $cursp['dst']) = explode("-", $upperspec[2]);
						$cursp['reqid'] =  substr($upperspec[3], strpos($upperspec[3], "#")+1);
						break;
				}
			}
			$i++;
		}
		if (is_array($cursp) && count($cursp))
			$spd[] = $cursp;
		pclose($fd);
	}

	return $spd;
}

/*
 * Return dump of SAD table
 */
function ipsec_dump_sad()
{
	$fd = @popen("/usr/local/sbin/setkey -D", "r");
	$sad = array();
	if ($fd) {
		while (!feof($fd)) {
			$line = chop(fgets($fd));
			if (!$line)
				continue;
			if ($line == "No SAD entries.")
				break;
			if ($line[0] != "\t")
			{
				if (is_array($cursa))
					$sad[] = $cursa;
				$cursa = array();
				list($cursa['src'],$cursa['dst']) = explode(" ", $line);
				$i = 0;
			}
			else
			{
				$linea = explode(" ", trim($line));
				switch ($i) {
					case 1:
						$cursa['proto'] = $linea[0];
						$cursa['spi'] = substr($linea[2], strpos($linea[2], "x")+1, -1);
						$reqid = substr($linea[3], strpos($linea[3], "=")+1);
						$cursa['reqid'] = substr($reqid, 0, strcspn($reqid,"("));
						break;
					case 2:
						$cursa['ealgo'] = $linea[1];
						break;
					case 3:
						$cursa['aalgo'] = $linea[1];
						break;
				}
			}
			$i++;
		}
		if (is_array($cursa) && count($cursa))
			$sad[] = $cursa;
		pclose($fd);
	}

	return $sad;
}

?>
