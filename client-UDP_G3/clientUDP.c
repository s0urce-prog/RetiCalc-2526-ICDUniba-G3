#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    struct hostent *host;
    char serverHostname[256];
    int port;
    char buffer[BUFFER_SIZE];
    char inputString[BUFFER_SIZE];
    int serverAddrLen = sizeof(serverAddr);
    int bytesReceived;
    char serverIP[16];

    // Inizializza Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Errore nell'inizializzazione di Winsock. Codice errore: %d\n", WSAGetLastError());
        return 1;
    }

    // Legge il nome dell'host del server
    printf("Inserire il nome dell'host del server: ");
    scanf("%255s", serverHostname);

    // Legge il numero di porta del server
    printf("Inserire il numero di porta del server: ");
    scanf("%d", &port);
    getchar(); // Consuma il newline rimasto nel buffer

    // Crea socket UDP
    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET) {
        printf("Errore nella creazione del socket. Codice errore: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Risolve il nome dell'host usando gethostbyname
    host = gethostbyname(serverHostname);
    if (host == NULL) {
        printf("Errore nella risoluzione del nome host. Codice errore: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Configura l'indirizzo del server
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    memcpy(&serverAddr.sin_addr, host->h_addr_list[0], host->h_length);

    // Ottiene l'indirizzo IP del server per la visualizzazione
    strcpy(serverIP, inet_ntoa(serverAddr.sin_addr));

    printf("\nConnessione al server %s (%s) sulla porta %d\n\n", serverHostname, serverIP, port);

    // Invia il messaggio iniziale "Hello"
    const char *helloMsg = "Hello";
    if (sendto(clientSocket, helloMsg, strlen(helloMsg), 0,
               (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Errore nell'invio del messaggio Hello. Codice errore: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    printf("Messaggio 'Hello' inviato al server.\n");

    // Legge la stringa da inviare
    printf("Inserire la stringa da inviare al server: ");
    fgets(inputString, BUFFER_SIZE, stdin);

    // Rimuove il newline finale se presente
    size_t len = strlen(inputString);
    if (len > 0 && inputString[len - 1] == '\n') {
        inputString[len - 1] = '\0';
    }

    // Invia la stringa al server
    if (sendto(clientSocket, inputString, strlen(inputString), 0,
               (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Errore nell'invio della stringa. Codice errore: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    printf("Stringa '%s' inviata al server.\n", inputString);

    // Riceve la risposta dal server
    memset(buffer, 0, BUFFER_SIZE);
    bytesReceived = recvfrom(clientSocket, buffer, BUFFER_SIZE - 1, 0,
                              (struct sockaddr *)&serverAddr, &serverAddrLen);

    if (bytesReceived == SOCKET_ERROR) {
        printf("Errore nella ricezione della risposta. Codice errore: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    buffer[bytesReceived] = '\0';

    // Visualizza la risposta con le informazioni del server
    printf("\nStringa \"%s\" ricevuta dal server nome: %s   indirizzo: %s\n",
           buffer, serverHostname, serverIP);

    // Chiude il socket e pulisce Winsock
    closesocket(clientSocket);
    WSACleanup();

    printf("\nClient terminato.\n");

    return 0;
}
