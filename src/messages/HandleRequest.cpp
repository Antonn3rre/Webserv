// include si class
#include "RequestMessage.hpp"
#include "Server.hpp"

// return std::pair<code, page> ?
void handleRequest(Server &server, RequestMessage &request) {
	// iterer location de server pour attribuer le bon
	// stocker resultat temporaire dans std pair avec index et nmb de char en commun ?
	// check entre chaque /

	// Quand loc trouve, verifier si method autorise
	// -> sinon return error
	// -> si methods existe pas ??

	//!\ check si return ou method a la priorite
	// si un return -> renvoye direct le bon code erreur + page

	// recuperer uri et construire chemin avec root (si aucun root defini ?)

	// check si dossier, si oui envoyer sur index
	// Si pas d'index, check autoindent et faire en fonction

	// A PLACER : CGI
}
