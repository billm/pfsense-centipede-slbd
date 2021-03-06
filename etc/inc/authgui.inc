<?php
/* $Id$ */
/*
	Copyright (C) 2007, 2008 Scott Ullrich <sullrich@gmail.com>
	All rights reserved.

        Copyright (C) 2005-2006 Bill Marquette <bill.marquette@gmail.com>
        All rights reserved.

        Copyright (C) 2006 Paul Taylor <paultaylor@winn-dixie.com>.
        All rights reserved.

        Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
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

include_once("auth.inc");
require_once("functions.inc");

/* We only support htpasswd backed HTTP Basic auth and session
 * based backing methods at the moment.
 * 		session_auth - this will use session based authentication and timeout
 * 		htpasswd_backed - this uses the "standard" .htpasswd file
 * 		passwd_backed - this will use the system passwd file in /etc
 * 		radius_backed - this will allow you to use a radius server
 *		pam_backed - this uses the system's PAM facility .htpasswd file
 */

//log_error("debug: FILE_NAME = {$_SERVER['REQUEST_URI']}");
//log_error("debug: FILE_NAME = {$_SERVER['SCRIPT_FILENAME']}");
//log_error("debug: SCRIPT_NAME = {$_SERVER['SCRIPT_NAME']}");

/* enable correct auth backend, default to htpasswd_backed */
$ldapcase = $config['system']['webgui']['backend'];
switch($ldapcase)
{
	case ldap:
		$backing_method="ldap_backed";
		break;
	case ldapother:
		$backing_method="ldap_backed";
		break;
	default:
		$backing_method="local_backed";
}

/* Authenticate user - exit if failed */
if (!session_auth($backing_method))
	exit;

/*
 * Once here, the user has authenticated with the web server.
 * We give them access only to the appropriate pages based on
 * the user or group privileges.
 */
$allowedpages = getAllowedPages($HTTP_SERVER_VARS['AUTH_USER']);

/*
 * redirect to first allowed page if requesting a wrong url
 */
if (!isAllowedPage($_SERVER['SCRIPT_NAME'])) {
	if (count($allowedpages) > 0) {
		$page = str_replace('*', '', $allowedpages[0]);
		$_SESSION['Post_Login'] = true;
		pfSenseHeader("/{$page}");
		exit;
	} else {
		display_error_form("201", "No page assigned to this user! Click here to logout.");
		exit;
	}
} else 
	$_SESSION['Post_Login'] = true;

/*
 * redirect browsers post-login to avoid pages
 * taking action in reponse to a POST request
 */
if (!$_SESSION['Post_Login']) {
	$_SESSION['Post_Login'] = true;
	pfSenseHeader($_SERVER['REQUEST_URI']);
	exit;
}

/*
 * determine if the user is allowed access to the requested page
 */
function display_error_form($http_code, $desc) {
	global $config, $g;
	$g['theme'] = $config['theme'];
	if(isAjax()) {
		echo "Error: {$http_code} Description: {$desc}";
		return;
	}

?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
        "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
	<head>
		<script type="text/javascript" src="/javascript/scriptaculous/prototype.js"></script>
		<script type="text/javascript" src="/javascript/scriptaculous/scriptaculous.js"></script>
		<title><?=$http_code?></title>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
		<link rel="shortcut icon" href="/themes/<?= $g['theme'] ?>/images/icons/favicon.ico" />
		<?php if (file_exists("{$g['www_path']}/themes/{$g['theme']}/login.css")): ?>
		<link rel="stylesheet" type="text/css" href="/themes/<?= $g['theme'] ?>/login.css" media="all" />
		<?php else: ?>
		<link rel="stylesheet" type="text/css" href="/themes/<?= $g['theme'] ?>/all.css" media="all" />
		<?php endif; ?>
		<script type="text/javascript">
		<!--
			function page_load() {}
			function clearError() {
				if($('inputerrors'))
				$('inputerrors').innerHTML='';
			}
			<?php
				require("headjs.php");
				echo getHeadJS();
			?>
		//-->
		</script>
		<script type="text/javascript" src="/themes/<?= $g['theme'] ?>/javascript/niftyjsCode.js"></script>
	</head>
	<body onload="page_load();">
		<div id="errordesc">
			<h1>&nbsp</h1>
			<a href="/index.php?logout">
			<p id="errortext" style="vertical-align: middle; text-align: center;">
				<span style="color: #000000; font-weight: bold;">
					<?=$desc;?>
				</span>
			</p>
		</div>
	</body>
</html>

<?php

} // end function


function display_login_form() {
	require_once("globals.inc");
	global $config, $g;
	$g['theme'] = $config['theme'];

	unset($input_errors);

	if(isAjax()) {
		if (isset($_POST['login'])) {
			if($_SESSION['Logged_In'] <> "True") {
				isset($_SESSION['Login_Error']) ? $login_error = $_SESSION['Login_Error'] : $login_error = "unknown reason";
				echo "showajaxmessage('Invalid login ({$login_error}).');";
			}
			if (file_exists("{$g['tmp_path']}/webconfigurator.lock")) {
				// TODO: add the IP from the user who did lock the device
				$whom = file_get_contents("{$g['tmp_path']}/webconfigurator.lock");
				echo "showajaxmessage('This device is currently beeing maintained by: {$whom}.');";
			}
		}
		exit;
	}

?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
        "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
	<head>
		<script type="text/javascript" src="/javascript/scriptaculous/prototype.js"></script>
		<script type="text/javascript" src="/javascript/scriptaculous/scriptaculous.js"></script>
		<title><?=gettext("Login"); ?></title>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
		<link rel="shortcut icon" href="/themes/<?= $g['theme'] ?>/images/icons/favicon.ico" />
		<?php if (file_exists("{$g['www_path']}/themes/{$g['theme']}/login.css")): ?>
		<link rel="stylesheet" type="text/css" href="/themes/<?= $g['theme'] ?>/login.css" media="all" />
		<?php else: ?>
		<link rel="stylesheet" type="text/css" href="/themes/<?= $g['theme'] ?>/all.css" media="all" />
		<?php endif; ?>
		<script type="text/javascript">
		<!--
			function page_load() {}
			function clearError() {
				if($('inputerrors'))
				$('inputerrors').innerHTML='';
			}
			<?php
				require("headjs.php");
				echo getHeadJS();
			?>
		//-->
		</script>
		<script type="text/javascript" src="/themes/<?= $g['theme'] ?>/javascript/niftyjsCode.js"></script>
	</head>
	<body onload="page_load()">
		<div id="login">
			<form id="iform" name="login_iform" method="post" autocomplete="off" action="<?=$_SERVER['SCRIPT_NAME'];?>">
				<h1></h1>
				<div id="inputerrors"><?=$_SESSION['Login_Error'];?></div>
				<p>
					<span style="text-align:left">
						<?=gettext("Username"); ?>:<br>
						<input onclick="clearError();" onchange="clearError();" id="usernamefld" type="text" name="usernamefld" class="formfld user" tabindex="1" />
					</span>
				</p>
				<br>
				<p>
					<span style="text-align:left">
						<?=gettext("Password"); ?>: <br>
						<input onclick="clearError();" onchange="clearError();" id="passwordfld" type="password" name="passwordfld" class="formfld pwd" tabindex="2" />
					</span>
				</p>
				<br>
				<p>
					<span style="text-align:center; font-weight: normal ; font-style: italic">
						<?=gettext("Enter username and password to login."); ?>
					</span>
				</p>        
				<p>
					<span style="text-align:center">
						<input type="submit" name="login" class="formbtn" value="<?=gettext("Login"); ?>" tabindex="3" />
					</span>
				</P>
			</form>
		</div>
	</body>
</html>
<?php
} // end function

?>
