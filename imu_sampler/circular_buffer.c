#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include "circular_buffer.h"
#include "offset_memcpy.h"


struct cbuffer_t {
  void *buffer;
  size_t write_pos;
  size_t read_pos; 	
  size_t max_elements; //of the buffer
  bool full;
  bool print; //enable logging
  size_t element_size; //size of each element in bytes
  pthread_mutex_t lock;
};



cbuffer_handle_t empty_cbuffer()
{
  return (cbuffer_handle_t) malloc(sizeof(cbuffer_t));
}

int cbuffer_init(cbuffer_handle_t cbuf,size_t _max_elements,size_t _element_size)
{	
  // Fill the data structure
  cbuf->buffer= malloc(_max_elements*_element_size);
  cbuffer_buf_reset(cbuf);
  cbuf->element_size=_element_size;
  cbuf->max_elements=_max_elements;
  cbuf->print = false;
	
  if (pthread_mutex_init(&cbuf->lock, NULL) != 0)
    {
      printf("Mutex init failed.\n");
      return -1;
    }
	
  return 0;
	
}

void cbuffer_buf_reset(cbuffer_handle_t cbuf)
{
  if(cbuf->print) printf("Reset buffer\n");
  cbuf->write_pos = 0;
  cbuf->read_pos = 0;
  cbuf->full = false;
}

void cbuffer_enable_print_outs(cbuffer_handle_t cbuf)
{
  cbuf->print = true;
  if(cbuf->print) printf("Print out enabled\n");
}

void cbuffer_disable_print_outs(cbuffer_handle_t cbuf)
{
  if(cbuf->print) printf("Print out disabled\n");
  cbuf->print = false;
	
}


int cbuffer_free(cbuffer_handle_t cbuf)
{
  if(cbuf->print) printf("Freeing buffer\n");
  free(cbuf->buffer);
  pthread_mutex_destroy(&cbuf->lock);
  free(cbuf);
  return 0;
}

size_t cbuffer_capacity(cbuffer_handle_t cbuf)
{
  return cbuf->max_elements;
}

bool cbuffer_full(cbuffer_handle_t cbuf)
{
  return cbuf->full;
}

bool cbuffer_empty(cbuffer_handle_t cbuf)
{
  return (!cbuf->full && (cbuf->write_pos == cbuf->read_pos));
}

size_t cbuffer_size(cbuffer_handle_t cbuf)
{

  size_t size = cbuf->max_elements;

  if(!cbuf->full)
    {
      if(cbuf->write_pos >= cbuf->read_pos)
        {
	  size = (cbuf->write_pos - cbuf->read_pos);
        }
      else
        {
	  size = (cbuf->max_elements + cbuf->write_pos - cbuf->read_pos);
        }
    }
  if(cbuf->print) printf("Query of size. Size is %d\n",size);
  return size;
}

// helper function for advancing write position and read position if buffer is full
static void advance_pointer(cbuffer_handle_t cbuf)
{
  if(cbuf->print) printf("Advancing pointer\n");
	
  if(cbuf->full)
    {
      cbuf->read_pos = (cbuf->read_pos + 1) % cbuf->max_elements;
    }

  cbuf->write_pos = (cbuf->write_pos + 1) % cbuf->max_elements;
  cbuf->full = (cbuf->write_pos == cbuf->read_pos);
	
  if (cbuf->full)
    {
      if(cbuf->print) printf("Buffer is now full\n");
    }
}

// helper function for advancing read position
static void retreat_pointer(cbuffer_handle_t cbuf)
{
  if(cbuf->print) printf("Retreating pointer\n");
  cbuf->full = false;
  cbuf->read_pos = (cbuf->read_pos + 1) % cbuf->max_elements;
}

int cbuffer_put(cbuffer_handle_t cbuf, void * in_data)
{
  /* 	if (cbuf->element_size != data_size )
	{
	printf("Size of added element does not match the size specified at init.\n");
	return -1;
	} */
	
  pthread_mutex_lock(&cbuf->lock);
  if(cbuf->print) printf("Put-rutine got lock\n");
	
  memcpy_offset_dest(cbuf->buffer,in_data, cbuf->element_size * cbuf->write_pos, cbuf->element_size);
  if(cbuf->print) printf("Put data at pos %d\n",cbuf->write_pos );
	
  advance_pointer(cbuf);
	
  pthread_mutex_unlock(&cbuf->lock);
  if(cbuf->print) printf("Put-routine released lock\n");
  return 0;
	
}

int cbuffer_get(cbuffer_handle_t cbuf, void * out_data)
{

  if(!cbuffer_empty(cbuf))
    {
      pthread_mutex_lock(&cbuf->lock);
      if(cbuf->print) printf("Get-rutine got lock\n");
      memcpy_offset_source(out_data,cbuf->buffer, cbuf->element_size * cbuf->read_pos, cbuf->element_size);
		
      retreat_pointer(cbuf);
		
      pthread_mutex_unlock(&cbuf->lock);
      if(cbuf->print) printf("Get-routine released lock\n");
      return 0;
    }
  if(cbuf->print) printf("Attempted to read empty buffer\n");
  return -1; // should this be !?
}






