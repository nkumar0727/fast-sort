#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CACHE_SIZE (1<<15)
//#define BLOCK_BYTES (CACHE_SIZE - (2 * sizeof(void*) + 2 * sizeof(size_t)))
#define BLOCK_BYTES 20

typedef struct cache_block {
  struct cache_block* prev;
  struct cache_block* next;
  size_t free_bytes;
  size_t offset;
  char blk[BLOCK_BYTES];
} cblock;

cblock* 
cb_new() {
  cblock* b = (cblock*) malloc(sizeof(cblock)); 
  if(b) {
    b->prev = b->next = NULL;
    b->free_bytes = BLOCK_BYTES; 
    b->offset = 0;
  }
  return b;
}

void
cb_delete(cblock** root) {
  if(root) {
    cblock* curr = *root;
    cblock* n;
    while(curr) {
      n = curr->next;
      free(curr);
      curr = n;
    }
    *root = NULL;
  }
}

void
cb_print(cblock* b) {
  printf("Free Bytes: %lu, Offset: %lu\n", b->free_bytes, b->offset);
  size_t pos = 0;
  char t;
  while(pos < b->offset) {
    t = b->blk[pos];    
    if(t == '\0') 
      t = ' ';
    printf("%c", t);
    ++pos;
  }
  printf("\n");
}

void
cb_pchain(cblock* root) {
  size_t count = 0;
  cblock* n = root;
  printf("\n----------CACHE BLOCK DUMP----------\n\n");
  while(n) {
    printf("BLOCK %lu\n----------\n", count);
    cb_print(n);
    printf("\n");
    n = n->next;
    ++count;
  }
}

char*
cb_insert(char* record, size_t len, cblock* root) {
  char* source = NULL;
  cblock* src = root;
  cblock* n;
  while(src && src->free_bytes <= len) {
    n = src;
    src = src->next;
  }
  if(!src) {
    n->next = cb_new();
    n->next->prev = n; 
    src = n->next;
  }
  source = src->blk + src->offset;
  strncpy(src->blk + src->offset, record, len);
  src->blk[src->offset + len] = '\0';
  src->free_bytes -= len + 1;
  src->offset += len + 1;
  return source;
}

int
main() {
  cblock* b = cb_new();
  char* r1 = "Nikhil Kumar";
  char* r2 = "abcdefghijklmnop";
  char* r3 = "BYE";
  char* r4 = "something";
  char* r5 = "neanderthal";
  char* r6 = "train simulator";
  char* r7 = "asdfghjkl";
  char* r8 = "larry";
  cb_pchain(b);
  cb_insert(r1, strlen(r1), b);
  cb_pchain(b);
  cb_insert(r2, strlen(r2), b);
  cb_pchain(b);
  cb_insert(r3, strlen(r3), b);
  cb_pchain(b);
  cb_insert(r4, strlen(r4), b);
  cb_pchain(b);
  cb_insert(r5, strlen(r5), b);
  cb_pchain(b);
  cb_insert(r6, strlen(r6), b);
  cb_pchain(b);
  cb_insert(r7, strlen(r7), b);
  cb_pchain(b);
  cb_insert(r8, strlen(r8), b);
  cb_pchain(b);
  cb_delete(&b);
  return 0;
}
