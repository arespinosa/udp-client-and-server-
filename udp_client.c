#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h> //Defines core socket functions and constants.
#include <netinet/in.h> //Defines Internet address structures.
#define PORT 8080 // Port number
int main() {

    // Creating the struct packet headers 
    struct packet_headers {
        int seq_num;
        double time_stamp;
    };

    /*struct sockaddr_in {
    short sin_family; // e.g. AF_INET
    unsigned short sin_port; // e.g. htons(3490)
    struct in_addr sin_addr; // see struct in_addr, below
    char sin_zero[8]; // zero this if you want to
    }; */
    int sockfd;
    struct sockaddr_in server_addr;
    int addrlen = sizeof(server_addr);
    socklen_t AddrLen = sizeof(server_addr);
    struct packet_headers packet, recievedPacket;

    // Input Variables we'll be getting from user 
    int N;
    int B;
    int seconds_Delay;
    char s_ipAddy[20]; 
    
    // Will be used to extract time and compute RTT values
    struct timeval tv;
    double microseconds;
    double total_rtt = 0.0;
    double succ_packets = 0.0;

    /**
     * Accepting user input for:
     * N - # of Packets
     * B - Message Size in Bytes
     * seconds_Delay - Inter-packet delay (seconds)
     * s_ipAddy - Server IP Address 
    */
    printf("Number of Packets to send: ");
    scanf("%d", &N);   
    printf("N = %d \n", N);

    printf("Message size (in Bytes): ");
    scanf("%d", &B);
    printf("Message Size = %d Bytes \n", B);

    printf("Inter-packet delay (in seconds): ");
    scanf("%d", &seconds_Delay);
    printf("Inter-packet delay is %d seconds \n", seconds_Delay);

    printf("Enter the IP Address: ");
    scanf("%s", s_ipAddy);
    printf("Server IP Address: %s \n", s_ipAddy);

    int packet_size = sizeof(struct packet_headers) + B;
    
    // Create Socket with Af_Inet + Datagram 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Converting the User's server IP Address into a byte format 
    if (inet_pton(AF_INET, s_ipAddy, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    // Iterating through the amount of packets we're going to send 
    for (int i = 0; i < N; i++) {
        gettimeofday(&tv, NULL);
        microseconds = tv.tv_usec / 1000000.00;
        double currentTime = tv.tv_sec + microseconds;

        packet.seq_num = i;
        packet.time_stamp = currentTime;

        // Sending packet
        sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&server_addr, addrlen);
        // Receiving echo back from server
        int n = recvfrom(sockfd, &recievedPacket, sizeof(recievedPacket), 0, (struct sockaddr *)&server_addr, &AddrLen);
        
        if (n < 0) {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
        }

        if(recievedPacket.seq_num != i){
            printf("Packet %d was lost", i);
            succ_packets -= 1.0;
        }

        gettimeofday(&tv, NULL);
        microseconds = tv.tv_usec / 1000000.00;

        double recvTime = tv.tv_sec + microseconds;
        // Calculating the round trip time and multiplying by 1000 to get it in the form of ms
        double rtt = (recvTime - packet.time_stamp) * 1000.0;

        printf("Packet %d has RTT = %.3f ms\n", recievedPacket.seq_num, rtt);
        total_rtt += rtt;
        succ_packets += 1.0;

        // Inter-packet delay 
        sleep(seconds_Delay);
    }

    // Sending a packet with -1 to let the server know we are done sending N amount of packets and 
    // allow the server to compute the avg. 
    packet.seq_num = -1;

    if(sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&server_addr, addrlen) < 0){
        perror("Send Failed");
        exit(EXIT_FAILURE);
    }

    // At the end of the run, computing the average RTT 
    double avg_rtt = total_rtt / N;
    double lost_packets = N - succ_packets;
    printf("Average RTT: %.3f \n", avg_rtt);       
    printf("Packet Loss Percentage: %.2f %% \n", lost_packets);

    // Close socket            
    close(sockfd);

    return 0;
    }