#include<stdio.h>
#include<stdlib.h>
#include "parse_hdmi.h"
//#define SHOW
int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return -1;
	}
	int select = atoi(argv[1]);
	int ret;
	if (select == 0)
		ret = hdmi_parse(argv[2]);
	else
		ret = hdmi_encode(argv[2], argv[3], argv[4]);
	if (ret == -1) {
		fprintf(stderr, "fail to parse hdmi\n");
		return -1;
	}
#ifdef SHOW
    system("python3 show.py");
#endif
	return 0;
}

