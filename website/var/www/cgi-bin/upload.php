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

if ($_SERVER['REQUEST_METHOD'] == "GET") {
	echo ("GET METHOD NOT SUITED TO THE SCRIPT\r\n");
	return ;
}


if (isset($_FILES['file'])) {
    $file = $_FILES['file'];
    // Vérifie s'il y a eu une erreur d'upload
    if ($file['error'] === UPLOAD_ERR_OK) {
        $upload_dir = '/tmp/'; // Assure-toi que ce dossier existe et est inscriptible
        $upload_file = $upload_dir . basename($file['name']);

        if (move_uploaded_file($file['tmp_name'], $upload_file)) {
            echo "File " . htmlspecialchars(basename($file['name'])) . " uploaded successfully.\r\n";
            echo "Path : " . $upload_file . "\r\n";
        } else {
            echo "Error moving file.\r\n";
            error_log("Error moving file: " . $file['tmp_name'] . " to " . $upload_file);
        }
  } else {

        echo "Upload error : Code " . $file['error'] . "\r\n";
        error_log("Upload error code: " . $file['error']);
        switch ($file['error']) {
            case UPLOAD_ERR_INI_SIZE:
                error_log("The uploaded file exceeds the upload_max_filesize directive in php.ini.");
                break;
            case UPLOAD_ERR_FORM_SIZE:
                error_log("The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form.");
                break;
            case UPLOAD_ERR_PARTIAL:
                error_log("The uploaded file was only partially uploaded.");
                break;
            case UPLOAD_ERR_NO_FILE:
                error_log("No file was uploaded.");
                break;
            case UPLOAD_ERR_NO_TMP_DIR:
                error_log("Missing a temporary folder.");
                break;
            case UPLOAD_ERR_CANT_WRITE:
                error_log("Failed to write file to disk.");
                break;
            case UPLOAD_ERR_EXTENSION:
                error_log("A PHP extension stopped the file upload.");
                break;
            default:
                error_log("Unknown upload error.");
                break;
        }
    }
} else {
    echo "Error: 'file' key is missing.\r\n";
    error_log("Error: 'file' key is missing in \$_FILES array.");
}

?>
