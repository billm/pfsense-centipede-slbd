<?php
/* $Id$ */
/*
    diag_cpu_pftop.php
    Copyright (C) 2008 Scott Ullrich
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
##|*IDENT=page-diag-system-activity
##|*NAME=Diagnostics: System Activity
##|*DESCR=Allows access to the 'Diagnostics: System Activity' page
##|*MATCH=diag_system_activity*
##|-PRIV

require("guiconfig.inc");

$pfSversion = str_replace("\n", "", file_get_contents("/etc/version"));

$pgtitle = "Diagnostics: PFTop";

if($_REQUEST['getactivity']) {
	if($_REQUEST['sorttype'])
		$sorttype = escapeshellarg($_REQUEST['sorttype']);
	else
		$sorttype = "bytes";	
	$text = `pftop -b -a -o {$sorttype}`;
	echo $text;
	exit;
}

include("head.inc");

if($_REQUEST['sorttype'])
	$sorttype = htmlentities($_REQUEST['sorttype']);
else
	$sorttype = "bytes";

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<form method="post" action="diag_system_pftop.php">
<script type="text/javascript">
	function getcpuactivity() {
		var url = "/diag_system_pftop.php";
		var pars = 'getactivity=yes&sorttype=' + $('sorttype').value;
		var myAjax = new Ajax.Request(
			url,
			{
				method: 'post',
				parameters: pars,
				onComplete: activitycallback
			});
	}
	function activitycallback(transport) {
		$('cpuactivitydiv').innerHTML = '<font face="Courier"><font size="2"><b><pre>' + transport.responseText  + '</pre></font>';
		setTimeout('getcpuactivity()', 2500);		
	}
	setTimeout('getcpuactivity()', 1000);	
</script>
<div id='maincontent'>
<script src="/javascript/scriptaculous/prototype.js" type="text/javascript"></script>
<?php
	include("fbegin.inc"); 
	if(strstr($pfSversion, "1.2")) 
		echo "<p class=\"pgtitle\">{$pgtitle}</p>";
	if($savemsg) {
		echo "<div id='savemsg'>";
		print_info_box($savemsg);
		echo "</div>";	
	}
	if ($input_errors)
		print_input_errors($input_errors);
?>
	<form method="post">
	Sort type: 
	<select name='sorttype' id='sorttype' onChange='this.form.submit();'>
		<option value='<?=$sorttype?>'><?=$sorttype?></option>
		<option value='age'>age</option>
		<option value='bytes'>bytes</option>
		<option value='dest'>dest</option>
		<option value='dport'>dport</option>
		<option value='exp'>exp</option>
		<option value='none'>none</option>
		<option value='peak'>peak</option>
		<option value='pkt'>pkt</option>
		<option value='rate'>rate</option>
		<option value='size'>size</option>
		<option value='sport'>sport</option>
		<option value='src'>src</option>														
	</select>
	<p/>
<table width="100%" border="0" cellpadding="0" cellspacing="0">  
  <tr>
    <td>
	<table id="backuptable" class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
		<tr>
			<td>
				<center>
				<table>
					<tr><td>
						<div name='cpuactivitydiv' id='cpuactivitydiv'>
							<b>Gathering pfTOP activity, please wait...
						</div>
					</td></tr>
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
