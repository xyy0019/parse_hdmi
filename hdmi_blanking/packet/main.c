#include<stdio.h>
#include "parse_hdmi.h"
int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return -1;
	}
 
	int ret = hdmi_parse(argv[1]);
	if (ret == -1) {
		fprintf(stderr, "fail to parse hdmi\n");
		return -1;
	}
	return 0;
}
