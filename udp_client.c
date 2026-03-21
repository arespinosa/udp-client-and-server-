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

    struct packet_headers {
        int seq_num;
        time_t time_stamp;
    };

    int sockfd;
    struct sockaddr_in server_addr;
    int addrlen = sizeof(server_addr);
    socklen_t AddrLen = sizeof(server_addr);
    struct packet_headers packet, recievedPacket;
    int N;
    int B;
    int seconds_Delay;
    char s_ipAddy[20]; 
    
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

    printf("Enter the IP Address:");
    scanf("%s", s_ipAddy);
    printf("Server IP Address: %s \n", s_ipAddy);

    int packet_size = sizeof(struct packet_headers) + B;
    char buffer[packet_size];
    memset(buffer, 0, packet_size);

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

    double total_rtt = 0.0;

    for (int i = 0; i < N; i++) {
        // Allocate packet with payload
        struct timeval tv;
        gettimeofday(&tv, NULL);
        double currentTime = tv.tv_sec + tv.tv_usec / 1e6;


        packet.seq_num = i;
        packet.time_stamp = currentTime;
    
        // Send packet
        sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&server_addr, addrlen);
        // Receive echo
        int n = recvfrom(sockfd, &recievedPacket, sizeof(recievedPacket), 0, (struct sockaddr *)&server_addr, &AddrLen);
        
        //FIXME: Does this indicate packet loss? 
        if (n < 0) {
            perror("recvfrom failed");
        }
        sleep(seconds_Delay);
        //After receiving the response, recording the RTT 
        gettimeofday(&tv, NULL);
        double recvTime = tv.tv_sec + tv.tv_usec / 1e6;

        double rtt = (recvTime - packet.time_stamp) * 1000.0;


        printf("Packet %d has RTT = %.3f ms\n", recievedPacket.seq_num, rtt);

        total_rtt += rtt;
    }

    // After the end of the run, printing the average RTT 
    double avg_rtt = total_rtt / N;
    printf("Average RTT after %d runs is: %.3f", N, avg_rtt);        
            
    close(sockfd);


    return 0;
    }

