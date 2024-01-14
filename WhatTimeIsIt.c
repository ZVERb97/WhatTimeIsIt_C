#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <stdio.h>
#include <time.h>

typedef struct NTP_Packet
{
    unsigned char leap_version_mode;
    unsigned char stratum;
    unsigned char poll;
    unsigned char precision;
    unsigned root_delay;
    unsigned int root_dispersion;
    char rfid[4];
    unsigned long long ref_timestamp;
    unsigned long long original_timestamp;
    unsigned int receive_timestamp_sec;
    unsigned int receive_timestamp_frac;
    unsigned long long transmit_timestamp;

}Packet;



int main(int argc, char **argv)
{
#ifdef _WIN32
    // this part is only required on Windows: it initializes the Winsock2 dll
    WSADATA wsa_data;
    if (WSAStartup(0x0202, &wsa_data))
    {
        printf("unable to initialize winsock2 \n");
        return -1;
    }
#endif
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0)
    {
        printf("unable to initialize the UDP socket \n");
        return -1;
    }

    printf("socket %d created \n", s);
    struct sockaddr_in sin;
    inet_pton(AF_INET, "216.239.35.12", &sin.sin_addr); // this will create a big endian 32 bit address
    sin.sin_family = AF_INET;
    sin.sin_port = htons(123); // converts 9999 to big endian

    Packet packet;
    time_t current_time = time(NULL);
   
    packet.leap_version_mode = (0 << 6 | 4 << 3 | 3 );
    packet.stratum = 0;
    packet.poll = 0;
    packet.precision = 0;
    packet.root_delay = 0;
    packet.root_dispersion = 0;
    packet.rfid[0] = 0;
    packet.rfid[1] = 0;
    packet.rfid[2] = 0;
    packet.rfid[3] = 0;
    packet.ref_timestamp = 0;
    packet.original_timestamp = current_time;
    packet.receive_timestamp_sec = 0;
    packet.receive_timestamp_frac = 0;
    packet.transmit_timestamp = 0;

    int sent_bytes = sendto(s,(char*)&packet, sizeof(packet),0, (struct sockaddr*)&sin,sizeof(sin));
    printf("sent %d bytes via UDP\n",sent_bytes);

    
    for (;;)
    {

        struct sockaddr_in sender_in;
        int sender_in_size = sizeof(sender_in);
        int len = recvfrom(s, (char*)&packet, sizeof(packet), 0, (struct sockaddr *)&sender_in, &sender_in_size);
        if (len > 0)
        {
            char addr_as_string[64];
            inet_ntop(AF_INET, &sender_in.sin_addr, addr_as_string, 64);
            printf("received %d bytes from %s:%d\n", len, addr_as_string, ntohs(sender_in.sin_port));
            break;
        }

    }

    char buffer[80];
    struct tm time_info;
    time_t universal_time = ntohl(packet.receive_timestamp_sec);
    universal_time -= (unsigned int)365 * (unsigned int)3600 * (unsigned int)24 * (unsigned int)70;
    universal_time -= (unsigned int)3600 * (unsigned int)24 * (unsigned int)(70/4);
    localtime_s(&time_info, &universal_time);
    strftime(buffer,sizeof(buffer),"%d-%m-%Y -%H:%M:%S", &time_info);
    printf("%s",buffer);


    return 0;
}

