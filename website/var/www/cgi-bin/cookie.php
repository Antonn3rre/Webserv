#!/usr/bin/php-cgi
<?php

// Active l'affichage des erreurs pour le développement
#ini_set('display_errors', 1);
#ini_set('display_startup_errors', 1);
#error_reporting(E_ALL);

// Log les variables d'environnement que PHP reçoit
#error_log("--- PHP SERVER VARIABLES ---");
#foreach ($_SERVER as $key => $value) {
#    error_log("$key = $value");
#}
#error_log("----------------------------");

// Log infos du fichier recu
#error_log("--- PHP _FILES array ---");
#error_log(print_r($_FILES, true));
#error_log("------------------------");

// Si GET -> juste dire si on connait
// Si POST -> dire si on connait et si non set

echo "Content-Type: text/plain\r\n\r\n";
// Recuperer le header cookie que le navigateur envoie
if ($_SERVER['REQUEST_METHOD'] == "GET") {
	// if le header existe return la valeur du cookie
	if (!empty($_SERVER['HTTP_COOKIE'])) {
		$cookies = [];
		parse_str(str_replace('; ', '&', $_SERVER['HTTP_COOKIE']), $cookies);
#		echo "Content-Type: text/plain\r\n\r\n";
		if (isset($cookies['username']))
			echo $cookies['username'];
		else
			echo "Unkwown";
	}
}
else {
	if ($_POST['username']) {
		echo("Set-Cookie: username=" . $_POST['username'] . "; Path=/\r\n\r\n");
	}
}

?>
