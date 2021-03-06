<?php
/* $Id$ */
/*
	load_balancer_pool.php
	part of pfSense (http://www.pfsense.com/)

	Copyright (C) 2005-2008 Bill Marquette <bill.marquette@gmail.com>.
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
##|*IDENT=page-loadbalancer-pool
##|*NAME=Load Balancer: Pool page
##|*DESCR=Allow access to the 'Load Balancer: Pool' page.
##|*MATCH=load_balancer_pool.php*
##|-PRIV


require("guiconfig.inc");

if (!is_array($config['load_balancer']['lbpool'])) {
	$config['load_balancer']['lbpool'] = array();
}
$a_pool = &$config['load_balancer']['lbpool'];


if ($_POST) {
	$pconfig = $_POST;

	if ($_POST['apply']) {
		$retval = 0;

		config_lock();
		$retval |= filter_configure();
		$retval |= relayd_configure();
		config_unlock();

		$savemsg = get_std_save_message($retval);
		unlink_if_exists($d_vsconfdirty_path);
	}
}

if ($_GET['act'] == "del") {
	if ($a_pool[$_GET['id']]) {
		/* make sure no virtual servers reference this entry */
		if (is_array($config['load_balancer']['virtual_server'])) {
			foreach ($config['load_balancer']['virtual_server'] as $vs) {
				if ($vs['pool'] == $a_pool[$_GET['id']]['name']) {
					$input_errors[] = "This entry cannot be deleted because it is still referenced by at least one virtual server.";
					break;
				}
			}
		}

		if (!$input_errors) {
			unset($a_pool[$_GET['id']]);
			write_config();
			touch($d_vsconfdirty_path);
			header("Location: load_balancer_pool.php");
			exit;
		}
	}
}

/* Index monitor_type array for easy hyperlinking */
$mondex = array();
for ($i = 0; isset($config['load_balancer']['monitor_type'][$i]); $i++) {
	$mondex[$config['load_balancer']['monitor_type'][$i]['name']] = $i;
}
for ($i = 0; isset($config['load_balancer']['lbpool'][$i]); $i++) {
	$a_pool[$i]['monitor'] = "<a href=\"/load_balancer_monitor_edit.php?id={$mondex[$a_pool[$i]['monitor']]}\">{$a_pool[$i]['monitor']}</a>";
}

$pgtitle = array("Services", "Load Balancer","Pool");
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<form action="load_balancer_pool.php" method="post">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<?php if (file_exists($d_vsconfdirty_path)): ?><p>
<?php print_info_box_np("The load balancer configuration has been changed.<br>You must apply the changes in order for them to take effect.");?><br>
<?php endif; ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
  <?php
        /* active tabs */
        $tab_array = array();
        $tab_array[] = array("Pools", true, "load_balancer_pool.php");
        $tab_array[] = array("Virtual Servers", false, "load_balancer_virtual_server.php");
        $tab_array[] = array("Monitors", false, "load_balancer_monitor.php");
        $tab_array[] = array("Relay Protocols", false, "load_balancer_relay_protocol.php");
        $tab_array[] = array("Relay Actions", false, "load_balancer_relay_action.php");
        display_top_tabs($tab_array);
  ?>
  </td></tr>
  <tr>
    <td>
	<div id="mainarea">
<?
			$t = new MainTable();
			$t->edit_uri('load_balancer_pool_edit.php');
			$t->my_uri('load_balancer_pool.php');
			$t->add_column('Name','name',10);
			$t->add_column('Servers','servers',15);
			$t->add_column('Port','port',10);
			$t->add_column('Monitor','monitor',15);
			$t->add_column('Description','desc',30);
			$t->add_button('edit');
			$t->add_button('dup');
			$t->add_button('del');
			$t->add_content_array($a_pool);
			$t->display();
?>

	</div>
    </td>
  </tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>
