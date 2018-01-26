#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NT '\0'

typedef struct kv_pair {
	char* key;
	char* value;
} kv_pair;

// custom dynamic array struct
typedef struct vector {
	kv_pair* array;
	size_t size;
	size_t cap;
} vector;

void free_kv(kv_pair* p) {
  if(p) {
    free(p->key);
    free(p->value);
  }
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
		v->array = (kv_pair*) malloc(sizeof(kv_pair) * init_cap);
		if(!v->array) {
			free(v);
			return NULL;
		}
	}
	return v;
}

// frees vector
void free_v(vector* v) {
	if(v && v->array) {
		for(size_t i = 0; i < v->size; ++i) {
			free_kv(v->array + i);	
		}
		free(v->array);
		free(v);
	}
}

void print_v(vector* v) {
	if(v) {
		printf("Capacity: %lu\nSize: %lu\n", v->cap, v->size);
		for(size_t i = 0; i < v->size; ++i) {
      kv_pair* p = v->array + i;
			printf("kv[%lu] = <%s, %s>\n", i, p->key, p->value);
		}
	}
}

// inserts into vector
// returns 1 if malloc fails
// returns 0 if all goes well
int insert_v(vector* v, const char* valSrc, const char* keySrc) {
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
kv_pair* get_v(vector* v, size_t p) {
	if(p >= v->size) 
		return NULL;
	return v->array + p;
}

int main() {
	vector* v = init_v(3);
	print_v(v);
	int r;
	r = insert_v(v, "Hello", "ALPHA");	
	printf("Result: %d\n", r);
	print_v(v);
	r = insert_v(v, "Goodbye", "BETA");	
	printf("Result: %d\n", r);
	print_v(v);
	r = insert_v(v, "Nikhil", "GAMMA");	
	printf("Result: %d\n", r);
	print_v(v);
	r = insert_v(v, "Kumar", "DELTA");	
	printf("Result: %d\n", r);
	print_v(v);
	r = insert_v(v, "ASDF", "EPSILON");	
	printf("Result: %d\n", r);
	print_v(v);
  kv_pair* pair;
  pair = get_v(v, 0);
	printf("v[0] = <%s, %s>\n", pair->key, pair->value);
  pair = get_v(v, 1);
	printf("v[1] = <%s, %s>\n", pair->key, pair->value);
  pair = get_v(v, 2);
	printf("v[2] = <%s, %s>\n", pair->key, pair->value);
  pair = get_v(v, 3);
	printf("v[3] = <%s, %s>\n", pair->key, pair->value);
  pair = get_v(v, 4);
	printf("v[4] = <%s, %s>\n", pair->key, pair->value);
  pair = get_v(v, 5);
  if(pair)
	  printf("v[5] = <%s, %s>\n", pair->key, pair->value);
  else
    printf("NULL\n");
	free_v(v);	
	return 0;
}
