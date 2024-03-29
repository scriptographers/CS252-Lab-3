#include <stdio.h> // For IO operations
#include <stdlib.h> // Standard Library
#include <unistd.h> // Standard POSIX functions, like "close"
#include <string.h> // String manipulation for string packets etc
#include <sys/time.h> // For timeval struct
#include <sys/socket.h> // For socket programming
#include <arpa/inet.h> // Provides definitions for internet operations

int status; // Used for error handling
int SIZE = 16;   // Length of strings like "Acknowledgment:1", hardcoded
const char* LOCAL_HOST = "127.0.0.1"; // Standard address for IPv4 loopback traffic

int main(int argc, char *argv[]){

    // Check no. of args specified:
    if (argc != 5){ // 1+4 args are needed
        printf("Usage: %s <SenderPort> <ReceiverPort> <RetransmissionTimer> <NoOfPacketsToBeSent>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int sport   = atoi(argv[1]); // Sender' port, ascii to integer
    int rport   = atoi(argv[2]); // Receiver's port
    int timeout = atoi(argv[3]); // Timeout specified in seconds
    int n_pkts  = atoi(argv[4]); // No. of packets to be sent

    // Initialize the sender's log file
    FILE *s_log;
    s_log = fopen("sender.txt", "w+"); // This also flushes any content already present

    // Create a UDP socket (a socket descriptor) for the sender:
    int sockfd = socket(
        AF_INET,    // IPv4
        SOCK_DGRAM, // Specifies UDP socket
        IPPROTO_UDP // UDP protocol, setting this to 0 also works
    );
    if (sockfd < 0){
        perror("(Sender) Error while creating the socket");
        exit(EXIT_FAILURE);
    }

    // Bind the sender explicitly to a given port:
    struct sockaddr_in sen_addr; // Specifies the sender's address
    memset(&sen_addr, 0, sizeof(sen_addr));
    sen_addr.sin_family = AF_INET;
    sen_addr.sin_port = htons(sport); // htons: https://stackoverflow.com/questions/19207745/htons-function-in-socket-programing
    sen_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);

    status = bind(
        sockfd,
        (const struct sockaddr*) &sen_addr,
        sizeof(struct sockaddr_in)
    );
    if (status != 0){
        perror("(Sender) Bind failed");
        exit(EXIT_FAILURE);
    }

    // Specify retransmission timeout: Reference: https://stackoverflow.com/questions/13547721/udp-socket-set-timeout
    struct timeval tv;
    tv.tv_sec  = timeout; // in seconds
    tv.tv_usec = 0; // in microseconds

    // Set the sock options to specify timeout:
    status = setsockopt(
        // The socket:
        sockfd,
        // The socket layer, more info: https://stackoverflow.com/questions/21515946/what-is-sol-socket-used-for
        SOL_SOCKET,
        // This option allows us to set a timeout
        SO_RCVTIMEO,
        // Timeout needs to be specified as a timeval struct
        &tv,
        sizeof(tv)
    );
    if (status != 0){
        perror("(Sender) An error occured while setting the timeout");
        exit(EXIT_FAILURE);
    }

    // Set sock options to completely close the socket, for immediate reuse:
    int reuse_flag = 1;
    status = setsockopt(
        // The socket:
        sockfd,
        // The socket layer, more info: https://stackoverflow.com/questions/21515946/what-is-sol-socket-used-for
        SOL_SOCKET,
        // This option allows your server to bind to an address which is in a TIME_WAIT state 
        SO_REUSEADDR,
        // Setting to true
        &reuse_flag,
        sizeof(int)
    );
    if (status != 0){
        perror("(Sender) An error occured while setting the socket options");
        exit(EXIT_FAILURE);
    }

    // Create the struct for the receiver's address:
    struct sockaddr_in rec_addr; // Specifies the receiver/destination address
    memset(&rec_addr, 0, sizeof(rec_addr));
    rec_addr.sin_family = AF_INET;
    rec_addr.sin_port = htons(rport);
    rec_addr.sin_addr.s_addr =  inet_addr(LOCAL_HOST);
    
    int i_pkt = 1;
    int i_ack = 2;
    while (1){

        // Create the data in an appropriate format
        char temp_pkt[128];
        sprintf(temp_pkt, "%d", i_pkt);
        char message_sent[8] = "Packet:";
        strcat(message_sent, temp_pkt);

        // Send the data to the receiver:
        status = sendto(
            // Socket:
            sockfd,
            // Data to be sent:
            message_sent,
            strlen(message_sent), // Size of the data to be sent
            // Flags:
            MSG_CONFIRM, // Tells the link layer that we you got a successful reply from the other side, reference: https://stackoverflow.com/questions/16594387/why-should-i-use-or-not-use-msg-confirm
            // Destination Address:
            (const struct sockaddr *) &rec_addr,
            sizeof(rec_addr) // Size of the destination address struct
        );
        if (status != -1){
            printf("(Sender) Sent: %s\n", message_sent);
            // Also log this
            fprintf(s_log, "Sent: %s\n", message_sent);
        }
        else{
            // Packet not sent
            perror("(Sender) An error occured while sending the data");
            exit(EXIT_FAILURE);
        }

        // Wait for receiver's response:
        int len_raddr = sizeof(rec_addr);

        SIZE = 64; // Reset size
        char message_rec[SIZE];
        memset(message_rec, '\0', sizeof(message_rec));

        status = recvfrom(
            // Socket:
            sockfd,
            // Store the received ACK:
            message_rec,
            sizeof(message_rec), // Length of the received message_sent buffer
            // Flags: None
            0,
            // Struct containing src address is returned: 
            (struct sockaddr *) &rec_addr,
            &len_raddr
        );
        if (status != -1){
            // Reset SIZE depending on received ack's length
            SIZE = strlen(message_rec);
            message_rec[SIZE] = '\0'; // A C string is a char array with a binary zero (\0) as the final char
            printf("(Sender) Received: '%s' from IP: %s and Port: %i\n\n", message_rec, inet_ntoa(rec_addr.sin_addr), ntohs(rec_addr.sin_port));

            // Check whether ACK received has sequence number = i_pkt + 1
            char temp_1[64];
            strcpy(temp_1, message_rec + 15);
            i_ack = atoi(temp_1);

            if (i_ack == i_pkt + 1){
                // Received the proper ACK, increment i_pkt
                i_pkt++;
                fprintf(s_log, "\n");
            }
            else{
                // Received a wrong ACK
                printf("(Sender) Received a different ACK, ignoring it\n");
            }

            // Check whether the required number of packets have been transmitted:
            if (i_pkt > n_pkts){
                printf("(Common) All packets sent and received successfully. Press CTRL+C to stop.\n");
                break;
            }
        }
        else{
            // ACK not received, will automatically retransmit due to the while loop
            printf("(Sender) ACK not received, sender timed out, will retransmit\n\n");
            fprintf(s_log, "Retransmission timer expired\n\n");
        }
    }

    // Close the socket:
    close(sockfd);
    // Close the log file:
    fclose(s_log);

    return 0;
}