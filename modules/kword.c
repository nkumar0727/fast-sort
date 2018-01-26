#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 80
#define NT '\0'
#define NL '\n'

struct line_info {
	int end;
	char* key;
};

struct line_info 
gline(char* buf, size_t k) {
  struct line_info info;
  info.key = NULL;
	if(buf) {
		size_t pos, wrd, len, start;
    pos = len = start = 0;
    wrd = 0;
    char state = 0;
		int input = NL;
		do {
			input = fgetc(stdin);
			if(pos < MAX_LINE) {
        if(isspace(input))
          state = 0;
        else if(state == 0) {
          state = 1;
          ++wrd;
          if(wrd <= k) {
            start = pos;
            len = 1;
          }
        }
        if(wrd <= k && state == 1)
          ++len;
				buf[pos++] = (char) input;
			}
		} while(input != NL && input != EOF);
    info.key = (char*) malloc(sizeof(char) * (len + 1));   
    printf("Start: %c, Len: %lu\n", buf[start], len);
    strncpy(info.key, buf+start, len);
    info.key[len] = NT;
		if(input == NL || input == EOF)
			buf[pos - 1] = NT;
		else
			buf[MAX_LINE] = NT;
    info.end = (input == EOF);
	}
  else
    info.end = 1;
  return info;
}

int
main(int argc, char** argv) {
  size_t k = atoi(argv[1]); 
  char* buf = (char*) malloc(sizeof(char) * MAX_LINE);
  struct line_info inf = gline(buf, k);
  printf("Key: %s\nResult: %d\n", inf.key, inf.end);
  free(inf.key);
  free(buf);
	return 0;
}
