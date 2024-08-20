// client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024
#define GNB_PORT 54321
#define GNB_IP "127.0.0.1"
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
} mib_message_t;

typedef struct
{
    int message_id;
    int sfn;
    int pagingRecordList[10]; // Example size, adjust accordingly
} rrc_paging_t;

// Global variables
int sfn = 0; // System Frame Number (SFN)
struct sockaddr_in broadcast_addr;
pthread_mutex_t lock;

// Function Prototypes
void send_paging_message(int sock);
void *sfn_scheduler(void *arg);

int main()
{
    int sock;
    struct sockaddr_in gnb_addr;
    char command[BUFFER_SIZE];

    // Create TCP socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configure gNB address
    memset(&gnb_addr, 0, sizeof(gnb_addr));
    gnb_addr.sin_family = AF_INET;
    gnb_addr.sin_port = htons(GNB_PORT);
    if (inet_pton(AF_INET, GNB_IP, &gnb_addr.sin_addr) <= 0)
    {
        perror("inet_pton failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Connect to gNB
    if (connect(sock, (struct sockaddr *)&gnb_addr, sizeof(gnb_addr)) < 0)
    {
        perror("connect failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to gNB at %s:%d\n", GNB_IP, GNB_PORT);

    // Start the SFN scheduler in a separate thread
    pthread_t scheduler_thread;
    pthread_create(&scheduler_thread, NULL, sfn_scheduler, (void *)&sock);

    // Main client loop to send commands
    while (1)
    {
        printf("Enter command: ");
        if (fgets(command, sizeof(command), stdin) != NULL)
        {
            command[strcspn(command, "\n")] = 0; // Remove newline character
            if (strcmp(command, "send") == 0)
            {
                send_paging_message(sock);
            }
            else
            {
                printf("Unknown command\n");
            }
        }
        else
        {
            perror("Error reading command");
        }
    }

    close(sock);
    return 0;
}

void send_paging_message(int sock)
{
    ngap_paging_t paging;
    paging.message_id = 100;
    paging.ue_id = 123;
    paging.tai = 45204;

    // Send NGAP Paging to gNB
    if (send(sock, &paging, sizeof(paging), 0) < 0)
    {
        perror("send failed");
    }
    else
    {
        printf("Sent NGAP Paging to gNB: message_id=%d, ue_id=%d, tai=%d\n", paging.message_id, paging.ue_id, paging.tai);
    }
}

void *sfn_scheduler(void *arg)
{
    int sock = *(int *)arg;
    struct sockaddr_in broadcast_addr;
    int queueSize[1024] = {0}; // Example queue size, adjust accordingly
    mib_message_t mib_message;

    // Configure broadcast address
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(12345); // Adjust port as needed
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    while (1)
    {
        // Broadcast MIB when SFN is divisible by 8
        pthread_mutex_lock(&lock);
        if (sfn % 8 == 0)
        {
            mib_message.message_id = 200;
            mib_message.sfn = sfn;

            if (sendto(sock, &mib_message, sizeof(mib_message), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0)
            {
                perror("sendto failed");
                close(sock);
                exit(EXIT_FAILURE);
            }

            printf("Broadcasted MIB: message_id=%d, sfn=%u\n", mib_message.message_id, mib_message.sfn);
        }
        pthread_mutex_unlock(&lock);

        // Increment SFN and reset every 1024 frames
        pthread_mutex_lock(&lock);
        sfn = (sfn + 1) % 1024;
        pthread_mutex_unlock(&lock);

        sleep(1); // 1-second delay for each SFN increment
    }

    return NULL;
}
