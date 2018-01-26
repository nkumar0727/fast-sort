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

#define ERR_COF "Error: Cannot open file %s\n"
#define ERR_MF "Error: malloc failed\n"
#define ERR_NFS "Error: No file stats\n"
#define ERR_NR "Error: No read\n"

#define MAX_BUF 128

#define CACHE_SIZE (1<<15)
#define BLOCK_BYTES (CACHE_SIZE - (2 * sizeof(void*) + 2 * sizeof(size_t)))
//#define BLOCK_BYTES 20 

///////////////////////////////////////////////////////////////////////////////
// CACHE BLOCK STORAGE LINKED LIST
///////////////////////////////////////////////////////////////////////////////

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

/*
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
*/

// DEBUG
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

// DEBUG
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

///////////////////////////////////////////////////////////////////////////////
// AUXILIARY STRUCTURES FOR RETURNING INFO 
///////////////////////////////////////////////////////////////////////////////

typedef struct {
  char* hndl;
  size_t sz;
} FMeta;

typedef struct {
  char* key;
  char* val;
} Kvp;

typedef struct {
  clist cache;
  Kvp* pairs;
} SortBlock;

///////////////////////////////////////////////////////////////////////////////
// FILE I/O and STATS 
///////////////////////////////////////////////////////////////////////////////

FMeta
load_file(char* f_path) {
  FMeta meta;
  meta.hndl = NULL;
  int fd = open(f_path, O_RDONLY);
  if(fd == -1)
    fprintf(stderr, ERR_COF, f_path);
  else {
    struct stat f_st;
    if(fstat(fd, &f_st) == -1)
      fprintf(stderr, ERR_NFS);
    else {
      meta.sz = f_st.st_size;
      meta.hndl = (char*) malloc(meta.sz + sizeof(char));
      if(!meta.hndl)
        fprintf(stderr, ERR_MF);
      else {
        if(read(fd, meta.hndl, meta.sz) < 1) {
          fprintf(stderr, ERR_NR);
          free(meta.hndl);
          meta.hndl = NULL;
        }
      }

    }
  }
  close(fd);
  return meta;
}

size_t
line_count(FMeta meta) {
  size_t count, pos;
  count = pos = 0;
  while(pos < meta.sz) {
    if(meta.hndl[pos] == NL) {
      ++count;
      meta.hndl[pos] = NT;
    }
    ++pos;
  }
  return count;
}

// DEBUG
void
print_sblock(SortBlock* blk, size_t len) {
  for(size_t i = 0; i < len; ++i) {
    //printf("Key: %s\n", blk->pairs[i].key);
    printf("%s\n", blk->pairs[i].val);
  }
}

///////////////////////////////////////////////////////////////////////////////
// KEY-VALUE STORING FOR LOCALITY 
///////////////////////////////////////////////////////////////////////////////

SortBlock
gen_kvps(FMeta meta, size_t n_lines, size_t k) {
  SortBlock blk;
  blk.cache.root = blk.cache.tail = NULL;
  blk.pairs = NULL;
  blk.pairs = (Kvp*) malloc(sizeof(Kvp) * n_lines); 
  if(!blk.pairs)
    fprintf(stderr, ERR_MF);
  else {
    blk.cache.root = cb_new();
    blk.cache.tail = blk.cache.root;
    size_t line_no, f_pos, w_count, w_len, w_start;
    line_no = f_pos = w_count = w_len = w_start = 0;
    char input = NL;
    char state = 0;
    char* curr_line = meta.hndl;
    while(f_pos < meta.sz) {
      input = meta.hndl[f_pos];      
      if(input == NT) {
        blk.pairs[line_no].val = curr_line;
        curr_line = meta.hndl + f_pos + 1;
        blk.pairs[line_no].key = cl_insert(meta.hndl + w_start, w_len, &blk.cache);
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
      else if(w_count <= k) 
        ++w_len;
      ++f_pos;
    }
  }
  return blk; 
}

int
cmpkey(const void* p1, const void* p2) {
	return strcmp(((Kvp*) p1)->key, ((Kvp*) p2)->key);
}

int
main(int argc, char** argv) {
	clock_t start, cp1, cp2, end, final;
	double cpu_time_used;
	start = clock();
  FMeta meta = load_file(argv[1]);
    cp1 = clock();
    cpu_time_used = ((double) (cp1- start)) / CLOCKS_PER_SEC;
    printf("CPU Time for INITIAL LOAD (common and needed): %f\n", cpu_time_used);
  if(meta.hndl) {
    //printf("Size: %lu\n", meta.sz);
    size_t line = line_count(meta);
    cp2 = clock();
    cpu_time_used = ((double) (cp2- cp1)) / CLOCKS_PER_SEC;
    printf("CPU Time for SECOND PASS (obtain line #, not needed): %f\n", cpu_time_used);
    //printf("Line Count: %lu\n", line);
    SortBlock blk = gen_kvps(meta, line, atoi(argv[2])); 
    end = clock();
    cpu_time_used = ((double) (end - cp2)) / CLOCKS_PER_SEC;
    printf("CPU Time for CACHING KEYS and WRAPPING: %f\n", cpu_time_used);
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("CPU Time for ENTIRE LOAD: %f\n", cpu_time_used);
    qsort(blk.pairs, line, sizeof(Kvp), cmpkey);
    final = clock();
    cpu_time_used = ((double) (final - end)) / CLOCKS_PER_SEC;
    printf("CPU Time for SORTING: %f\n", cpu_time_used);
    //cb_pchain(blk.cache.root);    
    //print_sblock(&blk, line);
    cl_delete(&blk.cache);   
    free(blk.pairs);
    free(meta.hndl);
  }
  return 0;
}
