#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> // Standard POSIX functions, like "close"
#include <string.h> 
#include <sys/time.h> // For timeval struct
#include <sys/socket.h> 
#include <arpa/inet.h> 

const int SIZE = 16;   // Length of strings like "Acknowledgment:1", hardcoded
const char* LOCAL_HOST = "127.0.0.1"; // Standard address for IPv4 loopback traffic

int main(int argc, char *argv[]){

    // Check no. of args specified:
    if (argc != 5){ // 1+4 args are needed
        printf("Usage: %s <SenderPort> <ReceiverPort> <RetransmissionTimer> <NoOfPacketsToBeSent>\n", argv[0]);
        exit(1);
    }
    int sport   = atoi(argv[1]); // Sender' port, ascii to integer
    int rport   = atoi(argv[2]); // Receiver's port
    int timeout = atoi(argv[3]); // Timeout specified in seconds
    int n_pkts  = atoi(argv[4]); // No. of packets to be sent

    // Create a UDP socket (a socket descriptor) for the sender:
    int sockfd = socket(
        AF_INET,    // IPv4
        SOCK_DGRAM, // Specifies UDP socket
        0           // 0: Default value for the protocol parameter
    );

    // To-do: https://www.geeksforgeeks.org/explicitly-assigning-port-number-client-socket/
    
    // Specify timeout: Reference: https://stackoverflow.com/questions/13547721/udp-socket-set-timeout
    struct timeval tv;
    tv.tv_sec  = timeout; // in seconds
    tv.tv_usec = 0; // in microseconds

    // Set the sock options to specify timeout:
    int status = setsockopt(
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
    if (status != 0)
        perror("An error occured while setting the timeout");

    // Create the struct for the receiver's address:
    struct sockaddr_in rec_addr; // Specifies the receiver/destination address
    memset(&rec_addr, 0, sizeof(rec_addr)); 
    rec_addr.sin_family = AF_INET; 
    rec_addr.sin_port = htons(rport); 
    rec_addr.sin_addr.s_addr =  inet_addr(LOCAL_HOST); 
    
    // Send the data to the receiver:
    char *message_sent = "Packet:1"; 
    sendto(
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
    printf("Sent to receiver: %s\n", message_sent); 

    // Wait for receiver's response:
    int len_raddr = sizeof(rec_addr);
    char message_rec[SIZE]; 
    int n = recvfrom(
        // Socket:
        sockfd, 
        // Store the received ACK:
        message_rec, 
        SIZE, // Length of the received message_sent buffer
        // Flags: None
        0,
        // Struct containing src address is returned: 
        (struct sockaddr *) &rec_addr, 
        &len_raddr
    ); 
    message_rec[n] = '\0'; // A C string is a char array with a binary zero (\0) as the final char
    printf("Receiver response : %s\n", message_rec); 

    // Close the socket:
    close(sockfd); 
    return 0; 
} 
