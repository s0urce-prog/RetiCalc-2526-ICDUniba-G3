#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    SOCKET sockfd;
    WSADATA wsaData;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

    // Verifica argomenti (indirizzo IP del server)
    if (argc != 2) {
        printf("Uso: %s <indirizzo_IP_server>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Inizializzazione Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Errore nell'inizializzazione di Winsock\n");
        exit(EXIT_FAILURE);
    }

    // Creazione socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Errore nella creazione del socket: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Configurazione indirizzo server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    // Conversione indirizzo IP
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    if (server_addr.sin_addr.s_addr == INADDR_NONE) {
        printf("Indirizzo IP non valido\n");
        closesocket(sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Connessione al server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Errore nella connessione al server: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Connesso al server %s:%d\n", argv[1], SERVER_PORT);

    // 1. Invio messaggio iniziale "Hello" al server
    strcpy(buffer, "Hello");
    if (send(sockfd, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
        printf("Errore nell'invio del messaggio Hello: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    printf("Inviato messaggio: %s\n", buffer);

    // 3. Lettura stringa dallo standard input e invio al server
    printf("Inserisci una stringa: ");
    if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
        printf("Errore nella lettura dell'input\n");
        closesocket(sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Rimuove il newline finale
    input[strcspn(input, "\n")] = '\0';

    // Invio della stringa al server
    if (send(sockfd, input, strlen(input), 0) == SOCKET_ERROR) {
        printf("Errore nell'invio della stringa: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    printf("Stringa inviata: %s\n", input);

    // 5. Ricezione risposta dal server (stringa senza vocali)
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received == SOCKET_ERROR) {
        printf("Errore nella ricezione della risposta: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0';

    // Visualizzazione risposta
    printf("Risposta dal server (senza vocali): %s\n", buffer);

    // Chiusura connessione
    closesocket(sockfd);
    WSACleanup();
    printf("Connessione chiusa.\n");

    return 0;
}
