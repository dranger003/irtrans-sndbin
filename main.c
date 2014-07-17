#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    struct stat st = { 0 };
    stat(argv[1], &st);

    unsigned char *buf = (unsigned char *)malloc(st.st_size);

    FILE *f = fopen(argv[1], "rb");
    fread(buf, st.st_size, 1, f);
    fclose(f);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[2]);
    addr.sin_port = htons(21000);

    sendto(fd, buf, st.st_size, 0, (struct sockaddr *)&addr, sizeof(addr));

    free(buf);

    return 0;
}
