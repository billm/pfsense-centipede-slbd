<?php
/* $Id$ */
/*
	system_gateways.php
	part of pfSense (http://pfsense.com)

	Copyright (C) 2007 Seth Mos <seth.mos@xs4all.nl>.
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
##|*IDENT=page-system-gateways
##|*NAME=System: Gateways page
##|*DESCR=Allow access to the 'System: Gateways' page.
##|*MATCH=system_gateways.php*
##|-PRIV


require("guiconfig.inc");

$a_gateways = return_gateways_array();
$changedesc = "Gateways: ";

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
	}
}

if ($_GET['act'] == "del") {
	if ($a_gateways[$_GET['id']]) {
		/* remove the real entry */
		$realid = $a_gateways[$_GET['id']]['attribute'];
		$a_gateways = &$config['gateways']['gateway_item'];

		$changedesc .= "removed gateway {$realid}";
		unset($a_gateways[$realid]);
		write_config($changedesc);
		touch($d_staticroutesdirty_path);
		header("Location: system_gateways.php");
		exit;
	}
}


$pgtitle = array("System","Gateways");
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<form action="system_gateways.php" method="post">
<input type="hidden" name="y1" value="1">
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php if (file_exists($d_staticroutesdirty_path)): ?><p>
<?php print_info_box_np("The gateway configuration has been changed.<br>You must apply the changes in order for them to take 
effect.");?><br>
<?php endif; ?>
	<table width="100%" border="0" cellpadding="0" cellspacing="0">
		<tr>
		  <td>
<?php
			$tab_array = array();
			$tab_array[0] = array("Gateways", true, "system_gateways.php");
			$tab_array[1] = array("Routes", false, "system_routes.php");
			$tab_array[2] = array("Groups", false, "system_gateway_groups.php");
			display_top_tabs($tab_array);
?>
</td></tr>
 <tr>
   <td>
	<div id="mainarea">
             <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr>
                  <td width="15%" class="listhdrr">Name</td>
                  <td width="15%" class="listhdrr">Interface</td>
                  <td width="20%" class="listhdrr">Gateway</td>
                  <td width="20%" class="listhdrr">Monitor IP</td>
                  <td width="30%" class="listhdr">Description</td>
                  <td width="10%" class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			   <tr>
				<td width="17"></td>
				<td><a href="system_gateways_edit.php"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
			   </tr>
			</table>
		  </td>
		</tr>
			  <?php $i = 0; foreach ($a_gateways as $gateway): ?>
                <tr>
                  <td class="listlr" ondblclick="document.location='system_gateways_edit.php?id=<?=$i;?>';">
                    <?php
			echo strtoupper($gateway['name']);
			if(isset($gateway['defaultgw'])) {
				echo " <strong>(default)<strong>";
			}
			?>
			
                  </td>
                  <td class="listr" ondblclick="document.location='system_gateways_edit.php?id=<?=$i;?>';">
                    <?php
				$iflabels = get_configured_interface_with_descr(false, true);
				echo htmlspecialchars($iflabels[$gateway['interface']]); ?>
                  </td>
                  <td class="listr" ondblclick="document.location='system_gateways_edit.php?id=<?=$i;?>';">
				  <?php
					if(isset($gateway['interfacegateway'])) {
						echo strtoupper($gateway['interface']) . " ";
					} else {
						echo $gateway['gateway'] . " ";
					}
				  ?>
                  </td>
                  <td class="listr" ondblclick="document.location='system_gateways_edit.php?id=<?=$i;?>';">
                    <?=htmlspecialchars($gateway['monitor']);?>&nbsp;
                  </td>
                  <td class="listbg" ondblclick="document.location='system_gateways_edit.php?id=<?=$i;?>';">
                    <?=htmlspecialchars($gateway['descr']);?>&nbsp;
                  </td>
                  <td valign="middle" nowrap class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			   <tr>
				<td><a href="system_gateways_edit.php?id=<?=$i;?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a>
				<td><a href="system_gateways.php?act=del&id=<?=$i;?>" onclick="return confirm('Do you really want to delete this gateway?')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
			   </tr>
			   <tr>
				<td width="17"></td>
				<td><a href="system_gateways_edit.php?dup=<?=$i;?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
			   </tr>
			</table>

		</tr>
			  <?php $i++; endforeach; ?>
                <tr>
                  <td class="list" colspan="5"></td>
                  <td class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			   <tr>
				<td width="17"></td>
				<td><a href="system_gateways_edit.php"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
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
<?php include("fend.inc"); ?>
</body>
</html>
