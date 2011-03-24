#include <stdlib.h>
#include <stdarg.h>
#include "codegen.h"

#define ALLOC(p, t)	p = (t*)malloc(sizeof(t)); if(!p) return NULL;
#define ALLOCN(p, n)	p = (char*)malloc(n); if(!p) return NULL;

int *ialloc(int n) {
  return (int*)malloc(n * sizeof(int));
}

void ifree(int *arr) {
  free(arr);
}

void *carralloc_helper(int dim, IntListNode *sizes) {
  void *ptr;
  if(sizes) {
    int elem_size = ( dim > 0 ) ? SIZE_P : SIZE_CHAR;
    ALLOCN(ptr, sizes->n * elem_size);
    if(sizes->next) {
      void **ptr2;
      int i;
      ptr2 = (void**)ptr;
      for(i = 0; i < sizes->n; i++)
        ptr2[i] = carralloc_helper(dim - 1, sizes->next);
    }
  }
  return ptr;
}

void arrfree_helper(void *ptr, IntListNode *sizes) {
  if(sizes) {
    if(sizes->next) {
      void **ptr2;
      int i;
      ptr2 = (void**)ptr;
      for(i = 0; i < sizes->n; i++)
        arrfree_helper(ptr2[i], sizes->next);
    }
  }
  free(ptr);
}

void *iarralloc_helper(int dim, IntListNode *sizes) {
  void *ptr;
  if(sizes) {
    int elem_size = ( dim > 0 ) ? SIZE_P : SIZE_INT;
    ALLOCN(ptr, sizes->n * elem_size);
    if(sizes->next) {
      void **ptr2;
      int i;
      ptr2 = (void**)ptr;
      for(i = 0; i < sizes->n; i++)
        ptr2[i] = iarralloc_helper(dim - 1, sizes->next);
    }
  }
  return ptr;
}

void *iarralloc(int dim, int nsizes, ...) {
  va_list sizes;
  IntListNode *lsizes, *list;
  va_start(sizes, nsizes);
  ALLOC(lsizes, IntListNode);
  lsizes->n = va_arg(sizes, int);
  lsizes->next = NULL;
  list = lsizes;
  nsizes--;
  while(nsizes) {
    ALLOC(list->next, IntListNode);
    list = list->next;
    list->n = va_arg(sizes, int);
    list->next = NULL;
    nsizes--;
  }
  va_end(sizes);
  return iarralloc_helper(dim, lsizes);
}

void *carralloc(int dim, int nsizes, ...) {
  va_list sizes;
  IntListNode *lsizes, *list;
  va_start(sizes, nsizes);
  ALLOC(lsizes, IntListNode);
  lsizes->n = va_arg(sizes, int);
  lsizes->next = NULL;
  list = lsizes;
  nsizes--;
  while(nsizes) {
    ALLOC(list->next, IntListNode);
    list = list->next;
    list->n = va_arg(sizes, int);
    list->next = NULL;
    nsizes--;
  }
  va_end(sizes);
  return iarralloc_helper(dim, lsizes);
}


void* arrfree(void *ptr, int nsizes, ...) {
  va_list sizes;
  IntListNode *lsizes, *list;
  va_start(sizes, nsizes);
  ALLOC(lsizes, IntListNode);
  lsizes->n = va_arg(sizes, int);
  lsizes->next = NULL;
  list = lsizes;
  nsizes--;
  while(nsizes) {
    ALLOC(list->next, IntListNode);
    list = list->next;
    list->n = va_arg(sizes, int);
    list->next = NULL;
    nsizes--;
  }
  va_end(sizes);
  arrfree_helper(ptr, lsizes);
}
