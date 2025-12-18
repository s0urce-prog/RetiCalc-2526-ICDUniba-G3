#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define BUFFER_SIZE 1024

/* Funzione per verificare se un carattere e' una vocale */
int isVowel(char c) {
    char lower;
    lower = (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
    return (lower == 'a' || lower == 'e' || lower == 'i' || lower == 'o' || lower == 'u');
}

/* Funzione per rimuovere le vocali da una stringa */
void removeVowels(const char *input, char *output) {
    int i, j;
    j = 0;
    for (i = 0; input[i] != '\0'; i++) {
        if (!isVowel(input[i])) {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

int main() {
    socket_t serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen;
    char buffer[BUFFER_SIZE];
    char outputBuffer[BUFFER_SIZE];
    int port;
    int bytesReceived;
    char clientHostname[256];
    char clientIP[16];
    struct hostent *hostInfo;
#ifdef _WIN32
    WSADATA wsaData;
#endif

    clientAddrLen = sizeof(clientAddr);

#ifdef _WIN32
    /* Inizializza Winsock */
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Errore nell'inizializzazione di Winsock. Codice errore: %d\n", WSAGetLastError());
        return 1;
    }
#endif

    /* Legge il numero di porta da tastiera */
    printf("Inserire il numero di porta del server: ");
    scanf("%d", &port);

    /* Crea socket UDP */
    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (!SOCKET_VALID(serverSocket)) {
        printf("Errore nella creazione del socket. Codice errore: %d\n", GET_ERROR());
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    /* Configura l'indirizzo del server */
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons((unsigned short)port);

    /* Associa il socket all'indirizzo */
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Errore nel bind. Codice errore: %d\n", GET_ERROR());
        CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    printf("Server UDP in ascolto sulla porta %d...\n", port);

    /* Loop principale del server */
    while (1) {
        /* Riceve dati dal client */
        memset(buffer, 0, BUFFER_SIZE);
        clientAddrLen = sizeof(clientAddr);
        bytesReceived = recvfrom(serverSocket, buffer, BUFFER_SIZE - 1, 0,
                                  (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (bytesReceived < 0) {
            printf("Errore nella ricezione. Codice errore: %d\n", GET_ERROR());
            continue;
        }

        buffer[bytesReceived] = '\0';

        /* Ottiene l'indirizzo IP del client usando inet_ntoa (compatibile con vecchie versioni) */
        strcpy(clientIP, inet_ntoa(clientAddr.sin_addr));

        /* Prova a risolvere il nome host del client tramite gethostbyaddr */
        hostInfo = gethostbyaddr((char *)&clientAddr.sin_addr,
                                  sizeof(clientAddr.sin_addr), AF_INET);
        if (hostInfo != NULL) {
            strcpy(clientHostname, hostInfo->h_name);
        } else {
            strcpy(clientHostname, clientIP); /* Se fallisce, usa l'IP come nome */
        }

        /* Visualizza informazioni sul client */
        printf("Ricevuti dati dal client nome: %s   indirizzo: %s\n", clientHostname, clientIP);

        /* Se e' il messaggio "Hello", lo ignora e continua ad aspettare */
        if (strcmp(buffer, "Hello") == 0) {
            printf("Messaggio iniziale 'Hello' ricevuto. In attesa della stringa...\n");
            continue;
        }

        /* Visualizza la stringa ricevuta */
        printf("Stringa ricevuta: %s\n", buffer);

        /* Rimuove le vocali dalla stringa */
        removeVowels(buffer, outputBuffer);
        printf("Stringa senza vocali: %s\n", outputBuffer);

        /* Invia la stringa modificata al client */
        if (sendto(serverSocket, outputBuffer, strlen(outputBuffer), 0,
                   (struct sockaddr *)&clientAddr, clientAddrLen) < 0) {
            printf("Errore nell'invio. Codice errore: %d\n", GET_ERROR());
        } else {
            printf("Risposta inviata al client.\n");
        }

        printf("\nIn attesa di altri dati...\n\n");
    }

    /* Chiude il socket */
    CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
