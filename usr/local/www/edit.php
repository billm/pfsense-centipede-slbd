<?php
/* $Id$ */
/*
	edit.php
	Copyright (C) 2004, 2005 Scott Ullrich
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

$pgtitle = array(gettext("Diagnostics"), gettext("Edit file"));
require("guiconfig.inc");

if($_REQUEST['action']) {
	switch($_REQUEST['action']) {
		case 'load':
			if(strlen($_REQUEST['file']) < 1) {
				echo "|5|No file name specified.|";
			} elseif(is_dir($_REQUEST['file'])) {
				echo "|4|Loading a directory is not supported.|";
			} elseif(! is_file($_REQUEST['file'])) {
				echo "|3|File does not exist or is not a regular file.|";
			} else {
				$data = file_get_contents(urldecode($_REQUEST['file']));
				if($data === false) {
					echo "|1|Failed to read file.|";
				} else {
					echo "|0|{$_REQUEST['file']}|{$data}|";	
				}
			}
			exit;
		case 'save':
			if(strlen($_REQUEST['file']) < 1) {
				echo "|No file name specified.|";
			} else {
				$ret = file_put_contents($_REQUEST['file'], $_REQUEST['data']);
				if($_REQUEST['file'] == "config.xml")
					if(file_exists("/tmp/config.cache"))
						unlink("/tmp/config.cache");
				if($ret === false) {
					echo "|Failed to write file.|";
				} elseif($ret <> strlen($_REQUEST['data'])) {
					echo "|Error while writing file.|";
				} else {
					echo "|File successfully saved.|";
				}
			}
			exit;
	}
	exit;
}

require("head.inc");
outputCSSFileInline("code-syntax-highlighter/SyntaxHighlighter.css");
outputJavaScriptFileInline("javascript/scriptaculous/prototype.js");
outputJavaScriptFileInline("javascript/scriptaculous/scriptaculous.js");
outputJavaScriptFileInline("javascript/scriptaculous/effects.js");
outputJavaScriptFileInline("filebrowser/browser.js");

?>

<body link="#000000" vlink="#000000" alink="#000000">
<?php include("fbegin.inc"); ?>

<script type="text/javascript">	
	function loadFile() {
		$("fileStatus").innerHTML = "Loading file ...";
		Effect.Appear("fileStatusBox", { duration: 0.5 });

		new Ajax.Request(
			"<?=$_SERVER['SCRIPT_NAME'];?>", {
				method:     "post",
				postBody:   "action=load&file=" + $("fbTarget").value,
				onComplete: loadComplete
			}
		);
	}

	function loadComplete(req) {
		Element.show("fileContent")
		var values = req.responseText.split("|");
		values.shift(); values.pop();

		if(values.shift() == "0") {
			var file = values.shift();
			$("fileStatus").innerHTML = "File successfully loaded.";
			$("fileContent").value    = values.join("|");

			var lang = "none";
				 if(file.indexOf(".php") > 0) lang = "php";
			else if(file.indexOf(".inc") > 0) lang = "php";
			else if(file.indexOf(".xml") > 0) lang = "xml";
			else if(file.indexOf(".js" ) > 0) lang = "js";
			else if(file.indexOf(".css") > 0) lang = "css";

			if($("highlight").checked && lang != "none") {
				$("fileContent").className = lang + ":showcolumns";
				dp.SyntaxHighlighter.HighlightAll("fileContent", true, false);
			}
		}
		else {
			$("fileStatus").innerHTML = values[0];
			$("fileContent").value = "";
		}
		new Effect.Appear("fileContent");
	}

	function saveFile(file) {
		$("fileStatus").innerHTML = "Saving file ...";
		Effect.Appear("fileStatusBox", { duration: 0.5 });

		new Ajax.Request(
			"<?=$_SERVER['SCRIPT_NAME'];?>", {
				method:     "post",
				postBody:   "action=save&file=" + $("fbTarget").value +
							"&data=" + $("fileContent").value,
				onComplete: function(req) {
					var values = req.responseText.split("|");
					$("fileStatus").innerHTML = values[1];
				}
			}
		);
	}
</script>

<!-- file status box -->
<div style="display:none; background:#eeeeee;" id="fileStatusBox">
	<div class="vexpl" style="padding-left:15px;">
		<strong id="fileStatus"></strong>
	</div>
</div>

<br />

<table width="100%" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td class="tabcont" align="center">

<!-- controls -->
<table width="100%" cellpadding="9" cellspacing="9">
	<tr>
		<td align="center" class="list">
			Save / Load from path:
			<input type="text"   class="formfld file" id="fbTarget"         size="45" />
			<input type="button" class="formbtn"      onclick="loadFile();" value="<?=gettext('Load');?>" />
			<input type="button" class="formbtn"      id="fbOpen"           value="<?=gettext('Browse');?>" />
			<input type="button" class="formbtn"      onclick="saveFile();" value="<?=gettext('Save');?>" />
			<br />
			<?php
			/*
			<input type="checkbox" id="highlight" /><?=gettext("Enable syntax highlighting");
			*/
			?>
		</td>
	</tr>
</table>

<!-- filebrowser -->
<div id="fbBrowser" style="display:none; border:1px dashed gray; width:98%;"></div>

<!-- file viewer/editor -->
<table width="100%">
	<tr>
		<td valign="top" class="label">
			<div style="background:#eeeeee;" id="fileOutput">
				<textarea id="fileContent" name="fileContent" style="width:100%;" rows="30" wrap="off"></textarea>
			</div>
		</td>
	</tr>
</table>

		</td>
	</tr>
</table>

<script type="text/javascript" src="/code-syntax-highlighter/shCore.js"></script>
<script type="text/javascript" src="/code-syntax-highlighter/shBrushCss.js"></script>
<script type="text/javascript" src="/code-syntax-highlighter/shBrushJScript.js"></script>
<script type="text/javascript" src="/code-syntax-highlighter/shBrushPhp.js"></script>
<script type="text/javascript" src="/code-syntax-highlighter/shBrushXml.js"></script>
<script type="text/javascript">
	Event.observe(
		window, "load",
		function() {
			$("fbTarget").focus();

			NiftyCheck();
			Rounded("div#fileStatusBox", "all", "#ffffff", "#eeeeee", "smooth");
		}
	);

	<?php if($_GET['action'] == "load"): ?>
		Event.observe(
			window, "load",
			function() {
				$("fbTarget").value = "<?=$_GET['path'];?>";
				loadFile();
			}
		);
	<?php endif; ?>
</script>

<?php include("fend.inc"); ?>
</body>
</html>
