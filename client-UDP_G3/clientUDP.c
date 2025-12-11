/*
 * Client UDP - Compatibile Windows e UNIX (retrocompatibile C89)
 */
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

int main() {
    socket_t clientSocket;
    struct sockaddr_in serverAddr;
    struct hostent *host;
    char serverHostname[256];
    int port;
    char buffer[BUFFER_SIZE];
    char inputString[BUFFER_SIZE];
    socklen_t serverAddrLen;
    int bytesReceived;
    char serverIP[16];
    char helloMsg[6];
    size_t len;
#ifdef _WIN32
    WSADATA wsaData;
#endif

    serverAddrLen = sizeof(serverAddr);

#ifdef _WIN32
    /* Inizializza Winsock */
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Errore nell'inizializzazione di Winsock. Codice errore: %d\n", WSAGetLastError());
        return 1;
    }
#endif

    /* Legge il nome dell'host del server */
    printf("Inserire il nome dell'host del server: ");
    scanf("%255s", serverHostname);

    /* Legge il numero di porta del server */
    printf("Inserire il numero di porta del server: ");
    scanf("%d", &port);
    getchar(); /* Consuma il newline rimasto nel buffer */

    /* Crea socket UDP */
    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (!SOCKET_VALID(clientSocket)) {
        printf("Errore nella creazione del socket. Codice errore: %d\n", GET_ERROR());
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    /* Risolve il nome dell'host usando gethostbyname */
    host = gethostbyname(serverHostname);
    if (host == NULL) {
        printf("Errore nella risoluzione del nome host. Codice errore: %d\n", GET_ERROR());
        CLOSE_SOCKET(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    /* Configura l'indirizzo del server */
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons((unsigned short)port);
    memcpy(&serverAddr.sin_addr, host->h_addr_list[0], (size_t)host->h_length);

    /* Ottiene l'indirizzo IP del server per la visualizzazione */
    strcpy(serverIP, inet_ntoa(serverAddr.sin_addr));

    printf("\nConnessione al server %s (%s) sulla porta %d\n\n", serverHostname, serverIP, port);

    /* Invia il messaggio iniziale "Hello" */
    strcpy(helloMsg, "Hello");
    if (sendto(clientSocket, helloMsg, strlen(helloMsg), 0,
               (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Errore nell'invio del messaggio Hello. Codice errore: %d\n", GET_ERROR());
        CLOSE_SOCKET(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    printf("Messaggio 'Hello' inviato al server.\n");

    /* Legge la stringa da inviare */
    printf("Inserire la stringa da inviare al server: ");
    fgets(inputString, BUFFER_SIZE, stdin);

    /* Rimuove il newline finale se presente */
    len = strlen(inputString);
    if (len > 0 && inputString[len - 1] == '\n') {
        inputString[len - 1] = '\0';
    }

    /* Invia la stringa al server */
    if (sendto(clientSocket, inputString, strlen(inputString), 0,
               (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Errore nell'invio della stringa. Codice errore: %d\n", GET_ERROR());
        CLOSE_SOCKET(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    printf("Stringa '%s' inviata al server.\n", inputString);

    /* Riceve la risposta dal server */
    memset(buffer, 0, BUFFER_SIZE);
    bytesReceived = recvfrom(clientSocket, buffer, BUFFER_SIZE - 1, 0,
                              (struct sockaddr *)&serverAddr, &serverAddrLen);

    if (bytesReceived < 0) {
        printf("Errore nella ricezione della risposta. Codice errore: %d\n", GET_ERROR());
        CLOSE_SOCKET(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    buffer[bytesReceived] = '\0';

    /* Visualizza la risposta con le informazioni del server */
    printf("\nStringa \"%s\" ricevuta dal server nome: %s   indirizzo: %s\n",
           buffer, serverHostname, serverIP);

    /* Chiude il socket */
    CLOSE_SOCKET(clientSocket);
#ifdef _WIN32
    WSACleanup();
#endif

    printf("\nClient terminato.\n");

    return 0;
}
