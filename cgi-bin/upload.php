#!/usr/bin/php-cgi
<?php

echo "Hello World!\n";
echo "Request method : ";
echo getenv("REQUEST_METHOD");
echo "\n";
$file = $_FILES['uploaded_file'];
#echo $_SERVER['argv'];
#echo $_SERVER['argv'];



// Active l'affichage des erreurs pour le développement
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

// Log les variables d'environnement que PHP reçoit
error_log("--- PHP SERVER VARIABLES ---");
foreach ($_SERVER as $key => $value) {
    error_log("$key = $value");
}
error_log("----------------------------");

// Log le contenu brut de la requête POST
$raw_post_data = file_get_contents('php://input');
error_log("--- RAW POST DATA (Length: " . strlen($raw_post_data) . ") ---");
error_log($raw_post_data);
error_log("----------------------------");

// Log les tableaux $_POST et $_FILES
error_log("--- PHP _POST array ---");
error_log(print_r($_POST, true));
error_log("-----------------------");

error_log("--- PHP _FILES array ---");
error_log(print_r($_FILES, true));
error_log("------------------------");

// La partie de ton code qui pose problème
if (isset($_FILES['uploaded_file'])) {
    $file = $_FILES['uploaded_file'];

    // Vérifie s'il y a eu une erreur d'upload
    if ($file['error'] === UPLOAD_ERR_OK) {
        $upload_dir = '/tmp/'; // Assure-toi que ce dossier existe et est inscriptible
        $upload_file = $upload_dir . basename($file['name']);

        if (move_uploaded_file($file['tmp_name'], $upload_file)) {
            echo "Fichier " . htmlspecialchars(basename($file['name'])) . " a été uploadé avec succès.\n";
            echo "Chemin d'accès : " . $upload_file . "\n";
        } else {
            echo "Erreur lors du déplacement du fichier.\n";
            error_log("Error moving file: " . $file['tmp_name'] . " to " . $upload_file);
        }
    } else {
        echo "Erreur d'upload : Code " . $file['error'] . "\n";
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
    echo "Aucun fichier n'a été uploadé ou la clé 'uploaded_file' est manquante.\n";
    error_log("Error: 'uploaded_file' key is missing in \$_FILES array.");
}

?>
