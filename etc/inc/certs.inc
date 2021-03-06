<?php
/* $Id$ */
/*
		Copyright (C) 2008 Shrew Soft Inc
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

		DISABLE_PHP_LINT_CHECKING
*/

require_once("functions.inc");

function & lookup_ca($refid) {
	global $config;

	if (is_array($config['system']['ca']))
		foreach ($config['system']['ca'] as & $ca)
			if ($ca['refid'] == $refid)
				return $ca;

	return false;
}

function & lookup_cert($refid) {
	global $config;

	if (is_array($config['system']['cert']))
		foreach ($config['system']['cert'] as & $cert)
			if ($cert['refid'] == $refid)
				return $cert;

	return false;
}

function ca_import(& $ca, $str) {

	$ca['crt'] = base64_encode($str);

	return true;
}

function ca_create(& $ca, $keylen, $lifetime, $dn) {

	$args = array(
		"digest_alg" => "sha1",
		"private_key_bits" => $keylen,
		"private_key_type" => OPENSSL_KEYTYPE_RSA,
		"encrypt_key" => false);

	// generate a new key pair
	$res_key = openssl_pkey_new();

	// generate a certificate signing request
	$res_csr = openssl_csr_new($dn, $res_key, $args);

	// self sign the certificate
	$res_crt = openssl_csr_sign($res_csr, null, $res_key, $lifetime, $args);

	// export our certificate data
	openssl_pkey_export($res_key, $str_key);
	openssl_x509_export($res_crt, $str_crt);

	// return our ca information
	$ca['crt'] = base64_encode($str_crt);
	$ca['prv'] = base64_encode($str_key);
	$ca['serial'] = 0;

	return true;
}

function cert_import(& $cert, $crt_str, $key_str) {

	$cert['crt'] = base64_encode($crt_str);
	$cert['prv'] = base64_encode($key_str);

	return true;
}

function cert_create(& $cert, $caref, $keylen, $lifetime, $dn) {

	$ca =& lookup_ca($caref);
	if (!$ca)
		return false;

	$ca_str_crt = base64_decode($ca['crt']);
	$ca_str_key = base64_decode($ca['prv']);
	$ca_res_crt = openssl_x509_read($ca_str_crt);
	$ca_res_key = openssl_pkey_get_private($ca_str_key);
	$ca_serial = $ca['serial']++;

	$args = array(
		"digest_alg" => "sha1",
		"private_key_bits" => $keylen,
		"private_key_type" => OPENSSL_KEYTYPE_RSA,
		"encrypt_key" => false);

	// generate a new key pair
	$res_key = openssl_pkey_new();

	// generate a certificate signing request
	$res_csr = openssl_csr_new($dn, $res_key, $args);

	// self sign the certificate
	$res_crt = openssl_csr_sign($res_csr, $ca_res_crt, $ca_res_key, $lifetime,
				 $args, $ca_serial);

	// export our certificate data
	openssl_pkey_export($res_key, $str_key);
	openssl_x509_export($res_crt, $str_crt);

	// return our certificate information
	$cert['caref'] = $caref;
	$cert['crt'] = base64_encode($str_crt);
	$cert['prv'] = base64_encode($str_key);

	return true;
}

function csr_generate(& $cert, $keylen, $dn) {

	$args = array(
		"digest_alg" => "sha1",
		"private_key_bits" => $keylen,
		"private_key_type" => OPENSSL_KEYTYPE_RSA,
		"encrypt_key" => false);

	// generate a new key pair
	$res_key = openssl_pkey_new();

	// generate a certificate signing request
	$res_csr = openssl_csr_new($dn, $res_key, $args);

	// export our request data
	openssl_pkey_export($res_key, $str_key);
	openssl_csr_export($res_csr, $str_csr);

	// return our request information
	$cert['csr'] = base64_encode($str_csr);
	$cert['prv'] = base64_encode($str_key);

	return true;
}

function csr_complete(& $cert, $str_crt) {

	// return our request information
	$cert['crt'] = base64_encode($str_crt);
	unset($cert['csr']);

	return true;
}

function csr_get_subject($str_crt, $decode = true) {

	if ($decode)
		$str_crt = base64_decode($str_crt);

	$components = openssl_csr_get_subject($str_crt);

	if (!is_array($components))
		return "unknown";

	foreach ($components as $a => $v) {
		if (!strlen($subject))
			$subject = "{$a}={$v}";
		else
			$subject = "{$a}={$v}, {$subject}";
	}

	return $subject;
}

function cert_get_subject($str_crt, $decode = true) {

	if ($decode)
		$str_crt = base64_decode($str_crt);

	$inf_crt = openssl_x509_parse($str_crt);
	$components = $inf_crt['subject'];

	if (!is_array($components))
		return "unknown";

	foreach ($components as $a => $v) {
		if (!strlen($subject))
			$subject = "{$a}={$v}";
		else
			$subject = "{$a}={$v}, {$subject}";
	}

	return $subject;
}

function cert_get_subject_array($crt) {
	$str_crt = base64_decode($crt);
	$inf_crt = openssl_x509_parse($str_crt);
	$components = $inf_crt['subject'];
	$subject_array = array();

	foreach($components as $a => $v)
		$subject_array[] = array('a' => $a, 'v' => $v);

	return $subject_array;
}

?>
