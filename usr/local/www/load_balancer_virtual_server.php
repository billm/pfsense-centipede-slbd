<?php
/* $Id$ */
/*
	load_balancer_virtual_server.php
	part of pfSense (http://www.pfsense.com/)

	Copyright (C) 2005 Bill Marquette <bill.marquette@gmail.com>.
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

require_once("guiconfig.inc");
require_once("vslb.inc");

if (!is_array($config['load_balancer']['virtual_server'])) {
	$config['load_balancer']['virtual_server'] = array();
}
$a_vs = &$config['load_balancer']['virtual_server'];

if ($_POST) {
	$pconfig = $_POST;

	if ($_POST['apply']) {
		$retval = 0;
		config_lock();
		$retval |= filter_configure();
		$retval |= slbd_configure();
		config_unlock();
		$savemsg = get_std_save_message($retval);
		unlink_if_exists($d_vsconfdirty_path);
	}
}

if ($_GET['act'] == "del") {
	if ($a_vs[$_GET['id']]) {

		if (!$input_errors) {
			unset($a_vs[$_GET['id']]);
			write_config();
			touch($d_vsconfdirty_path);
			header("Location: load_balancer_virtual_server.php");
			exit;
		}
	}
} else {
	/* yuck - IE won't send value attributes for image buttons, while Mozilla does -
	   so we use .x/.y to fine move button clicks instead... */
	unset($movebtn);
	foreach ($_POST as $pn => $pd) {
		if (preg_match("/move_(\d+)_x/", $pn, $matches)) {
			$movebtn = $matches[1];
			break;
		}
	}
	/* move selected rules before this rule */
	if (isset($movebtn) && is_array($_POST['rule']) && count($_POST['rule'])) {
		$a_vs_new = array();

		/* copy all rules < $movebtn and not selected */
		for ($i = 0; $i < $movebtn; $i++) {
			if (!in_array($i, $_POST['rule']))
				$a_vs_new[] = $a_vs[$i];
		}

		/* copy all selected rules */
		for ($i = 0; $i < count($a_vs); $i++) {
			if ($i == $movebtn)
				continue;
			if (in_array($i, $_POST['rule']))
				$a_vs_new[] = $a_vs[$i];
		}

		/* copy $movebtn rule */
		if ($movebtn < count($a_vs))
			$a_vs_new[] = $a_vs[$movebtn];

		/* copy all rules > $movebtn and not selected */
		for ($i = $movebtn+1; $i < count($a_vs); $i++) {
			if (!in_array($i, $_POST['rule']))
				$a_vs_new[] = $a_vs[$i];
		}

		$a_vs = $a_vs_new;
		write_config();
		touch($d_vsconfdirty_path);
		header("Location: load_balancer_virtual_server.php");
		exit;
	}
}
$closehead = false;
$pgtitle = "Services: Load Balancer: Virtual Servers";
include("head.inc");
?>
	<script type="text/javascript" language="javascript" src="row_toggle.js"></script>
</head>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<p class="pgtitle"><?=$pgtitle?></p>
<form action="load_balancer_virtual_server.php" method="post">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php if (file_exists($d_vsconfdirty_path)): ?><p>
<?php print_info_box_np("The virtual server configuration has been changed.<br>You must apply the changes in order for them to take effect.");?><br>
<?php endif; ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
  <?php
        /* active tabs */
        $tab_array = array();
        $tab_array[] = array("Pools", false, "load_balancer_pool.php");
        $tab_array[] = array("Virtual Servers", true, "load_balancer_virtual_server.php");
        display_top_tabs($tab_array);
  ?>
  </td></tr>
  <tr>
    <td>
	<div id="mainarea">
              <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr id="frheader">
                  <td width="3%" class="list">&nbsp;</td>
                  <td width="2%" class="list">&nbsp;</td>
                  <td width="10%" class="listhdrr">Name</td>
                  <td width="15%" class="listhdrr">Server address</td>
                  <td width="5%" class="listhdrr">Interface</td>
                  <td width="10%" class="listhdrr">Port</td>
                  <td width="15%" class="listhdrr">Pool</td>
                  <td width="30%" class="listhdr">Description</td>
                  <td width="10%" class="list">
                    <table border="0" cellspacing="0" cellpadding="1">
                      <tr>
                        <td width="17"></td>
                        <td valign="middle"><a href="load_balancer_virtual_server_edit.php"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
                      </tr>
                    </table>
		  </td>
		</tr>
			  <?php $i = 0; foreach ($a_vs as $vsent): ?>
                <tr valign="top" id="fr<?=$i;?>">
                  <td class="listt"><input type="checkbox" id="frc<?=$i;?>" name="rule[]" value="<?=$i;?>" onClick="fr_bgcolor('<?=$i;?>')" style="margin: 0; padding: 0; width: 15px; height: 15px;"></td>
                  <td class="listt"></td>
                  <td class="listlr" onClick="fr_toggle(<?=$i;?>)" id="frd<?=$i;?>" ondblclick="document.location='load_balancer_virtual_server_edit.php?id=<?=$i;?>';">
                    <?=$vsent['name'];?>
                  </td>
                  <td class="listr" onClick="fr_toggle(<?=$i;?>)" id="frd<?=$i;?>" ondblclick="document.location='load_balancer_virtual_server_edit.php?id=<?=$i;?>';">
                    <?=$vsent['ipaddr'];?>
                  </td>
                  <td class="listr" onClick="fr_toggle(<?=$i;?>)" id="frd<?=$i;?>" ondblclick="document.location='load_balancer_virtual_server_edit.php?id=<?=$i;?>';">
                    <?=$vsent['interface'];?>
                  </td>
                  <td class="listr" onClick="fr_toggle(<?=$i;?>)" id="frd<?=$i;?>" ondblclick="document.location='load_balancer_virtual_server_edit.php?id=<?=$i;?>';">
                    <?=$vsent['port'];?>
                  <td class="listr" onClick="fr_toggle(<?=$i;?>)" id="frd<?=$i;?>" align="center" ondblclick="document.location='load_balancer_virtual_server_edit.php?id=<?=$i;?>';">
                    <?=$vsent['pool'];?>
                  </td>
                  <td class="listbg" onClick="fr_toggle(<?=$i;?>)" id="frd<?=$i;?>" ondblclick="document.location='load_balancer_virtual_server_edit.php?id=<?=$i;?>';">
                    <font color="#FFFFFF"><?=htmlspecialchars($vsent['desc']);?>&nbsp;
                  </td>
                  <td class="list" nowrap>
                    <table border="0" cellspacing="0" cellpadding="1">
                      <tr>
                        <td valign="middle"><input name="move_<?=$i;?>" type="image" src="./themes/<?= $g['theme']; ?>/images/icons/icon_left.gif" width="17" height="17" title="move selected virtual server before this virtual server" onMouseOver="fr_insline(<?=$i;?>, true)" onMouseOut="fr_insline(<?=$i;?>, false)"></td>
                        <td valign="middle"><a href="load_balancer_virtual_server_edit.php?id=<?=$i;?>"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a></td>
                      </tr>
                      <tr>
                        <td valign="middle"><a href="load_balancer_virtual_server.php?act=del&id=<?=$i;?>" onclick="return confirm('Do you really want to delete this entry?')"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
                      </tr>
                    </table>
                  </td>
                </tr>
                <?php $i++; endforeach; ?>
                <tr>
                  <td class="list" colspan="8"></td>
                  <td class="list">
                    <table border="0" cellspacing="0" cellpadding="1">
                      <tr>
                        <?php if ($i == 0): ?><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_left_d.gif" width="17" height="17" title="move selected virtual servers to end" border="0"><?php else: ?><input name="move_<?=$i;?>" type="image" src="./themes/<?= $g['theme']; ?>/images/icons/icon_left.gif" width="17" height="17" title="move selected virtual servers to end" onMouseOver="fr_insline(<?=$i;?>, true)" onMouseOut="fr_insline(<?=$i;?>, false)"><?php endif; ?></td>
                        <td valign="middle"><a href="load_balancer_virtual_server_edit.php"><img src="/themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
                      </tr>
                    </table>
                  </td>
                </tr>
              </table>
	   </div>
	</table>
            </form>
<?php include("fend.inc"); ?>
</body>
</html>
