<?php
/* $Id$ */
/*
	status_slbd_vs.php
	part of pfSense (http://www.pfsense.com/)

	Copyright (C) 2007 Seth Mos <seth.mos@xs4all.nl>.
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
##|*IDENT=page-status-loadbalancer-virtualserver
##|*NAME=Status: Load Balancer: Virtual Server page
##|*DESCR=Allow access to the 'Status: Load Balancer: Virtual Server' page.
##|*MATCH=status_slbd_vs.php*
##|-PRIV


require("guiconfig.inc");

if (!is_array($config['load_balancer']['lbpool'])) {
	$config['load_balancer']['lbpool'] = array();
}
if (!is_array($config['load_balancer']['virtual_server'])) {
	$config['load_balancer']['virtual_server'] = array();
}
$a_vs = &$config['load_balancer']['virtual_server'];
$a_pool = &$config['load_balancer']['lbpool'];



// # relayctl show summary
// Id   Type      Name                      Avlblty Status
// 1    redirect  testvs2                           active
// 5    table     test2:80                          active (3 hosts up)
// 11   host      192.168.1.2               91.55%  up
// 10   host      192.168.1.3               100.00% up
// 9    host      192.168.1.4               88.73%  up
// 3    table     test:80                           active (1 hosts up)
// 7    host      192.168.1.2               66.20%  down
// 6    host      192.168.1.3               97.18%  up
// 0    redirect  testvs                            active
// 3    table     test:80                           active (1 hosts up)
// 7    host      192.168.1.2               66.20%  down
// 6    host      192.168.1.3               97.18%  up
// 4    table     testvs-sitedown:80                active (1 hosts up)
// 8    host      192.168.1.4               84.51%  up
// # relayctl show redirects
// Id   Type      Name                      Avlblty Status
// 1    redirect  testvs2                           active
// 0    redirect  testvs                            active
// # relayctl show redirects
// Id   Type      Name                      Avlblty Status
// 1    redirect  testvs2                           active
//            total: 2 sessions
//            last: 2/60s 2/h 2/d sessions
//            average: 1/60s 0/h 0/d sessions
// 0    redirect  testvs                            active

$redirects_a = exec_command_and_return_text_array('/usr/local/sbin/relayctl show redirects');
$summary_a = exec_command_and_return_text_array('/usr/local/sbin/relayctl show summary');
$rdr_a = parse_redirects($redirects_a);
//$server_a = parse_summary($summary_a, parse_redirects($redirects_a));

function parse_redirects($rdr_a) {
  $vs = array();
  for ($i = 0; isset($rdr_a[$i]); $i++) {
    $line = $rdr_a[$i];
    if (preg_match("/^[0-9]+/", $line)) {
      $regs = array();
      if($x = preg_match("/^[0-9]+\s+redirect\s+([0-9a-zA-Z]+)\s+([a-z]+)/", $line, $regs)) {
        $vs[$regs[1]] = array();
        $vs[$regs[1]]['status'] = $regs[2];
      }
    }
  }
  return $vs;
}

function parse_summary($summary, $rdrs_a) {
  $server_a = array();
  return $server_a;
}

$pgtitle = array("Status","Load Balancer","Virtual Server");
include("head.inc");

?>
<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>
<table width="100%" border="0" cellpadding="0" cellspacing="0">
  <tr><td class="tabnavtbl">
  <?php
        /* active tabs */
        $tab_array = array();
        $tab_array[] = array("Pools", false, "status_slbd_pool.php");
        $tab_array[] = array("Virtual Servers", true, "status_slbd_vs.php");
        display_top_tabs($tab_array);
  ?>
  </td></tr>
  <tr>
    <td>
	<div id="mainarea">
              <table class="tabcont" width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr>
                  <td width="10%" class="listhdrr">Name</td>
		  <td width="10%" class="listhdrr">Port</td>
                  <td width="10%" class="listhdrr">Servers</td>
                  <td width="30%" class="listhdrr">Status</td>
                  <td width="30%" class="listhdr">Description</td>
				</tr>
			  <?php $i = 0; foreach ($a_vs as $vsent): ?>
                <tr>
                  <td class="listlr">
				<?=$vsent['name'];?>
                  </td>
                  <td class="listr" align="center" >
                                <?=$vsent['port'];?>
                                <br />
                  </td>
                  <td class="listr" align="center" >
			<table border="0" cellpadding="0" cellspacing="2">
                        <?php
			foreach ($a_pool as $vipent) {
				if ($vipent['name'] == $vsent['pool']) {
					foreach ((array) $vipent['servers'] as $server) {
						print "<tr><td> {$server} </td></tr>";
					}
				}
			}
			?>
			</table>
                  </td>
                  <?php
                  switch ($rdr_a[$vsent['name']]['status']) {
                    case 'active':
                      $bgcolor = "lightgreen";
                      break;
                    default:
                      $bgcolor = "lightcoral";
                  }
                  ?>
                  <td class="listr" bgcolor="<?=$bgcolor?>">
                  <?=$rdr_a[$vsent['name']]['status']?>
                  </td>
                  <td class="listbg" >
						<?=$vipent['desc'];?>
                  </td>
                </tr>
		<?php $i++; endforeach; ?>
             </table>
	   </div>
	</table>

<?php include("fend.inc"); ?>
</body>
</html>
