#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/time.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include "../shared/structs.h"
#include "../shared/session.h"
#include "../shared/structured_log.h"

// #define PORT 23
// #define DELAY_MS 100
// #define HEARTBEAT_INTERVAL_MS 600000 // 10 minutes
// #define FD_LIMIT 4096
#define SERVER_ID "Telnet"

#define IAC 255
#define DO 253
#define DONT 254
#define WILL 251
#define WONT 252

int port;
int delay;
int maxNoClients;
const char *eventhorizonMode = "baseline";

// Telnet negotiation options
unsigned char negotiations[][3] = {
    {IAC, WILL, 1}, 
    {IAC, DO, 3}, 
    {IAC, DONT, 5},
    {IAC, WILL, 31}, 
    {IAC, DO, 24}, 
    {IAC, WONT, 39}
};
int num_options = sizeof(negotiations) / sizeof(negotiations[0]);

// void heartbeatLog() {
//     syslog(LOG_INFO, "Server is running with %d connected clients. Number of most concurrent connected clients is %d", clientQueueTelnet.length, statsTelnet.mostConcurrentConnections);
//     syslog(LOG_INFO, "Current statistics: wasted time: %lld ms. Total connected clients: %ld", statsTelnet.totalWastedTime, statsTelnet.totalConnects);
// }

void initializeStats(){
    statsTelnet.totalConnects = 0;
    statsTelnet.totalWastedTime = 0;
    statsTelnet.mostConcurrentConnections = 0;
}

int deceptionDelayMs(long long timeConnectedMs) {
    int delayMs = delay + (int)(timeConnectedMs / 10000);
    int maxDelayMs = delay + 250;

    if (delayMs > maxDelayMs) {
        return maxDelayMs;
    }
    return delayMs;
}

void logTelnetDisconnect(struct telnetAndUpnpClient *client, long long now, const char *reason) {
    long long durationMs = -1;

    if (client->started_at_ms > 0 && now >= client->started_at_ms) {
        durationMs = now - client->started_at_ms;
    } else if (client->base.timeConnected >= 0) {
        durationMs = client->base.timeConnected;
    }

    structured_log_event(
        "disconnect",
        "telnet",
        client->session_id,
        client->base.ipaddr,
        eventhorizonMode,
        "none",
        "none",
        durationMs,
        reason ? reason : "unknown",
        -1
    );
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    
    // testing
    // char msg[256];
    // snprintf(msg, sizeof(msg), "%s connect %s\n",
    //     SERVER_ID, "82.211.213.247");
    // fprintf(stderr, "%s", msg);
    // sendMetric(msg);
    (void)argc;
    port = atoi(argv[1]);
    delay = atoi(argv[2]);
    maxNoClients = atoi(argv[3]);
    const char *configuredMode = getenv("EVENTHORIZON_MODE");
    if (configuredMode && strcmp(configuredMode, "deception") == 0) {
        eventhorizonMode = "deception";
    }
    structured_log_init(NULL);
    initializeStats();
    setFdLimit(maxNoClients);
    signal(SIGPIPE, SIG_IGN); // Ignore 
    queue_init(&clientQueueTelnet);
    
    int serverSock = createServer(port);
    if (serverSock < 0) {
        fprintf(stderr, "Invalid server socket fd: %d", serverSock);
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    
    struct pollfd fds;
    memset(&fds, 0, sizeof(fds));
    fds.fd = serverSock;
    fds.events = POLLIN;
    
    // long long lastHeartbeat = currentTimeMs();
    while (1) {
        long long now = currentTimeMs();
        int timeout = -1;

        // if (now - lastHeartbeat >= HEARTBEAT_INTERVAL_MS) {
        //     heartbeatLog();
        //     lastHeartbeat = now;
        // }

        // Process clients in queue
        while (clientQueueTelnet.head) {
            if(clientQueueTelnet.head->sendNext <= now){
                struct baseClient *bc = queue_pop(&clientQueueTelnet);
                struct telnetAndUpnpClient *c = (struct telnetAndUpnpClient *)bc;
                
                int optionIndex = rand() % num_options;
                ssize_t out = write(c->fd, negotiations[optionIndex], sizeof(negotiations[optionIndex]));
                
                if (out == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) { // Avoid blocking
                        c->base.sendNext = now + delay;
                        c->base.timeConnected += delay;
                        statsTelnet.totalWastedTime += delay;
                        queue_append(&clientQueueTelnet, (struct baseClient *)c);
                    } else {
                        long long timeTrapped = c->base.timeConnected;
                        char msg[256];
                        snprintf(msg, sizeof(msg), "%s disconnect %s %lld\n",
                            SERVER_ID, c->base.ipaddr, timeTrapped);
                        printf("%s", msg);
                        sendMetric(msg);
                        logTelnetDisconnect(c, now, "write_error");
                        close(c->fd);
                        free(c);
                    }
                } else {
                    c->base.sendNext = now + delay;
                    c->base.timeConnected += delay;
                    statsTelnet.totalWastedTime += delay;
                    queue_append(&clientQueueTelnet, (struct baseClient *)c);
                }
            } else {
                timeout = clientQueueTelnet.head->sendNext - now;
                break;
            }
        }
        
        int pollResult = poll(&fds, 1, timeout);
        now = currentTimeMs(); // Poll will cause old value to be misrepresenting
        if (pollResult < 0) {
            fprintf(stderr, "Poll error with error %s", strerror(errno));
            continue;
        }

        // Accept new connections
        if (fds.revents & POLLIN) {
            int clientFd = accept(serverSock, (struct sockaddr *)&clientAddr, &addrLen);
            if(clientFd == -1) {
                fprintf(stderr, "Failed accepting new client with error %s", strerror(errno));
                continue;
            }

            fcntl(clientFd, F_SETFL, O_NONBLOCK); // Set non-blocking mode
            struct telnetAndUpnpClient* newClient = malloc(sizeof(struct telnetAndUpnpClient));
            if (!newClient) {
                fprintf(stderr, "Out of memory");
                close(clientFd);
                continue;
            }

            statsTelnet.totalConnects += 1;
            newClient->fd = clientFd;
            newClient->base.type = TELNET_CLIENT;
            newClient->base.sendNext = now + delay;
            newClient->base.timeConnected = 0;
            snprintf(newClient->base.ipaddr, INET_ADDRSTRLEN, "%s", inet_ntoa(clientAddr.sin_addr));
            newClient->started_at_ms = now;
            make_session_id(
                newClient->session_id,
                sizeof(newClient->session_id),
                "telnet",
                newClient->base.ipaddr,
                newClient->started_at_ms
            );
            queue_append(&clientQueueTelnet, (struct baseClient*)newClient);

            if(statsTelnet.mostConcurrentConnections < clientQueueTelnet.length) {
                statsTelnet.mostConcurrentConnections = clientQueueTelnet.length;
            }

            char msg[256];
            snprintf(msg, sizeof(msg), "%s connect %s\n",
                SERVER_ID, newClient->base.ipaddr);
            printf("%s", msg);
            sendMetric(msg);

            structured_log_event(
                "connect",
                "telnet",
                newClient->session_id,
                newClient->base.ipaddr,
                eventhorizonMode,
                "none",
                "none",
                -1,
                NULL,
                -1
            );

            if (strcmp(eventhorizonMode, "deception") == 0) {
                structured_log_event(
                    "deception_applied",
                    "telnet",
                    newClient->session_id,
                    newClient->base.ipaddr,
                    eventhorizonMode,
                    "generic_iot_camera",
                    "camouflage_graduated",
                    -1,
                    NULL,
                    deceptionDelayMs(0)
                );
            }
        }
    }

    close(serverSock);
    return 0;
}
