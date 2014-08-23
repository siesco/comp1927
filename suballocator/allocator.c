/*
 * COMP1927 Assignment 1 - Memory Suballocator
 * allocator.c ... implementation
 *
 * Created by Liam O'Connor on 18/07/12.
 * Modified by John Shepherd in August 2014
 * Remodified by Matthew Siesco on 22/08/2014
 * Copyright (c) 2012-2014 UNSW. All rights reserved.
 *
 * To-do-list:
 *
 *
 *
 *
 */



#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define HEADER_SIZE    sizeof(struct free_list_header)  
#define MAGIC_FREE     0xDEADBEEF
#define MAGIC_ALLOC    0xBEEFDEAD

typedef unsigned char byte;
typedef u_int32_t vlink_t;
typedef u_int32_t vsize_t;
typedef u_int32_t vaddr_t;

typedef struct free_list_header {
   u_int32_t magic;           // ought to contain MAGIC_FREE
   vsize_t size;              // # bytes in this block (including header)
   vlink_t next;              // memory[] index of next free block
   vlink_t prev;              // memory[] index of previous free block
} free_header_t;

typedef free_header_t *header;

static void *traversal(int n);
static int regionSpace(header h, int n);
static int regionFree(header h);

// Global data

static byte *memory = NULL;   // pointer to start of suballocator memory
static vaddr_t free_list_ptr; // index in memory[] of first block in free list
static vsize_t memory_size;   // number of bytes malloc'd in memory[]


void sal_init(u_int32_t size)
{
   int expo = 0;
	while((1<<expo) < size){
		expo++;
	}
	
	memory = malloc(sizeof(byte)*(1<<expo));

	if(memory == NULL){
		fprintf(stderr,"sal_init: insufficient memory");
		abort();
	}

	free_list_ptr = 0;
	memory_size = 1<<expo;

	header initial = (header)&(memory[0]);

	initial->size = memory_size;
	initial->magic = MAGIC_FREE;
	initial->prev = free_list_ptr;
	initial->next = free_list_ptr; 
}

static int regionFree(header h) {
    return (h->magic == MAGIC_FREE);
}

static int regionSpace(header h, int n) {
    return (h->size >= (n + HEADER_SIZE));
}

static int canSplit(header h, int n) {
   return ((h->size / 2 ) >= (n + HEADER_SIZE));
}

static void split(header h, int n) {

   h->size /= 2;
   header newH = (header)&(memory[h->size]); 
   newH->magic = MAGIC_FREE;
   newH->size = h->size;
 
   if(h->next != free_list_ptr && h->prev != free_list_ptr) {
      header next = (header)&(memory[h->next]);
      header prev = (header)&(memory[h->prev]);

      newH->next = h->next;
      newH->prev = h->prev;
      h->next = h->size;

      //rearrange adjacent memory block's pointers
      next->prev = free_list_ptr + h->size;
      prev->next = free_list_ptr + h->size;
   } else if (h->next == h->prev) {
      h->next = h->size;
      h->prev = h->size;
      newH->prev = free_list_ptr;
      newH->next = free_list_ptr;
   }
}

static void *traversal(int n) {
   unsigned char *point = &(memory[free_list_ptr]);
   unsigned char *tmp = &(memory[free_list_ptr]);
   do {
      header detection = (header)point;
      if(regionFree(detection) && regionSpace(detection, n)) {
         while(canSplit(detection, n)) {
            split(detection, n);
         }

         detection->magic = MAGIC_ALLOC;
         int moot = free_list_ptr;

         free_list_ptr = detection->next;
         return ((void *)(moot + HEADER_SIZE));
      }
      free_list_ptr = detection->next;
      point = &(memory[detection->next]);

      header checker = (header)point;

      // incase there are no more regions
      if(checker->magic == MAGIC_ALLOC) break;

   } while (point != tmp);

   return NULL;
}

void *sal_malloc(u_int32_t n) {
   void *locationReturn = traversal(n);
   if(locationReturn != NULL) {
      return locationReturn; //HEADER_SIZE has already been taken into account
   } else {
      fprintf(stderr, "Not enough memory bro\n");
      //abort();
      return NULL; // temporarily
   }
}

void sal_free(void *object)
{
   // TODO
}

void sal_end(void)
{
   free(memory);
}

void sal_stats(void)
{
   // Optional, but useful
   printf("sal_stats\n");
    // we "use" the global variables here
    // just to keep the compiler quiet
   memory = memory;
   free_list_ptr = free_list_ptr;
   memory_size = memory_size;
}

