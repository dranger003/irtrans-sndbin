all:
	gcc -Wall -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -mcpu=arm1176jzf-s main.c -o irtrans-sndbin

clean:
	rm -f *.o irtrans-sndbin
