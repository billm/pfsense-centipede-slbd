<?php
/* $Id$ */
/*
	Copyright (C) 2008 Ermal Lu�i
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
##|*IDENT=page-services-rfc2136clients
##|*NAME=Services: RFC 2136 clients page
##|*DESCR=Allow access to the 'Services: RFC 2136 clients' page.
##|*MATCH=services_rfc2136.php*
##|-PRIV


require("guiconfig.inc");

if (!is_array($config['dnsupdates']['dnsupdate']))
	$config['dnsupdates']['dnsupdate'] = array();

$a_rfc2136 = &$config['dnsupdates']['dnsupdate'];

if ($_GET['act'] == "del") {
		unset($a_rfc2136[$_GET['id']]);

		write_config();

		header("Location: services_dyndns.php");
		exit;
}

$pgtitle = array("Services", "RFC 2136 clients");
include("head.inc");

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<form action="services_rfc2136.php" method="post" name="iform" id="iform">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td>
<?php
	$tab_array = array();
	$tab_array[] = array("DynDns", false, "services_dyndns.php");
	$tab_array[] = array("RFC 2136", true, "services_rfc2136.php");
	display_top_tabs($tab_array);
?>
  </td></tr>
  <tr>
    <td>
	<div id="mainarea">
	<table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr>
				  <td width="5%"  class="listhdrr"></td>
                  <td width="25%" class="listhdrr">Hostname</td>
                  <td width="60%" class="listhdr">Description</td>
                  <td width="10%" class="list"></td>
				</tr>
			  <?php $i = 0; foreach ($a_rfc2136 as $rfc2136): ?>
                <tr>
				  <td class="listlr">
				  <?php $iflist = get_configured_interface_with_descr();
				  		foreach ($iflist as $if => $ifdesc):
							if ($rfc2136['interface'] == $if): ?>
								<?=$ifdesc; break;?>
					<?php endif; endforeach; ?>
				  </td>
                  <td class="listr">
					<?=htmlspecialchars($rfc2136['host']);?>
                  </td>
                  <td class="listbg">
                    <?=htmlspecialchars($rfc2136['descr']);?>&nbsp;
                  </td>
                  <td valign="middle" nowrap class="list"> <a href="services_rfc2136_edit.php?id=<?=$i;?>"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_e.gif" width="17" height="17" border="0"></a>
                     &nbsp;<a href="services_rfc2136.php?act=del&id=<?=$i;?>" onclick="return confirm('Do you really want to delete this client?')"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_x.gif" width="17" height="17" border="0"></a></td>
				</tr>
			  <?php $i++; endforeach; ?>
                <tr>
                  <td class="list" colspan="3">&nbsp;</td>
                  <td class="list"> <a href="services_rfc2136_edit.php"><img src="./themes/<?= $g['theme']; ?>/images/icons/icon_plus.gif" width="17" height="17" border="0"></a></td>
				</tr>
				<tr>
				<td colspan="3" class="list"><p class="vexpl"><span class="red"><strong>
				  Note:<br>
				  </strong></span>
				  Add something meaningful here.
				  </td>
				<td class="list">&nbsp;</td>
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
