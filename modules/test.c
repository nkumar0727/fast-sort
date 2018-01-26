#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 20
#define NT '\0'
#define NL '\n'

struct line_info {
	int end;
	char* 
};

int
gline(char* buf, size_t k) {
	if(buf) {
		size_t pos = 0;
		size_t wrd = 0;
		char in_wrd = 0;
		char input = NL;
		do {
			input = (char) fgetc(stdin);
			if(pos < MAX_LINE) {
				buf[pos++] = input;
				if(input )
			}
		} while(input != NL && input != EOF);
		if(input == NL || input == EOF)
			buf[pos - 1] = NT;
		else
			buf[MAX_LINE] = NT;
		return input == EOF;
	}
	return 1;
}

int
main() {
	char* buf = (char*) malloc(sizeof(char) * MAX_LINE);
	
	return 0;
}
