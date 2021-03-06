<?php
/* $Id$ */
/*
	system_routes.php
	part of m0n0wall (http://m0n0.ch/wall)

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

##|+PRIV
##|*IDENT=page-system-staticroutes
##|*NAME=System: Static Routes page
##|*DESCR=Allow access to the 'System: Static Routes' page.
##|*MATCH=system_routes.php*
##|-PRIV


require("guiconfig.inc");

if (!is_array($config['staticroutes']['route']))
	$config['staticroutes']['route'] = array();

if (!is_array($config['gateways']['gateway_item']))
	$config['gateways']['gateway_item'] = array();

staticroutes_sort();
$a_routes = &$config['staticroutes']['route'];
$a_gateways = &$config['gateways']['gateway_item'];
$changedesc = "Static Routes: ";

if ($_POST) {

	$pconfig = $_POST;

	if ($_POST['apply']) {

		$retval = 0;

		$retval = system_routing_configure();
		$retval |= filter_configure();
		/* reconfigure our gateway monitor */
		setup_gateways_monitor();

		$savemsg = get_std_save_message($retval);
		if ($retval == 0) {
			if (file_exists($d_staticroutesdirty_path)) {
				config_lock();
				unlink($d_staticroutesdirty_path);
				config_unlock();
			}
		}
	} else {
		if ($_POST['enablefastrouting'] == "") {
			/* Only update config if something changed */
			if (isset($config['staticroutes']['enablefastrouting'])) {
				$changedesc .= " disable fast routing";
				unset($config['staticroutes']['enablefastrouting']);
				write_config($changedesc);
			}
		} else {
			/* Only update config if something changed */
			if (!isset($config['staticroutes']['enablefastrouting'])) {
				$changedesc .= " enable fast routing";
				$config['staticroutes']['enablefastrouting'] = "enabled";
				write_config($changedesc);
			}
		}
	}
}

if ($_GET['act'] == "del") {
	if ($a_routes[$_GET['id']]) {
		$changedesc .= "removed route to " . $a_routes[$_GET['id']['route']];
		unset($a_routes[$_GET['id']]);
		write_config($changedesc);
		touch($d_staticroutesdirty_path);
		header("Location: system_routes.php");
		exit;
	}
}

$pgtitle = array("System","Static Routes");
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<form action="system_routes.php" method="post">
<input type="hidden" name="y1" value="1">
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php if (file_exists($d_staticroutesdirty_path)): ?><p>
<?php print_info_box_np("The static route configuration has been changed.<br>You must apply the changes in order for them to take effect.");?><br>
<?php endif; ?>

	     <?php if($config['system']['disablefilter'] <> "") :?>
	       <table width="100%" border="0" cellpadding="0" cellspacing="0">

		<tr><td width="2%"><input type="checkbox" name="enablefastrouting" id="enablefastrouting" <?php if($config['staticroutes']['enablefastrouting'] == "enabled") echo " checked"; ?>></td><td><b>Enable fast routing</td></tr>

		<tr><td colspan=2><hr><input type="submit" value="Save"></td></tr>
	       </table><br>
	     <?php endif; ?>

              <table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
		  <td>
<?php
		$tab_array = array();
		$tab_array[0] = array("Gateways", false, "system_gateways.php");
		$tab_array[1] = array("Routes", true, "system_routes.php");
		$tab_array[2] = array("Groups", false, "system_gateway_groups.php");
		display_top_tabs($tab_array);
?>
</td></tr>
 <tr>
   <td>
	<div id="mainarea">
             <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr>
                  <td width="25%" class="listhdrr">Network</td>
                  <td width="20%" class="listhdrr">Gateway</td>
                  <td width="15%" class="listhdrr">Interface</td>
                  <td width="30%" class="listhdr">Description</td>
                  <td width="10%" class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			   <tr>
				<td width="17"></td>
				<td><a href="system_routes_edit.php"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
			   </tr>
			</table>
		  </td>
		</tr>
                <tr>
			<?php $i = 0; foreach ($a_routes as $route): ?>
                  <td class="listlr" ondblclick="document.location='system_routes_edit.php?id=<?=$i;?>';">
                    <?=strtolower($route['network']);?>
                  </td>
                  <td class="listr" ondblclick="document.location='system_routes_edit.php?id=<?=$i;?>';">
			<?php
				echo $route['gateway'] . " ";
			?>
                  </td>
                  <td class="listr" ondblclick="document.location='system_routes_edit.php?id=<?=$i;?>';">
			<?php
				foreach($a_gateways as $gateway) {
					if($gateway['name'] == $route['gateway']) {
						echo strtoupper($gateway['interface']) . " ";
					}
				}

			?>
                  </td>
                  <td class="listbg" ondblclick="document.location='system_routes_edit.php?id=<?=$i;?>';">
                    <?=htmlspecialchars($route['descr']);?>&nbsp;
                  </td>
                  <td valign="middle" nowrap class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			   <tr>
				<td><a href="system_routes_edit.php?id=<?=$i;?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a>
				<td><a href="system_routes.php?act=del&id=<?=$i;?>" onclick="return confirm('Do you really want to delete this route?')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
			   </tr>
			   <tr>
				<td width="17"></td>
				<td><a href="system_routes_edit.php?dup=<?=$i;?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
			   </tr>
			</table>
		  </td>
		</tr>
			  <?php $i++; endforeach; ?>
                <tr>
                  <td class="list" colspan="4"></td>
                  <td class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			   <tr>
				<td width="17"></td>
				<td><a href="system_routes_edit.php"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
			   </tr>
		                    </table>
				  </td>
		                </tr>
			</table>
			</div>
			</td>
		  </tr>
		</table>
            </form>
			<p><b>Note:</b>  Do not enter static routes for networks assigned on any interface of this firewall.  Static routes are only used for networks reachable via a different router, and not reachable via your default gateway.</p>
<?php include("fend.inc"); ?>
</body>
</html>
