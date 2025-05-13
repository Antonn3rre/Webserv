## WEBSERV

### Nouvelles fonctions
_gai_strerror_: const char *gai_strerror(int ecode); utile si getnameinfo() ou getaddrinfo() fail

_htons_:

_htonl_: convertit  l’entier  non  signé  hostlong  depuis
       l’ordre des octets de l’hôte vers celui du réseau
_htons_: pareil mais avec hostshort
_nhtonl_ et _nhtons_: font l'inverse (reseau vers hote)

_select_: il attend que l'un des descripteurs de fichier parmi un ensemble soit prêt pour effectuer des entrées-sorties.

_poll_: une variation de select(2) mais en mieux

_epoll_: API de 3 fonctions, comme poll mais en mieux
	_epoll_create_: cree une instance epoll et renvoie le fd qui lui correspond
	_epoll_ctl_: ajoute des fd a une liste de l'instance epoll, chaque liste correspondant a une operation a effectuer sur le fd
	_epoll_wait_: bloque le thread jusqu'au prochain evenement sur l'instance donnee

_kqueue_: cree une queue d'event kernel et renvoie son fd

_kevent_: ajoute des events dans la queue donnee

_socket_: int socket(int domain, int type, int protocol);  crée un point de communication pour un protocole specifique, et renvoie un descripteur de fichier

_socketpair_: cree une paire de sockets connectees

_accept_: accepte une connexion sur une socket

_listen_: marque une socket comme passive, qui servira a accepter les connexion avec _accept_

_send_: envoie un message sur un socket

_recv_: recoit un message sur un socket

_bind_: attribue un nom a un socket, generalement avant de pouvoir accepter les connexions

_connect_: 

### HTTP message syntax

#### Version
First line: version of http used (1.1 in this project)
```
HTTP/1.1
```

#### URI
Uniform Resource Identifier (_URI_): string identifying a resource (a website, a webservice, etc...), case-insensitive
```
http://<host>(:<port>)/<abs_path>
```
By default (if not specified), the port is 80, and the abs_path is "/"

#### Time format
Any of the three following formats is supported, although the first one is prefered
```
Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
```

#### Character sets
Specifies the character sets prefered by the client, if not specified it default to "US_ASCII". Several sets can be specified by separating them by a '/'.
Available are US-ASCII or ISO-8859-1 or ISO-8859-7
