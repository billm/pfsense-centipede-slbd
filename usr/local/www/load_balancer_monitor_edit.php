<?php
/* $Id$ */
/*
        load_balancer_monitor_edit.php
        part of pfSense (http://www.pfsense.com/)

        Copyright (C) 2008 Bill Marquette <bill.marquette@gmail.com>.
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
##|*IDENT=page-services-loadbalancer-monitor-edit
##|*NAME=Services: Load Balancer: Monitor: Edit page
##|*DESCR=Allow access to the 'Services: Load Balancer: Monitor: Edit' page.
##|*MATCH=load_balancer_monitor_edit.php*
##|-PRIV

require("guiconfig.inc");
if (!is_array($config['load_balancer']['monitor_type'])) {
	$config['load_balancer']['monitor_type'] = array();
}
$a_monitor = &$config['load_balancer']['monitor_type'];

if (isset($_POST['id']))
	$id = $_POST['id'];
else
	$id = $_GET['id'];

if (isset($id) && $a_monitor[$id]) {
	$pconfig['name'] = $a_monitor[$id]['name'];
	$pconfig['type'] = $a_monitor[$id]['type'];
	$pconfig['desc'] = $a_monitor[$id]['desc'];
	$pconfig['options'] = array();
	$pconfig['options'] = $a_monitor[$id]['options'];
} else {
	/* Some sane page defaults */
	$pconfig['options']['path'] = '/';
	$pconfig['options']['code'] = 200;
}

$changedesc = "Load Balancer: Monitor: ";
$changecount = 0;

if ($_POST) {
	$changecount++;

	unset($input_errors);
	$pconfig = $_POST;

	/* turn $_POST['http_options_*'] into $pconfig['options'][*] */
	foreach($_POST as $key => $val) {
		if (stristr($key, 'options') !== false) {
			if (stristr($key, $pconfig['type'].'_') !== false) {
				$opt = explode('_',$key);
				$pconfig['options'][$opt[2]] = $val;
			}
			unset($pconfig[$key]);
		}
	}

	/* input validation */
	$reqdfields = explode(" ", "name type desc");
	$reqdfieldsn = explode(",", "Name,Type,Description");

	do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);

	/* Ensure that our monitor names are unique */
	for ($i=0; isset($config['load_balancer']['monitor_type'][$i]); $i++)
		if (($_POST['name'] == $config['load_balancer']['monitor_type'][$i]['name']) && ($i != $id))
			$input_errors[] = "This monitor name has already been used.  Monitor names must be unique.";

	switch($_POST['type']) {
		case 'icmp': {
			break;
		}
		case 'tcp': {
			break;
		}
		case 'http':
		case 'https': {
			if (is_array($pconfig['options'])) {
				if (isset($pconfig['options']['host']) && $pconfig['options']['host'] != "") {
					if (!is_hostname($pconfig['options']['host'])) {
						$input_errors[] = "Invalid hostname.";
					}
				}
				if (isset($pconfig['options']['code']) && $pconfig['options']['code'] != "") {
					// Check code
					if(!is_rfc2616_code($pconfig['options']['code'])) {
						$input_errors[] = "HTTP(s) codes must be from RFC2616.";
					}
				}
				if (!isset($pconfig['options']['path']) || $pconfig['options']['path'] == "") {
					$input_errors[] = "The path to monitor must be set.";
				}
			}
			break;
		}
		case 'send': {
			if (is_array($pconfig['options'])) {
				if (isset($pconfig['options']['send']) && $pconfig['options']['send'] != "") {
					// Check send
				}
				if (isset($pconfig['options']['expect']) && $pconfig['options']['expect'] != "") {
					// Check expect
				}
			}
			break;
		}
	}

	if (!$input_errors) {
		$monent = array();
		if(isset($id) && $a_monitor[$id])
			$monent = $a_monitor[$id];
		if($monent['name'] != "")
			$changedesc .= " modified '{$monent['name']}' monitor:";
		
		update_if_changed("name", $monent['name'], $pconfig['name']);
		update_if_changed("type", $monent['type'], $pconfig['type']);
		update_if_changed("description", $monent['desc'], $pconfig['desc']);
		if($pconfig['type'] == "http" || $pconfig['type'] == "https" ) {
			/* log updates, then clear array and reassign - dumb, but easiest way to have a clear array */
			update_if_changed("path", $monent['options']['path'], $pconfig['options']['path']);
			update_if_changed("host", $monent['options']['host'], $pconfig['options']['host']);
			update_if_changed("code", $monent['options']['code'], $pconfig['options']['code']);
			$monent['options'] = array();
			$monent['options']['path'] = $pconfig['options']['path'];
			$monent['options']['host'] = $pconfig['options']['host'];
			$monent['options']['code'] = $pconfig['options']['code'];
		}
		if($pconfig['type'] == "send" ) {
			/* log updates, then clear array and reassign - dumb, but easiest way to have a clear array */
			update_if_changed("send", $monent['options']['send'], $pconfig['options']['send']);
			update_if_changed("expect", $monent['options']['expect'], $pconfig['options']['expect']);
			$monent['options'] = array();
			$monent['options']['send'] = $pconfig['options']['send'];
			$monent['options']['expect'] = $pconfig['options']['expect'];
		}
		if($pconfig['type'] == "tcp" || $pconfig['type'] == "icmp") {
			$monent['options'] = array();
		}

		if (isset($id) && $a_monitor[$id]) {
			/* modify all pools with this name */
			for ($i = 0; isset($config['load_balancer']['lbpool'][$i]); $i++) {
				if ($config['load_balancer']['lbpool'][$i]['monitor'] == $a_monitor[$id]['name'])
					$config['load_balancer']['lbpool'][$i]['monitor'] = $monent['name'];
			}
			$a_monitor[$id] = $monent;
		} else
			$a_monitor[] = $monent;
		
		if ($changecount > 0) {
			/* Mark config dirty */
			conf_mount_rw();
			touch($d_vsconfdirty_path);
			write_config($changedesc);
		}

		header("Location: load_balancer_monitor.php");
		exit;
	}
}

$pgtitle = array("Services", "Load Balancer","Monitor","Edit");
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<!-- <script type="text/javascript" language="javascript" src="mon.js"></script> -->

<script src="/javascript/scriptaculous/prototype.js" type="text/javascript"></script>
<script src="/javascript/scriptaculous/scriptaculous.js" type="text/javascript"></script>

<?php
	$types = array("icmp" => "ICMP", "tcp" => "TCP", "http" => "HTTP", "https" => "HTTPS", "send" => "Send/Expect");
?>

<script language="javascript">
function updateType(t){
	switch(t) {
<?php
	/* OK, so this is sick using php to generate javascript, but it needed to be done */
	foreach ($types as $key => $val) {
		echo "		case \"{$key}\": {\n";
		$t = $types;
		foreach ($t as $k => $v) {
			if ($k != $key) {
				echo "			$('{$k}').hide();\n";
			}
		}
		echo "		}\n";
	}
?>
	}
	$(t).appear();	
}
</script>

<?php include("fbegin.inc"); ?>
<?php if ($input_errors) print_input_errors($input_errors); ?>

	<form action="load_balancer_monitor_edit.php" method="post" name="iform" id="iform">
	<table width="100%" border="0" cellpadding="6" cellspacing="0">
 		<tr>
			<td colspan="2" valign="top" class="listtopic">Edit Load Balancer - Monitor entry</td>
                </tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Name</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="name" type="text" <?if(isset($pconfig['name'])) echo "value=\"{$pconfig['name']}\"";?> size="16" maxlength="16">
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Type</td>
			<td width="78%" class="vtable" colspan="2">
				<select id="type" name="type">
<?
	foreach ($types as $key => $val) {
		if(isset($pconfig['type']) && $pconfig['type'] == $key) {
			$selected = " selected";
		} else {
			$selected = "";
		}
		echo "<option value=\"{$key}\" onclick=\"updateType('{$key}');\"{$selected}>{$val}</option>\n";
	}
?>
				</select>
			</td>
		</tr>
		<tr align="left" id="icmp"<?= $pconfig['type'] == "icmp" ? "" : " style=\"display:none;\""?>>
		</tr>
		<tr align="left" id="tcp"<?= $pconfig['type'] == "tcp" ? "" : " style=\"display:none;\""?>>
		</tr>
		<tr align="left" id="http"<?= $pconfig['type'] == "http" ? "" : " style=\"display:none;\""?>>
			<td width="22%" valign="top" class="vncellreq">HTTP</td>
			<td width="78%" class="vtable" colspan="2">
				<table width="100%" border="0" cellpadding="6" cellspacing="0">
					<tr align="left">
						<td width="22%" valign="top" class="vncellreq">Path</td>
						<td width="78%" class="vtable" colspan="2">
							<input name="http_options_path" type="text" <?if(isset($pconfig['options']['path'])) echo "value=\"{$pconfig['options']['path']}\"";?>size="64">
						</td>
					</tr>
					<tr align="left">
						<td width="22%" valign="top" class="vncell">Host</td>
						<td width="78%" class="vtable" colspan="2">
							<input name="http_options_host" type="text" <?if(isset($pconfig['options']['host'])) echo "value=\"{$pconfig['options']['host']}\"";?>size="64"><br/>Hostname for Host: header if needed.
						</td>
					</td>
					<tr align="left">
						<td width="22%" valign="top" class="vncellreq">HTTP Code</td>
						<td width="78%" class="vtable" colspan="2">
							<?= print_rfc2616_select("http_options_code", $pconfig['options']['code']); ?>
						</td>
					</tr>
<!-- BILLM: XXX not supported digest checking just yet
					<tr align="left">
						<td width="22%" valign="top" class="vncell">MD5 Page Digest</td>
						<td width="78%" class="vtable" colspan="2">
							<input name="digest" type="text" <?if(isset($pconfig['digest'])) echo "value=\"{$pconfig['digest']}\"";?>size="32"><br /><b>TODO: add fetch functionality here</b>
						</td>
					</tr>
-->
				</table>
			</td>
		</tr>
		<tr align="left" id="https"<?= $pconfig['type'] == "https" ? "" : " style=\"display:none;\""?>>
			<td width="22%" valign="top" class="vncellreq">HTTPS</td>
			<td width="78%" class="vtable" colspan="2">
				<table width="100%" border="0" cellpadding="6" cellspacing="0">
					<tr align="left">
						<td width="22%" valign="top" class="vncellreq">Path</td>
						<td width="78%" class="vtable" colspan="2">
							<input name="https_options_path" type="text" <?if(isset($pconfig['options']['path'])) echo "value=\"{$pconfig['options']['path']}\"";?>size="64">
						</td>
					</tr>
					<tr align="left">
						<td width="22%" valign="top" class="vncellreq">Host</td>
						<td width="78%" class="vtable" colspan="2">
							<input name="https_options_host" type="text" <?if(isset($pconfig['options']['host'])) echo "value=\"{$pconfig['options']['host']}\"";?>size="64"><br/>Hostname for Host: header if needed.
						</td>
					</td>
					<tr align="left">
						<td width="22%" valign="top" class="vncellreq">HTTP Code</td>
						<td width="78%" class="vtable" colspan="2">
							<?= print_rfc2616_select("https_options_code", $pconfig['options']['code']); ?>
						</td>
					</tr>
<!-- BILLM: XXX not supported digest checking just yet

					<tr align="left">
						<td width="22%" valign="top" class="vncellreq">MD5 Page Digest</td>
						<td width="78%" class="vtable" colspan="2">
							<input name="digest" type="text" <?if(isset($pconfig['digest'])) echo "value=\"{$pconfig['digest']}\"";?>size="32"><br /><b>TODO: add fetch functionality here</b>
						</td>
					</tr>
-->
				</table>
			</td>
		</tr>
		<tr align="left" id="send"<?= $pconfig['type'] == "send" ? "" : " style=\"display:none;\""?>>
			<td width="22%" valign="top" class="vncellreq">Send/Expect</td>
			<td width="78%" class="vtable" colspan="2">
				<table width="100%" border="0" cellpadding="6" cellspacing="0">
					<tr align="left">
						<td width="22%" valign="top" class="vncellreq">Send string</td>
						<td width="78%" class="vtable" colspan="2">
							<input name="send_options_send" type="text" <?if(isset($pconfig['options']['send'])) echo "value=\"{$pconfig['options']['send']}\"";?>size="64">
						</td>
					</tr>
					<tr align="left">
						<td width="22%" valign="top" class="vncellreq">Expect string</td>
						<td width="78%" class="vtable" colspan="2">
							<input name="send_options_expect" type="text" <?if(isset($pconfig['options']['expect'])) echo "value=\"{$pconfig['options']['expect']}\"";?>size="64">
						</td>
					</tr>
				</table>
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top" class="vncellreq">Description</td>
			<td width="78%" class="vtable" colspan="2">
				<input name="desc" type="text" <?if(isset($pconfig['desc'])) echo "value=\"{$pconfig['desc']}\"";?>size="64">
			</td>
		</tr>
		<tr align="left">
			<td width="22%" valign="top">&nbsp;</td>
			<td width="78%">
				<input name="Submit" type="submit" class="formbtn" value="Save"><input type="button" class="formbtn" value="Cancel" onclick="history.back()">
				<?php if (isset($id) && $a_monitor[$id]): ?>
				<input name="id" type="hidden" value="<?=$id;?>">
				<?php endif; ?>
			</td>
		</tr>
	</table>
	</form>
<br>
<?php include("fend.inc"); ?>
</body>
</html>
