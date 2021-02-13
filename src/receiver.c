#include <time.h>
#include <stdio.h> // For IO operations
#include <stdlib.h> // Standard Library
#include <unistd.h> // Standard POSIX functions, like "close"
#include <string.h> // String manipulation for string packets etc
#include <sys/time.h> // For timeval struct
#include <sys/socket.h> // For socket programming 
#include <arpa/inet.h> // Provides definitions for internet operations

int status; // Used for error handling
const int SIZE = 8; // Length of strings like "Packet:1", hardcoded
const char* LOCAL_HOST = "127.0.0.1"; // Standard address for IPv4 loopback traffic

int main(int argc, char *argv[]){ 

    srand(time(NULL)); // Set seed

    // Check no. of args specified:
    if (argc != 4){ // 1+4 args are needed
        printf("Usage: %s <ReceiverPort> <SenderPort> <PacketDropProbability>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int rport = atoi(argv[1]); // Receiver's port
    int sport = atoi(argv[2]); // Sender' port
    double drop_prob = atof(argv[3]); // Probability of dropping the received packet

    // Create a UDP socket (a socket descriptor) for the receiver:
    int sockfd = socket(
        AF_INET,    // IPv4
        SOCK_DGRAM, // Specifies UDP socket
        0           // 0: Default value for the protocol parameter
    );
    if (sockfd < 0){
        perror("(Receiver) Error while creating the socket");
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
        perror("(Receiver) An error occured while setting the socket options");
        exit(EXIT_FAILURE);
    }
    
    // Create a struct for the sender's address:
    struct sockaddr_in sen_addr;
    memset(&sen_addr, 0, sizeof(sen_addr)); 

    // Create a struct for the receiver's address:
    struct sockaddr_in rec_addr; // Specifies the receiver/destination address
    memset(&rec_addr, 0, sizeof(rec_addr)); 
    rec_addr.sin_family = AF_INET; 
    rec_addr.sin_port = htons(rport); 
    rec_addr.sin_addr.s_addr =  inet_addr(LOCAL_HOST);
    
    // Bind receiver to receiver's port:
    status = bind(
        sockfd, 
        (const struct sockaddr*) &rec_addr, 
        sizeof(struct sockaddr_in)
    );
    if (status != 0){
        perror("(Receiver) Bind failed"); 
        exit(EXIT_FAILURE); 
    }
    
    // Listen for the sender's message and store it:
    int len_saddr = sizeof(sen_addr);
    char message_rec[SIZE]; // Stores message received from sender
    int n = recvfrom(
        // Socket:
        sockfd, 
        // Store the received packet:
        message_rec, 
        SIZE, // Length of the received message_sent buffer
        // Flags:
        MSG_WAITALL,
        // Struct containing src address is returned: 
        (struct sockaddr *) &sen_addr, 
        &len_saddr
    );
    if (n != -1){
        message_rec[n] = '\0'; // A C string is a char array with a binary zero (\0) as the final char
        printf("(Receiver) Received: %s\n", message_rec);
    }
    else{
        printf("(Receiver) Packet not received\n");
        exit(EXIT_FAILURE);
    }

    // Generate a random number in [0,1] and send ACK appropriately
    double flag = ((double)rand())/((double)RAND_MAX);
    printf("(Receiver) Drop flag: %f\n", flag);
    if (flag >= drop_prob){
        // Send the ACK to the sender:
        char *ack = "Acknowledgment:1";
        sendto(
            // Socket:
            sockfd, 
            // Data to be sent:
            ack,
            strlen(ack), // Size of the ack to be sent
            // Flags:
            MSG_CONFIRM,
            // Destination Address:
            (const struct sockaddr *) &sen_addr,
            sizeof(sen_addr) // Size of the destination address struct
        ); 
        printf("(Receiver) Sent: %s\n", ack);
    }
    
    return 0; 
} 
