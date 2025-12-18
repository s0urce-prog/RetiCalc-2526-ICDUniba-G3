#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
    /* Windows */
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    typedef int socklen_t;
    #define CLOSE_SOCKET(s) closesocket(s)
    #define SOCKET_VALID(s) ((s) != INVALID_SOCKET)
    #define GET_ERROR() WSAGetLastError()
#else
    /* UNIX/Linux/macOS */
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <errno.h>
    typedef int socket_t;
    #define CLOSE_SOCKET(s) close(s)
    #define SOCKET_VALID(s) ((s) >= 0)
    #define GET_ERROR() errno
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
#endif

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024
#define BACKLOG 5

/* Funzione per verificare se un carattere e' una vocale */
int is_vowel(char c) {
    char lower = (char)tolower((unsigned char)c);
    return (lower == 'a' || lower == 'e' || lower == 'i' || lower == 'o' || lower == 'u');
}

/* Funzione per rimuovere le vocali da una stringa */
void remove_vowels(const char *input, char *output) {
    int i, j;
    j = 0;
    for (i = 0; input[i] != '\0'; i++) {
        if (!is_vowel(input[i])) {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

int main() {
    socket_t server_sockfd, client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int opt;
    int bytes_received;
    char *client_ip;
#ifdef _WIN32
    WSADATA wsaData;
#endif

#ifdef _WIN32
    /* Inizializzazione Winsock */
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Errore nell'inizializzazione di Winsock\n");
        exit(EXIT_FAILURE);
    }
#endif

    /* Creazione socket TCP */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (!SOCKET_VALID(server_sockfd)) {
        printf("Errore nella creazione del socket: %d\n", GET_ERROR());
#ifdef _WIN32
        WSACleanup();
#endif
        exit(EXIT_FAILURE);
    }

    /* Opzione per riutilizzare l'indirizzo */
    opt = 1;
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0) {
        printf("Errore setsockopt: %d\n", GET_ERROR());
        CLOSE_SOCKET(server_sockfd);
#ifdef _WIN32
        WSACleanup();
#endif
        exit(EXIT_FAILURE);
    }

    /* Configurazione indirizzo server */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    /* Bind del socket */
    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Errore nel bind: %d\n", GET_ERROR());
        CLOSE_SOCKET(server_sockfd);
#ifdef _WIN32
        WSACleanup();
#endif
        exit(EXIT_FAILURE);
    }

    /* Messa in ascolto */
    if (listen(server_sockfd, BACKLOG) < 0) {
        printf("Errore nel listen: %d\n", GET_ERROR());
        CLOSE_SOCKET(server_sockfd);
#ifdef _WIN32
        WSACleanup();
#endif
        exit(EXIT_FAILURE);
    }

    printf("Server TCP in ascolto sulla porta %d...\n", SERVER_PORT);

    /* 6. Ciclo infinito per accettare connessioni da piu' client */
    while (1) {
        client_addr_len = sizeof(client_addr);

        /* Accetta connessione dal client */
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (!SOCKET_VALID(client_sockfd)) {
            printf("Errore nell'accept: %d\n", GET_ERROR());
            continue;
        }

        /* 2. Ricezione messaggio iniziale "Hello" e visualizzazione IP client */
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client_sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            printf("Errore nella ricezione del messaggio Hello\n");
            CLOSE_SOCKET(client_sockfd);
            continue;
        }
        buffer[bytes_received] = '\0';

        /* Visualizza indirizzo IP del client */
        client_ip = inet_ntoa(client_addr.sin_addr);
        printf("Ricevuti dati dal client con indirizzo: %s\n", client_ip);
        printf("Messaggio ricevuto: %s\n", buffer);

        /* 4. Ricezione stringa dal client */
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client_sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            printf("Errore nella ricezione della stringa\n");
            CLOSE_SOCKET(client_sockfd);
            continue;
        }
        buffer[bytes_received] = '\0';

        /* Visualizzazione stringa ricevuta */
        printf("Stringa ricevuta dal client: %s\n", buffer);

        /* Rimozione vocali dalla stringa */
        remove_vowels(buffer, response);
        printf("Stringa senza vocali: %s\n", response);

        /* Invio risposta al client */
        if (send(client_sockfd, response, strlen(response), 0) < 0) {
            printf("Errore nell'invio della risposta: %d\n", GET_ERROR());
        }

        /* Chiusura connessione con il client */
        CLOSE_SOCKET(client_sockfd);
        printf("Connessione con il client chiusa. In attesa di nuove connessioni...\n\n");
    }

    /* Chiusura socket server (mai raggiunto) */
    CLOSE_SOCKET(server_sockfd);
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
