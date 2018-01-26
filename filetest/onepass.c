#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define NL '\n'
#define NT '\0'
#define TB '\t'
#define DH '-'
#define SP ' '

#define MAX_BUF 128
#define CACHE_SIZE (1<<15)
#define BLOCK_BYTES (CACHE_SIZE - (2 * sizeof(void*) + 2 * sizeof(size_t)))

typedef struct cblock {
  struct cblock* prev;
  struct cblock* next;
  size_t free_bytes;
  size_t offset;
  char blk[BLOCK_BYTES];
} cblock;

typedef struct {
  cblock* root;
  cblock* tail;
} clist;

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
cl_delete(clist* cache) {
  if(cache) {
    cblock* curr = cache->root;
    cblock* n;
    while(curr) {
      n = curr->next;
      free(curr);
      curr = n;
    }
    cache->root = cache->tail = NULL;
  }
}

char*
cl_insert(char* record, size_t len, clist* cache) {
  char* source = NULL;
  cblock* src = cache->tail;
  if(src->free_bytes <= len) {
    src->next = cb_new();
    src->next->prev = src;
    src = src->next;
    cache->tail = src;
  }
  source = src->blk + src->offset;
  strncpy(src->blk + src->offset, record, len);
  src->blk[src->offset + len] = '\0';
  src->free_bytes -= len + 1;
  src->offset += len + 1;
  return source;
}

typedef struct {
  char* hndl;
  size_t fline;
} FMeta;

FMeta 
load(char* file) {
  FMeta meta;
  meta.hndl = NULL;
  FILE* fp = fopen(file, "r");
  if(!fp) 
    return meta;
  struct stat file_stat;
  fstat(fileno(fp), &file_stat);
  size_t fsize = file_stat.st_size;
  meta.hndl = (char*) malloc(fsize + 1);
  if(!meta.hndl) {
    fclose(fp);
    return meta;
  }
  meta.fline = 0;
  char* bptr = meta.hndl;
  while(fgets(bptr, MAX_BUF, fp)) {
    char* term = strchr(bptr, NL);
    if(!term) {
      fclose(fp);
      free(meta.hndl);
      meta.hndl = NULL;
      return meta;
    }
    *term = NT;
    bptr = term + 1;
    ++meta.fline;
  }
  fclose(fp);
  return meta;
}

typedef struct {
  char* key;
  char* val;
} Kvp;

typedef struct {
  clist cache;
  size_t cap;
  size_t sz;
  Kvp* pairs;
} SortBlock;

SortBlock 
super_load(char* file, size_t k) {
  SortBlock blk;
  blk.cap = blk.sz = 0;
  blk.pairs = NULL;
  blk.cache.root = blk.cache.tail = NULL;
  char* hndl = NULL;
  FILE* fp = fopen(file, "r");
  if(!fp) 
    return blk;
  struct stat file_stat;
  fstat(fileno(fp), &file_stat);
  size_t fsize = file_stat.st_size;
  hndl = (char*) malloc(fsize + 1);
  if(!hndl) {
    fclose(fp);
    return blk;
  }
  blk.pairs = (Kvp*) malloc(sizeof(Kvp) * 1024);
  if(!blk.pairs) {
    fclose(fp);
    free(hndl);
    return blk;
  }
  blk.cap = 1024;
  blk.cache.root = cb_new();
  blk.cache.tail = blk.cache.root;
  size_t line_no, f_pos, w_count, w_len, w_start;
  line_no = f_pos = w_count = w_len = w_start = 0;
  char input = NL;
  char state = 0;
  char* curr_line = hndl;
  while(f_pos < fsize) {
    input = (hndl[f_pos] = fgetc(fp));
    if(input == NL) {
      hndl[f_pos] = NT;      
      printf("Size: %lu, Cap: %lu\n", blk.sz, blk.cap);
      if(blk.sz == blk.cap) {
        printf("Danger Zone\n");
        if(realloc(blk.pairs, sizeof(Kvp) * blk.cap * 2) == NULL) {
          printf("Dude its fucked\n");
          cl_delete(&blk.cache);
          free(blk.pairs);
          fclose(fp);
          free(hndl);
          blk.cap = 0j
          return blk;
        }
        blk.cap *= 2;
      }
      printf("FUUUU\n");
      blk.pairs[line_no].val = curr_line;
      curr_line = hndl + f_pos + 1;
      printf("CK\n");
      blk.pairs[line_no].key = cl_insert(hndl + w_start, w_len, &blk.cache);
      printf("OH\n");
      blk.sz += 1;
      ++line_no;
      w_start = w_len = w_count = 0;
      state = 0;
    }
    else if(input == SP || input == TB)
      state = 0;
    else if(state == 0) {
      state = 1;
      ++w_count;
      if(w_count <= k) {
        w_start = f_pos;
        w_len = 1;
      }
    }
    else if(w_count <= k) {
      ++w_len;
    }
    ++f_pos;
  }
  fclose(fp);
  return blk;
}

int main(int argc, char** argv) {
  /*
  clist* cache = (clist*) malloc(sizeof(clist)); 
  cache->root = cb_new();
  cache->tail = cache->root;
*/
  clock_t start, end;
  double cpu_time_used;
  start = clock();
  SortBlock blk = super_load(argv[1], atoi(argv[2]));
  if(blk.cap == 0)
    return 1;
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("CPU Time for FULL LOAD: %f\n", cpu_time_used);
  printf("Lines: %lu\n", blk.sz);
  printf("Cap: %lu\n", blk.cap);
  return 0;
}
