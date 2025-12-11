#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctype.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024
#define BACKLOG 5

// Funzione per verificare se un carattere è una vocale
int is_vowel(char c) {
    char lower = tolower(c);
    return (lower == 'a' || lower == 'e' || lower == 'i' || lower == 'o' || lower == 'u');
}

// Funzione per rimuovere le vocali da una stringa
void remove_vowels(const char *input, char *output) {
    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (!is_vowel(input[i])) {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

int main() {
    SOCKET server_sockfd, client_sockfd;
    WSADATA wsaData;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Inizializzazione Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Errore nell'inizializzazione di Winsock\n");
        exit(EXIT_FAILURE);
    }

    // Creazione socket TCP
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd == INVALID_SOCKET) {
        printf("Errore nella creazione del socket: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Opzione per riutilizzare l'indirizzo
    char opt = 1;
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == SOCKET_ERROR) {
        printf("Errore setsockopt: %d\n", WSAGetLastError());
        closesocket(server_sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Configurazione indirizzo server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind del socket
    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Errore nel bind: %d\n", WSAGetLastError());
        closesocket(server_sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Messa in ascolto
    if (listen(server_sockfd, BACKLOG) == SOCKET_ERROR) {
        printf("Errore nel listen: %d\n", WSAGetLastError());
        closesocket(server_sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server TCP in ascolto sulla porta %d...\n", SERVER_PORT);

    // 6. Ciclo infinito per accettare connessioni da più client
    while (1) {
        client_addr_len = sizeof(client_addr);

        // Accetta connessione dal client
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sockfd == INVALID_SOCKET) {
            printf("Errore nell'accept: %d\n", WSAGetLastError());
            continue;
        }

        // 2. Ricezione messaggio iniziale "Hello" e visualizzazione IP client
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            printf("Errore nella ricezione del messaggio Hello\n");
            closesocket(client_sockfd);
            continue;
        }
        buffer[bytes_received] = '\0';

        // Visualizza indirizzo IP del client
        char *client_ip = inet_ntoa(client_addr.sin_addr);
        printf("Ricevuti dati dal client con indirizzo: %s\n", client_ip);
        printf("Messaggio ricevuto: %s\n", buffer);

        // 4. Ricezione stringa dal client
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client_sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            printf("Errore nella ricezione della stringa\n");
            closesocket(client_sockfd);
            continue;
        }
        buffer[bytes_received] = '\0';

        // Visualizzazione stringa ricevuta
        printf("Stringa ricevuta dal client: %s\n", buffer);

        // Rimozione vocali dalla stringa
        remove_vowels(buffer, response);
        printf("Stringa senza vocali: %s\n", response);

        // Invio risposta al client
        if (send(client_sockfd, response, strlen(response), 0) == SOCKET_ERROR) {
            printf("Errore nell'invio della risposta: %d\n", WSAGetLastError());
        }

        // Chiusura connessione con il client
        closesocket(client_sockfd);
        printf("Connessione con il client chiusa. In attesa di nuove connessioni...\n\n");
    }

    // Chiusura socket server (mai raggiunto)
    closesocket(server_sockfd);
    WSACleanup();

    return 0;
}
