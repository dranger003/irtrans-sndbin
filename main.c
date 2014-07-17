#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

unsigned char sig[] = // 76 bytes
{
    0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x08, 0xD6, 0x02, 0x27, 0x0B, 0x64, 0xB3, 0x8B
};

unsigned char rsp[] = // 78 bytes
{
    0xe7, 0x4d, 0x57, 0x36, 0x2e, 0x30, 0x39, 0x2e, 0x32, 0x30, 0xd9, 0x6d, 0x18, 0x01, 0xc7, 0x1a,
    0x01, 0x00, 0x52, 0x34, 0x2e, 0x34, 0x2e, 0x36, 0x00, 0x00, 0x00, 0x23, 0xa7, 0x21, 0x74, 0x32,
    0x67, 0x1c, 0x5a, 0x48, 0x0a, 0xa3, 0x00, 0x10, 0x11, 0x00, 0x00, 0x00, 0x07, 0x11, 0x02, 0x5a,
    0x23, 0x31, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned char ack[] = // 3 bytes
{
    0xd4, 0x02, 0x00
};

int buf_cmp(unsigned char *buf1, unsigned char *buf2, int len)
{
    int i;
    for (i = 0; i < len; i++)
        if (buf1[i] != buf2[i])
            return 0;

    return 1;
}

void recv_data(char **argv)
{
    int sckt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in server = { 0 };
    server.sin_family = AF_INET;
    server.sin_port = htons(21000);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sckt, (struct sockaddr *)&server, sizeof(server));

    unsigned char buf[4096] = { 0 };
    struct sockaddr_in client = { 0 };
    socklen_t client_len = sizeof(client);

    while (1)
    {
        ssize_t len = recvfrom(sckt, buf, sizeof(buf), 0, (struct sockaddr *)&client, &client_len);
        printf("%s:%d %d byte(s):\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), len);

        client.sin_port = htons(21000);

        if (len == sizeof(sig) && buf_cmp(buf, sig, len)) {
            sendto(sckt, rsp, sizeof(rsp), 0, (struct sockaddr *)&client, sizeof(client));
            printf("Initialized.\n\n");

            continue;
        }

        sendto(sckt, ack, sizeof(ack), 0, (struct sockaddr *)&client, sizeof(client));

        int i, c = 1;
        for (i = 0; i < len; i++) {
            printf("0x%02X ", buf[i]);

            if (!(c++ % 16) && i < len)
                printf("\n");
        }

        printf("\n\n");

        char fn[65535];
        printf("Filename: ");
        scanf("%s", fn);

        FILE *fs = fopen(fn, "wb");
        len = fwrite(buf, 1, len, fs);
        fclose(fs);
        printf("%d byte(s) written to %s.\n\n", len, fn);
    }

    close(sckt);
}

void snd_data(char **argv)
{
    struct stat st = { 0 };
    stat(argv[2], &st);

    unsigned char *buf = (unsigned char *)malloc(st.st_size);

    {
        FILE *fs = fopen(argv[2], "rb");
        fread(buf, st.st_size, 1, fs);
        fclose(fs);
    }

    int sckt = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in client = { 0 };
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(argv[3]);
    client.sin_port = htons(21000);

    sendto(sckt, buf, st.st_size, 0, (struct sockaddr *)&client, sizeof(client));

    close(sckt);
    free(buf);
}

int main(int argc, char *argv[])
{
    if (!strcmp(argv[1], "rx"))
        recv_data(argv);
    else if (!strcmp(argv[1], "tx"))
        snd_data(argv);

    return 0;
}
