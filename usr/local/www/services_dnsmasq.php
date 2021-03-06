<?php
/* $Id$ */
/*
	services_dnsmasq.php
	part of m0n0wall (http://m0n0.ch/wall)

	Copyright (C) 2003-2004 Bob Zoller <bob@kludgebox.com> and Manuel Kasper <mk@neon1.net>.
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

##|+PRIV
##|*IDENT=page-services-dnsforwarder
##|*NAME=Services: DNS Forwarder page
##|*DESCR=Allow access to the 'Services: DNS Forwarder' page.
##|*MATCH=services_dnsmasq.php*
##|-PRIV


require("guiconfig.inc");

$pconfig['enable']  = isset($config['dnsmasq']['enable']);
$pconfig['regdhcp'] = isset($config['dnsmasq']['regdhcp']);
$pconfig['regdhcpstatic'] = isset($config['dnsmasq']['regdhcpstatic']);

if (!is_array($config['dnsmasq']['hosts'])) 
	$config['dnsmasq']['hosts'] = array();

if (!is_array($config['dnsmasq']['domainoverrides'])) 
       $config['dnsmasq']['domainoverrides'] = array();

hosts_sort();

$a_hosts 	   = &$config['dnsmasq']['hosts'];
$a_domainOverrides = &$config['dnsmasq']['domainoverrides'];

if ($_POST) {

	$pconfig = $_POST;

	$config['dnsmasq']['enable'] = ($_POST['enable']) ? true : false;
	$config['dnsmasq']['regdhcp'] = ($_POST['regdhcp']) ? true : false;
	$config['dnsmasq']['regdhcpstatic'] = ($_POST['regdhcpstatic']) ? true : false;

	write_config();

	$retval = 0;

	config_lock();
	$retval = services_dnsmasq_configure();
	config_unlock();

	$savemsg = get_std_save_message($retval);

	if ($retval == 0) {
		if (file_exists($d_hostsdirty_path))
			unlink($d_hostsdirty_path);
	}
}

if ($_GET['act'] == "del") {
       if ($_GET['type'] == 'host') {
               if ($a_hosts[$_GET['id']]) {
                       unset($a_hosts[$_GET['id']]);
                       write_config();
                       touch($d_hostsdirty_path);
                       header("Location: services_dnsmasq.php");
                       exit;
               }
       }
       elseif ($_GET['type'] == 'doverride') {
               if ($a_domainOverrides[$_GET['id']]) {
                       unset($a_domainOverrides[$_GET['id']]);
                       write_config();
                       touch($d_hostsdirty_path);
                       header("Location: services_dnsmasq.php");
                       exit;
               }
       }
}

$pgtitle = array("Services","DNS forwarder");
include("head.inc");

?>

<script language="JavaScript">
<!--
function enable_change(enable_over) {
	var endis;
	endis = !(document.iform.enable.checked || enable_over);
	document.iform.regdhcp.disabled = endis;
	document.iform.regdhcpstatic.disabled = endis;
}
//-->
</script>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<form action="services_dnsmasq.php" method="post" name="iform" id="iform">
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php if (file_exists($d_hostsdirty_path)): ?><p>
<?php print_info_box_np("The DNS forwarder configuration has been changed.<br>You must apply the changes in order for them to take effect.");?><br>
<?php endif; ?>
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
                <tr>
                  <td class="vtable"><p>
                      <input name="enable" type="checkbox" id="enable" value="yes" <?php if ($pconfig['enable'] == "yes") echo "checked";?> onClick="enable_change(false)">
                      <strong>Enable DNS forwarder<br>
                      </strong></p></td>
                </tr>
                <tr>
                  <td class="vtable"><p>
                      <input name="regdhcp" type="checkbox" id="regdhcp" value="yes" <?php if ($pconfig['regdhcp'] == "yes") echo "checked";?>>
                      <strong>Register DHCP leases in DNS forwarder<br>
                      </strong>If this option is set, then machines that specify
                      their hostname when requesting a DHCP lease will be registered
                      in the DNS forwarder, so that their name can be resolved.
                      You should also set the domain in <a href="system.php">System:
                      General setup</a> to the proper value.</p>
                    </td>
                </tr>
                <tr>
                  <td class="vtable"><p>
                      <input name="regdhcpstatic" type="checkbox" id="regdhcpstatic" value="yes" <?php if ($pconfig['regdhcpstatic'] == "yes") echo "checked";?>>
                      <strong>Register DHCP static mappings in DNS forwarder<br>
                      </strong>If this option is set, then DHCP static mappings will 
					  be registered in the DNS forwarder, so that their name can be 
					  resolved. You should also set the domain in <a href="system.php">
					  System: General setup</a> to the proper value.</p>
                    </td>
                </tr>				
                <tr>
                  <td> <input name="submit" type="submit" class="formbtn" value="Save" onclick="enable_change(true)">
                  </td>
                </tr>
                <tr>
                  <td><p><span class="vexpl"><span class="red"><strong>Note:<br>
                      </strong></span>If the DNS forwarder is enabled, the DHCP
                      service (if enabled) will automatically serve the LAN IP
                      address as a DNS server to DHCP clients so they will use
                      the forwarder. The DNS forwarder will use the DNS servers
                      entered in <a href="system.php">System: General setup</a>
                      or those obtained via DHCP or PPP on WAN if the &quot;Allow
                      DNS server list to be overridden by DHCP/PPP on WAN&quot;
                      is checked. If you don't use that option (or if you use
                      a static IP address on WAN), you must manually specify at
                      least one DNS server on the <a href="system.php">System:
                      General setup</a> page.<br>
                      <br>
                      You may enter records that override the results from the
                      forwarders below.</span></p></td>
                </tr>
        </table>
        &nbsp;<br>
        <table width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr>
                  <td width="20%" class="listhdrr">Host</td>
                  <td width="25%" class="listhdrr">Domain</td>
                  <td width="20%" class="listhdrr">IP</td>
                  <td width="25%" class="listhdr">Description</td>
                  <td width="10%" class="list">
                    <table border="0" cellspacing="0" cellpadding="1">
                      <tr>
			<td width="17"></td>
                        <td valign="middle"><a href="services_dnsmasq_edit.php"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
                      </tr>
                    </table>
		  </td>
		</tr>
			  <?php $i = 0; foreach ($a_hosts as $hostent): ?>
                <tr>
                  <td class="listlr" ondblclick="document.location='services_dnsmasq_edit.php?id=<?=$i;?>';">
                    <?=strtolower($hostent['host']);?>&nbsp;
                  </td>
                  <td class="listr" ondblclick="document.location='services_dnsmasq_edit.php?id=<?=$i;?>';">
                    <?=strtolower($hostent['domain']);?>&nbsp;
                  </td>
                  <td class="listr" ondblclick="document.location='services_dnsmasq_edit.php?id=<?=$i;?>';">
                    <?=$hostent['ip'];?>&nbsp;
                  </td>
                  <td class="listbg" ondblclick="document.location='services_dnsmasq_edit.php?id=<?=$i;?>';">
                    <?=htmlspecialchars($hostent['descr']);?>&nbsp;
                  </td>
                  <td valign="middle" nowrap class="list">
                    <table border="0" cellspacing="0" cellpadding="1">
                      <tr>
                        <td valign="middle"><a href="services_dnsmasq_edit.php?id=<?=$i;?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
                        <td><a href="services_dnsmasq.php?type=host&act=del&id=<?=$i;?>" onclick="return confirm('Do you really want to delete this host?')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
		      </tr>
                   </table>
                </tr>
		<?php $i++; endforeach; ?>
                <tr>
                  <td class="list" colspan="4"></td>
                  <td class="list">
                    <table border="0" cellspacing="0" cellpadding="1">
                      <tr>
			<td width="17"></td>
                        <td valign="middle"><a href="services_dnsmasq_edit.php"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
                      </tr>
                    </table>
		   </td>
	</table>
<!-- update to enable domain overrides -->
        <table width="100%" border="0" cellpadding="6" cellspacing="0">
	       <tr><td>&nbsp;</td></tr>
               <tr>
                 <td><p>Below you can override an entire domain by specifying an
                        authoritative dns server to be queried for that domain.</p></td>
               </tr>
        </table>
	&nbsp;<br>
        <table width="100%" border="0" cellpadding="0" cellspacing="0">
               <tr>
                 <td width="35%" class="listhdrr">Domain</td>
                 <td width="20%" class="listhdrr">IP</td>
                 <td width="35%" class="listhdr">Description</td>
                 <td width="10%" class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			    <tr>
				<td width="17" heigth="17"></td>			
				<td><a href="services_dnsmasq_domainoverride_edit.php"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
			    </tr>
			</table>
		 </td>
                              </tr>
                        <?php $i = 0; foreach ($a_domainOverrides as $doment): ?>
               <tr>
                 <td class="listlr">
                   <?=strtolower($doment['domain']);?>&nbsp;
                 </td>
                 <td class="listr">
                   <?=$doment['ip'];?>&nbsp;
                 </td>
                 <td class="listbg">
                   <?=htmlspecialchars($doment['descr']);?>&nbsp;
                 </td>
                 <td valign="middle" nowrap class="list"> <a href="services_dnsmasq_domainoverride_edit.php?id=<?=$i;?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a>
                    &nbsp;<a href="services_dnsmasq.php?act=del&type=doverride&id=<?=$i;?>" onclick="return confirm('Do you really want to delete this domain override?')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
                              </tr>
                        <?php $i++; endforeach; ?>
               <tr>
                 <td class="list" colspan="3"></td>
                 <td class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			    <tr>
				<td width="17" heigth="17"></td>			
				<td><a href="services_dnsmasq_domainoverride_edit.php"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
			    </tr>
			</table>
		 </td>
	       </tr>
        </table>
        </form>
<script language="JavaScript">
<!--
enable_change(false);
//-->
</script>
<?php include("fend.inc"); ?>
</body>
</html>
