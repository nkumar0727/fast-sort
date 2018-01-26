#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_LINE 128
#define NL '\n'
#define NT '\0'
#define DH '-'

//////////////////////////////////////////////////////////////////
// KEY-VALUE PAIR
//////////////////////////////////////////////////////////////////

typedef struct kv_pair {
	char* key;
	char* value;
} kv_pair;

void free_kv(kv_pair* p) {
  if(p) {
    free(p->key);
    free(p->value);
  }
}

//////////////////////////////////////////////////////////////////
// DYNAMIC ARRAY OF KV_PAIRS
//////////////////////////////////////////////////////////////////

// custom dynamic array struct
typedef struct vector {
	kv_pair* array;
	size_t size;
	size_t cap;
} vector;

// constructor
// returns NULL on failure
// returns handle to vector on success
vector* 
init_v(size_t init_cap) {
	vector* v;
	v = (vector*) malloc(sizeof(vector));
	if(v) {
		v->size = 0;
		v->cap = init_cap;
		v->array = (kv_pair*) malloc(sizeof(kv_pair) * init_cap);
		if(!v->array) {
			free(v);
			return NULL;
		}
	}
	return v;
}

// frees vector
void 
free_v(vector* v) {
	if(v && v->array) {
		for(size_t i = 0; i < v->size; ++i) {
			free_kv(v->array + i);	
		}
		free(v->array);
		free(v);
	}
}

void 
print_v(vector* v) {
	if(v) {
		//printf("Capacity: %lu\nSize: %lu\n", v->cap, v->size);
		for(size_t i = 0; i < v->size; ++i) {
      kv_pair* p = v->array + i;
			printf("%s\n", p->value);
		}
	}
}

// inserts into vector
// returns 1 if malloc fails
// returns 0 if all goes well
int 
insert_v(vector* v, const char* valSrc, const char* keySrc) {
	size_t valLen = strlen(valSrc);
  size_t keyLen = strlen(keySrc);
	size_t curr_cap = v->cap;
	size_t curr_size = v->size;
	if(curr_size == curr_cap) {
    kv_pair* n_arr = (kv_pair*) malloc(sizeof(kv_pair) * curr_cap * 2);
		if(!n_arr)
			return 1;
		memcpy((void*) n_arr, (void*) v->array, curr_cap * sizeof(kv_pair));
		free(v->array);
		v->array = n_arr;
		v->cap = 2 * curr_cap;
	}
	kv_pair* loc = &v->array[v->size];
  loc->key = (char*) malloc(sizeof(char) * (keyLen + 1));
  if(!loc->key)
    return 1;
  loc->value = (char*) malloc(sizeof(char) * (valLen + 1));
  if(!loc->value) {
    free(loc->key);
    return 1;
  }
  strncpy(loc->key, keySrc, keyLen);
  loc->key[keyLen] = NT;
	strncpy(loc->value, valSrc, valLen);
  loc->value[valLen] = NT;
	v->size++;
	return 0;	
}

// gets item in vector (zero indexed)
// returns the string
// returns NULL if out of range
kv_pair* 
get_v(vector* v, size_t p) {
	if(p >= v->size) 
		return NULL;
	return v->array + p;
}

//////////////////////////////////////////////////////////////////
// FILE READ AND STORE 
//////////////////////////////////////////////////////////////////

struct line_info {
	int end;
	char* key;
};

struct line_info 
gline(FILE* file, char* buf, size_t k) {
  struct line_info info;
  info.key = NULL;
	if(buf) {
		size_t pos, wrd, len, start;
    pos = len = start = 0;
    wrd = 0;
    char state = 0;
		int input = NL;
		do {
			input = fgetc(file);
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
		if(input == NL || input == EOF)
			buf[pos - 1] = NT;
		else
			buf[MAX_LINE] = NT;
    info.key = (char*) malloc(sizeof(char) * (len + 1));   
    strncpy(info.key, buf+start, len);
    info.key[len] = NT;
    info.end = (input == EOF);
	}
  else
    info.end = 1;
  return info;
}

vector* 
store_f(const char* f_name, size_t k) {
	vector* v = NULL;
	FILE* file = fopen(f_name, "r");
	if(file) {
		char* line = (char*) malloc(sizeof(char) * MAX_LINE);		
		if(line) {
			v = init_v(16);
			if(v) {
				struct line_info inf;
				while(1) {
					inf = gline(file, line, k);
					if(inf.end == 1) {
						free(inf.key);
						break;
					}
					if(insert_v(v, line, inf.key) == 1) {
						free(inf.key);
						free_v(v);
						v = NULL;
						break;
					}
					free(inf.key);
				}
			}
			free(line);
		}
		fclose(file);	
	}
	return v;
}

//////////////////////////////////////////////////////////////////
// SORTING 
//////////////////////////////////////////////////////////////////

// comparison wrapper for qsort 
int
cmpkey(const void* p1, const void* p2) {
	return strcmp(((kv_pair*) p1)->key, ((kv_pair*) p2)->key);
}

//////////////////////////////////////////////////////////////////
// MAIN PROGRAM 
//////////////////////////////////////////////////////////////////

const char* ERR_CMDP = "Error: Bad command line parameters\n";
const char* ERR_COF = "Error: Cannot open file";

int 
main(int argc, char* argv[]) {
	vector* v;
	clock_t start, end, final;
	double cpu_time_used;
	start = clock();
	if(argc == 2) {
		v = store_f(argv[1], 0);
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("CPU Time for LOADING: %f\n", cpu_time_used);
		if(v) {
			/** printf("Sort %s by first word and print\n", argv[1]); */
			/** printf("Unsorted\n"); */
			/** print_v(v); */

			qsort(v->array, v->size, sizeof(kv_pair), cmpkey);
      final = clock();
	cpu_time_used = ((double) (final - end)) / CLOCKS_PER_SEC;
	printf("CPU Time for SORTING: %f\n", cpu_time_used);
			/** printf("Sorted\n"); */
			//print_v(v); 
			free_v(v);
		}
		else {
			fprintf(stderr, "%s %s\n", ERR_COF, argv[1]);
			exit(1);
		}
	}
	else if(argc == 3) {
		int len = strlen(argv[1]);
		if(len < 2 || argv[1][0] != DH) {
			fprintf(stderr, "%s", ERR_CMDP);
			exit(1);
		}
		char* endptr;
		long int n = strtol(argv[1]+1, &endptr, 10);	
		if(*endptr != '\0' || n < 1 || n > MAX_LINE) {
			fprintf(stderr, "%s", ERR_CMDP);
			exit(1);
		}
		v = store_f(argv[2], n);
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("CPU Time for LOADING: %f\n", cpu_time_used);
		if(v) {
			/** printf("Sort %s by word %ld and print\n", argv[2], n); */
			/** printf("Unsorted\n"); */
			/** print_v(v); */
			qsort(v->array, v->size, sizeof(kv_pair), cmpkey);
      final = clock();
	cpu_time_used = ((double) (final - end)) / CLOCKS_PER_SEC;
	printf("CPU Time for SORTING: %f\n", cpu_time_used);
			/** printf("Sorted\n"); */
			// print_v(v); 
			free_v(v);
		} 
		else{
			fprintf(stderr, "%s %s\n", ERR_COF, argv[1]);
			exit(1);
		}
	}
	else {
		fprintf(stderr, "%s", ERR_CMDP);
		exit(1);
	}
	exit(0);
}
