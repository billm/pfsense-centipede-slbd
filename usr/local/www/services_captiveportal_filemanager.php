<?php
/*
	services_captiveportal_filemanager.php
	part of m0n0wall (http://m0n0.ch/wall)

	Copyright (C) 2005-2006 Jonathan De Graeve (jonathan.de.graeve@imelda.be)
	and Paul Taylor (paultaylor@winn-dixie.com).
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
##|*IDENT=page-services-captiveportal-filemanager
##|*NAME=Services: Captive portal: File Manager page
##|*DESCR=Allow access to the 'Services: Captive portal: File Manager' page.
##|*MATCH=services_captiveportal_filemanager.php*
##|-PRIV


$pgtitle = array("Services","Captive portal");

require_once("guiconfig.inc");

if (!is_array($config['captiveportal']['element']))
	$config['captiveportal']['element'] = array();

cpelements_sort();
$a_element = &$config['captiveportal']['element'];

// Calculate total size of all files
$total_size = 0;
foreach ($a_element as $element) {
	$total_size += $element['size'];
}

if ($_POST) {
    unset($input_errors);

    if (is_uploaded_file($_FILES['new']['tmp_name'])) {

    	if(!stristr($_FILES['new']['name'], "captiveportal-"))
    		$name = "captiveportal-" . $_FILES['new']['name'];
    	else
    		$name = $_FILES['new']['name'];
    	$size = filesize($_FILES['new']['tmp_name']);

    	// is there already a file with that name?
    	foreach ($a_element as $element) {
			if ($element['name'] == $name) {
				$input_errors[] = "A file with the name '$name' already exists.";
				break;
			}
		}

		// check total file size
		if (($total_size + $size) > $g['captiveportal_element_sizelimit']) {
			$input_errors[] = "The total size of all files uploaded may not exceed " .
				format_bytes($g['captiveportal_element_sizelimit']) . ".";
		}

		if (!$input_errors) {
			$element = array();
			$element['name'] = $name;
			$element['size'] = $size;
			$element['content'] = base64_encode(file_get_contents($_FILES['new']['tmp_name']));

			$a_element[] = $element;

			write_config();
			captiveportal_write_elements();
			header("Location: services_captiveportal_filemanager.php");
			exit;
		}
    }
} else {
	if (($_GET['act'] == "del") && $a_element[$_GET['id']]) {
		conf_mount_rw();
		unlink_if_exists($g['captiveportal_path'] . "/" . $a_element[$id]['name']);
		unset($a_element[$_GET['id']]);
		write_config();
		captiveportal_write_elements();
		conf_mount_ro();
		header("Location: services_captiveportal_filemanager.php");
		exit;
	}
}

include("head.inc");

?>
<?php include("fbegin.inc"); ?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<form action="services_captiveportal_filemanager.php" method="post" enctype="multipart/form-data" name="iform" id="iform">
<?php if ($input_errors) print_input_errors($input_errors); ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
<?php
	$tab_array = array();
	$tab_array[] = array("Captive portal", false, "services_captiveportal.php");
	$tab_array[] = array("Pass-through MAC", false, "services_captiveportal_mac.php");
	$tab_array[] = array("Allowed IP addresses", false, "services_captiveportal_ip.php");
	$tab_array[] = array("Users", false, "services_captiveportal_users.php");
	$tab_array[] = array("File Manager", true, "services_captiveportal_filemanager.php");
	display_top_tabs($tab_array);
?>  </td></tr>
  <tr>
    <td class="tabcont">
	<table width="80%" border="0" cellpadding="0" cellspacing="0">
      <tr>
        <td width="70%" class="listhdrr">Name</td>
        <td width="20%" class="listhdr">Size</td>
        <td width="10%" class="list">
		<table border="0" cellspacing="0" cellpadding="1">
		    <tr>
			<td width="17" heigth="17"></td>
			<td><a href="services_captiveportal_filemanager.php?act=add"><img src="/themes/<?php echo $g['theme']; ?>/images/icons/icon_plus.gif" title="add file" width="17" height="17" border="0"></a></td>
		    </tr>
		</table>
	</td>
      </tr>
  <?php $i = 0; foreach ($a_element as $element): ?>
  	  <tr>
		<td class="listlr"><?=htmlspecialchars($element['name']);?></td>
		<td class="listr" align="right"><?=format_bytes($element['size']);?></td>
		<td valign="middle" nowrap class="list">
		<a href="services_captiveportal_filemanager.php?act=del&id=<?=$i;?>" onclick="return confirm('Do you really want to delete this file?')"><img src="/themes/<?php echo $g['theme']; ?>/images/icons/icon_x.gif" title="delete file" width="17" height="17" border="0"></a>
		</td>
	  </tr>
  <?php $i++; endforeach; ?>

  <?php if (count($a_element) > 0): ?>
  	  <tr>
		<td class="listlr" style="background-color: #eee"><strong>TOTAL</strong></td>
		<td class="listr" style="background-color: #eee" align="right"><strong><?=format_bytes($total_size);?></strong></td>
		<td valign="middle" nowrap class="list"></td>
	  </tr>
  <?php endif; ?>

  <?php if ($_GET['act'] == 'add'): ?>
	  <tr>
		<td class="listlr" colspan="2"><input type="file" name="new" class="formfld file" size="40" id="new">
		<input name="Submit" type="submit" class="formbtn" value="Upload"></td>
		<td valign="middle" nowrap class="list">
		<a href="services_captiveportal_filemanager.php"><img src="/themes/<?php echo $g['theme']; ?>/images/icons/icon_x.gif" title="cancel" width="17" height="17" border="0"></a>
		</td>
	  </tr>
  <?php else: ?>
	  <tr>
		<td class="list" colspan="2"></td>
		<td class="list">
			<table border="0" cellspacing="0" cellpadding="1">
			    <tr>
				<td width="17" heigth="17"></td>
				<td><a href="services_captiveportal_filemanager.php?act=add"><img src="/themes/<?php echo $g['theme']; ?>/images/icons/icon_plus.gif" title="add file" width="17" height="17" border="0"></a></td>
			    </tr>
			</table>
		</td>
	  </tr>
  <?php endif; ?>
	</table>
	<span class="vexpl"><span class="red"><strong>
	Note:<br>
	</strong></span>
	Any files that you upload here with the filename prefix of captiveportal- will
	be made available in the root directory of the captive portal HTTP(S) server.
	You may reference them directly from your portal page HTML code using relative paths.
	Example: you've uploaded an image with the name 'captiveportal-test.jpg' using the
	file manager. Then you can include it in your portal page like this:<br><br>
	<tt>&lt;img src=&quot;captiveportal-test.jpg&quot; width=... height=...&gt;</tt>
	<br><br>
	In addition, you can also upload .php files for execution.  You can pass the filename
	to your custom page from the initial page by using text similar to:
	<br><br>
	<tt>&lt;a href="/captiveportal-aup.php?redirurl=$PORTAL_REDIRURL$"&gt;Acceptable usage policy&lt/a&gt;</tt>
	<br><br>
	The total size limit for all files is <?=format_bytes($g['captiveportal_element_sizelimit']);?>.</span>
</td>
</tr>
</table>
</form>
<?php include("fend.inc"); ?>
</body>
</html>

