<?php
/* $Id$ */
/*
	ifstats.php
	part of pfSense (http://www.pfsense.com)

	Copyright (C) 2005-2006 Scott Ullrich (sullrich@gmail.com)
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
##|*IDENT=page-xmlrpcinterfacestats
##|*NAME=XMLRPC Interface Stats page
##|*DESCR=Allow access to the 'XMLRPC Interface Stats' page.
##|*MATCH=ifstats.php*
##|-PRIV


	require_once("interfaces.inc");

	$ifinfo = array();

	$if = $_GET['if'];

	$realif = get_real_interface($if);
	if(!$realif)
		$realif = $if; // Need for IPSec case interface.

	/* run netstat to determine link info */
	$linkinfo = "";
	unset($linkinfo);
	exec("/usr/bin/netstat -I {$realif} -nWb -f link", $linkinfo);
	$linkinfo = preg_split("/\s+/", $linkinfo[1]);
	if (preg_match("/^enc|^tun|^pppoe|^pptp/i", $realif)) {
		$ifinfo['inpkts'] = $linkinfo[3];
		$ifinfo['inbytes'] = $linkinfo[5];
		$ifinfo['outpkts'] = $linkinfo[6];
		$ifinfo['outbytes'] = $linkinfo[8];
	} else {
		$ifinfo['inpkts'] = $linkinfo[4];
		$ifinfo['inbytes'] = $linkinfo[6];
		$ifinfo['outpkts'] = $linkinfo[7];
		$ifinfo['outbytes'] = $linkinfo[9];
	}
	$temp = gettimeofday();
	$timing = (double)$temp["sec"] + (double)$temp["usec"] / 1000000.0;
	
	header("Last-Modified: " . gmdate( "D, j M Y H:i:s" ) . " GMT" );
	header("Expires: " . gmdate( "D, j M Y H:i:s", time() ) . " GMT" );
	header("Cache-Control: no-store, no-cache, must-revalidate" ); // HTTP/1.1
	header("Cache-Control: post-check=0, pre-check=0", FALSE );
	header("Pragma: no-cache"); // HTTP/1.0

	echo "$timing|" . $ifinfo['inbytes'] . "|" . $ifinfo['outbytes'] . "\n";

?>
