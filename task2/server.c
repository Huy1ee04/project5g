// server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024
#define SERVER_PORT 54321
#define DRX_CYCLE 320
#define NPF 4

typedef struct
{
    int message_id;
    int ue_id;
    int tai;
} ngap_paging_t;

typedef struct
{
    int message_id;
    int sfn;
    int pagingRecordList[10]; // Example size, adjust accordingly
} rrc_paging_t;

typedef struct
{
    int message_id;
    int sfn;
} mib_message_t;

// Global variables
int sfn = 0;               // System Frame Number (SFN)
int queueSize[1024] = {0}; // Example queue size, adjust accordingly
pthread_mutex_t lock;

// Function Prototypes
void message_handler(ngap_paging_t *paging);
int enqueueTMSI(int qid, int ueid);
int dequeueTMSI(int sfn, int *pagingRecordList);
void *tcp_server(void *arg);

int main()
{
    // Start the TCP server in a separate thread
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, tcp_server, NULL);

    pthread_join(server_thread, NULL); // Wait for the server thread to finish

    return 0;
}

void *tcp_server(void *arg)
{
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Create TCP socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address and port for gNB
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Set sockopt to handle errno 98 (address already in use)
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind socket to address and port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_sock, 5) < 0)
    {
        perror("listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    printf("gNB TCP server is listening on port %d...\n", SERVER_PORT);

    // Accept connections and receive messages
    while ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len)) > 0)
    {
        printf("Connected to AMF\n");

        while ((bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0)
        {
            buffer[bytes_received] = '\0';
            ngap_paging_t paging;
            memcpy(&paging, buffer, sizeof(ngap_paging_t));
            message_handler(&paging);
        }
        close(client_sock);
    }

    close(server_sock);
    return NULL;
}

void message_handler(ngap_paging_t *paging)
{
    int ueid_temp = paging->ue_id;
    int drxCycle = DRX_CYCLE;
    int nPF = NPF;
    int SFN_temp = sfn;
    int remainder = (drxCycle / nPF) * ((ueid_temp % 1024) % nPF);
    int qid = 0, offset = 0;

    while (1)
    {
        SFN_temp = sfn;

        if (remainder > (SFN_temp % drxCycle))
        {
            qid = (SFN_temp + (remainder - (SFN_temp % drxCycle)) + offset * drxCycle) % 1024;
        }
        else if (remainder == (SFN_temp % drxCycle))
        {
            qid = (SFN_temp + (remainder - (SFN_temp % drxCycle)) + (offset + 1) * drxCycle) % 1024;
        }
        else
        {
            qid = (SFN_temp + (drxCycle + (remainder - (SFN_temp % drxCycle))) + offset * drxCycle) % 1024;
        }

        if (enqueueTMSI(qid, ueid_temp) == 0)
        {
            printf("[SFN=%u] Queue ID = %d, TMSI = %d\n", SFN_temp, qid, ueid_temp);
            break;
        }
        else
        {
            offset++;
        }
    }
}

// Dummy implementations of enqueueTMSI and dequeueTMSI
int enqueueTMSI(int qid, int ueid)
{
    // Implement logic to enqueue the UE in the queue associated with qid
    queueSize[qid]++;
    return 0; // Return 0 on success
}

int dequeueTMSI(int sfn, int *pagingRecordList)
{
    // Implement logic to dequeue UEs from the queue associated with the SFN
    if (queueSize[sfn] > 0)
    {
        pagingRecordList[0] = 123; // Example UE ID, replace with actual logic
        queueSize[sfn]--;
        return 1; // Return the number of UEs dequeued
    }

    return 0; // No UEs dequeued
}
