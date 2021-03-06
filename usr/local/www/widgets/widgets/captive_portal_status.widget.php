<?php 
/*
	captive_portal_status.widget.php
	Copyright (C) 2007 Sam Wenham
	All rights reserved.

	status_captiveportal.php
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

require_once("guiconfig.inc");
require_once("pfsense-utils.inc");
require_once("functions.inc");

$concurrent = `cat /var/db/captiveportal.db | wc -l`;

?>

<script src="/javascript/sorttable.js"></script>
<?php

if ($_GET['act'] == "del") {
	captiveportal_disconnect_client($_GET['id']);
}

flush();

function clientcmp($a, $b) {
	global $order;
	return strcmp($a[$order], $b[$order]);
}

$cpdb = array();
captiveportal_lock();
$fp = @fopen("{$g['vardb_path']}/captiveportal.db","r");

if ($fp) {
	while (!feof($fp)) {
		$line = trim(fgets($fp));
		if ($line) {
			$cpent = explode(",", $line);
			if ($_GET['showact'])
				$cpent[4] = captiveportal_get_last_activity($cpent[1]);
			$cpdb[] = $cpent;
		}
	}
	
	fclose($fp);
	
	if ($_GET['order']) {
		if ($_GET['order'] == "ip")
			$order = 2;
		else if ($_GET['order'] == "mac")
			$order = 3;
		else if ($_GET['order'] == "lastact")
			$order = 4;
		else
			$order = 0;
		usort($cpdb, "clientcmp");
	}
}
captiveportal_unlock();
?>
<table class="sortable" name="sortabletable" id="sortabletable" width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr>
    <td class="listhdrr"><a href="?order=ip&showact=<?=$_GET['showact'];?>">IP address</a></td>
    <td class="listhdrr"><a href="?order=mac&showact=<?=$_GET['showact'];?>">MAC address</a></td>
    <td class="listhdrr"><a href="?order=start&showact=<?=$_GET['showact'];?>">Session start</a></td>
  </tr>
<?php foreach ($cpdb as $cpent): ?>
  <tr>
    <td class="listlr"><?=$cpent[2];?></td>
    <td class="listr"><?=$cpent[3];?>&nbsp;</td>
    <td class="listr"><?=htmlspecialchars(date("m/d/Y H:i:s", $cpent[0]));?></td>
	<?php if ($_GET['showact']): ?>
    <td class="listr"><?php if ($cpent[4]) echo htmlspecialchars(date("m/d/Y H:i:s", $cpent[4]));?></td>
	<?php endif; ?>
	<td valign="middle" class="list" nowrap>
	<a href="?order=<?=$_GET['order'];?>&showact=<?=$_GET['showact'];?>&act=del&id=<?=$cpent[1];?>" onclick="return confirm('Do you really want to disconnect this client?')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
  </tr>
<?php endforeach; ?>
</table>

