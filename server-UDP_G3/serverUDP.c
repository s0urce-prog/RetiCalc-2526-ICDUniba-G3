#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

// Funzione per verificare se un carattere e' una vocale
int isVowel(char c) {
    char lower = (c >= 'A' && c <= 'Z') ? c + 32 : c;
    return (lower == 'a' || lower == 'e' || lower == 'i' || lower == 'o' || lower == 'u');
}

// Funzione per rimuovere le vocali da una stringa
void removeVowels(const char *input, char *output) {
    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (!isVowel(input[i])) {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];
    char outputBuffer[BUFFER_SIZE];
    int port;
    int bytesReceived;

    // Inizializza Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Errore nell'inizializzazione di Winsock. Codice errore: %d\n", WSAGetLastError());
        return 1;
    }

    // Legge il numero di porta da tastiera
    printf("Inserire il numero di porta del server: ");
    scanf("%d", &port);

    // Crea socket UDP
    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
        printf("Errore nella creazione del socket. Codice errore: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Configura l'indirizzo del server
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    // Associa il socket all'indirizzo
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Errore nel bind. Codice errore: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Server UDP in ascolto sulla porta %d...\n", port);

    // Loop principale del server
    while (1) {
        // Riceve dati dal client
        memset(buffer, 0, BUFFER_SIZE);
        bytesReceived = recvfrom(serverSocket, buffer, BUFFER_SIZE - 1, 0,
                                  (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (bytesReceived == SOCKET_ERROR) {
            printf("Errore nella ricezione. Codice errore: %d\n", WSAGetLastError());
            continue;
        }

        buffer[bytesReceived] = '\0';

        // Ottiene il nome host del client
        char clientHostname[256];
        char clientIP[16];

        // Ottiene l'indirizzo IP del client usando inet_ntoa (compatibile con vecchie versioni)
        strcpy(clientIP, inet_ntoa(clientAddr.sin_addr));

        // Prova a risolvere il nome host del client tramite gethostbyaddr
        struct hostent *hostInfo = gethostbyaddr((char *)&clientAddr.sin_addr,
                                                   sizeof(clientAddr.sin_addr), AF_INET);
        if (hostInfo != NULL) {
            strcpy(clientHostname, hostInfo->h_name);
        } else {
            strcpy(clientHostname, clientIP); // Se fallisce, usa l'IP come nome
        }

        // Visualizza informazioni sul client
        printf("Ricevuti dati dal client nome: %s   indirizzo: %s\n", clientHostname, clientIP);

        // Se e' il messaggio "Hello", lo ignora e continua ad aspettare
        if (strcmp(buffer, "Hello") == 0) {
            printf("Messaggio iniziale 'Hello' ricevuto. In attesa della stringa...\n");
            continue;
        }

        // Visualizza la stringa ricevuta
        printf("Stringa ricevuta: %s\n", buffer);

        // Rimuove le vocali dalla stringa
        removeVowels(buffer, outputBuffer);
        printf("Stringa senza vocali: %s\n", outputBuffer);

        // Invia la stringa modificata al client
        if (sendto(serverSocket, outputBuffer, strlen(outputBuffer), 0,
                   (struct sockaddr *)&clientAddr, clientAddrLen) == SOCKET_ERROR) {
            printf("Errore nell'invio. Codice errore: %d\n", WSAGetLastError());
        } else {
            printf("Risposta inviata al client.\n");
        }

        printf("\nIn attesa di altri dati...\n\n");
    }

    // Chiude il socket e pulisce Winsock
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
