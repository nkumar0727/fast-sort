#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// custom dynamic array struct
typedef struct vector {
	char** array;
	size_t size;
	size_t cap;
} vector;

typedef struct kv_pair {
	char* key;
	char* value;
} kv_pair;

void free_kv(kv_pair** p) {
	free((*p)->key);
	free((*p)->value);
	*p = NULL;
}

// constructor
// returns NULL on failure
// returns handle to vector on success
vector* init_v(size_t init_cap) {
	vector* v;
	v = (vector*) malloc(sizeof(vector));
	if(v) {
		v->size = 0;
		v->cap = init_cap;
		v->array = (char**) malloc(sizeof(char*) * init_cap);
		if(!v->array) {
			free(v);
			return NULL;
		}
	}
	return v;
}

// frees vector
void free_v(vector** v) {
	if(v && *v) {
		vector* hndl = *v;
		for(size_t i = 0; i < hndl->size; ++i) {
			free(hndl->array[i]);	
		}
		free(hndl->array);
		free(*v);
	}
}

void print_v(vector* v) {
	if(v) {
		printf("Capacity: %lu\nSize: %lu\n", v->cap, v->size);
		for(size_t i = 0; i < v->size; ++i) {
			printf("v[%lu] = %s\n", i, v->array[i]);
		}
	}
}

// inserts into vector
// returns 1 if malloc fails
// returns 0 if all goes well
int insert_v(vector* v, const char* src) {
	size_t len = strlen(src);
	size_t curr_cap = v->cap;
	size_t curr_size = v->size;
	if(curr_size == curr_cap) {
		char** n_arr = (char**) malloc(sizeof(char*) * curr_cap * 2);
		if(!n_arr)
			return 1;
		memcpy((void*) n_arr, (void*) v->array, curr_cap * sizeof(char*));
		free(v->array);
		v->array = n_arr;
		v->cap = 2 * curr_cap;
	}
	char* loc;
	loc = (v->array[v->size] = (char*) malloc(sizeof(char) * len));
	if(!loc)
		return 1;
	strncpy(loc, src, len);
	v->size++;
	return 0;	
}

// gets item in vector (zero indexed)
// returns the string
// returns NULL if out of range
char* get_v(vector* v, size_t p) {
	if(p >= v->size) 
		return NULL;
	return v->array[p];
}

int main() {
	vector* v = init_v(3);
	print_v(v);
	int r;
	r = insert_v(v, "Hello");	
	printf("Result: %d\n", r);
	print_v(v);
	r = insert_v(v, "Goodbye");	
	printf("Result: %d\n", r);
	print_v(v);
	r = insert_v(v, "Nikhil");	
	printf("Result: %d\n", r);
	print_v(v);
	r = insert_v(v, "Kumar");	
	printf("Result: %d\n", r);
	print_v(v);
	r = insert_v(v, "ASDF");	
	printf("Result: %d\n", r);
	print_v(v);
	printf("v[0] = %s\n", get_v(v, 0));
	printf("v[1] = %s\n", get_v(v, 1));
	printf("v[2] = %s\n", get_v(v, 2));
	printf("v[3] = %s\n", get_v(v, 3));
	printf("v[4] = %s\n", get_v(v, 4));
	printf("v[5] = %s\n", get_v(v, 5));
	free_v(&v);	
	return 0;
}
