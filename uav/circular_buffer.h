#include "stdlib.h"
#include <stdbool.h>
typedef struct cbuffer_t cbuffer_t;
typedef cbuffer_t* cbuffer_handle_t;


// Usage Example
/*
 cbuffer_handle_t my_int_buffer = create_cbuffer();
 cbuffer_init(my_int_buffer,10 ,sizeof(int) );
 int a =11;
 cbuffer_put(my_int_buffer,&a);
 a =12;
 cbuffer_put(my_int_buffer,&a);
 int b;
 cbuffer_get(my_int_buffer,&b);
 printf("read %d\n",b);

 cbuffer_free(my_int_buffer);
 */



// Returns a buffer handle pointing to a buffer on the heap
cbuffer_handle_t create_cbuffer();
// Must be called on the buffer before any operations are permitted
// The user is responsible for keeping track of which size the elements on the struct are supposed to be
// Writing a different size than declared at init may result in lost data
int cbuffer_init(cbuffer_handle_t, size_t _max_elements ,size_t _element_size);

// Resets the buffer
void cbuffer_buf_reset(cbuffer_handle_t );

// Call this when your program has finnished or when you want to delete the buffer
int cbuffer_free(cbuffer_handle_t);

// Put any element type on the buffer. I recommend not mixing different data types in the same buffer since this
// makes it nigh impossible to now what cbuffer_get() will give you
int cbuffer_put(cbuffer_handle_t, void * in_data);

// Get the oldest data. Will wait until buffer is free
int cbuffer_get(cbuffer_handle_t, void * out_data);

// Get newest data. Will wait until buffer is free
int cbuffer_top(cbuffer_handle_t cbuf, void * out_data);

// Try to get oldest data. Will return -1 if buffer is busy
int cbuffer_try_get(cbuffer_handle_t, void * out_data);

// How many elements can be stored
size_t cbuffer_capacity(cbuffer_handle_t);

// Is the buffer full?
bool cbuffer_full(cbuffer_handle_t);

// Is the buffer empty?
bool cbuffer_empty(cbuffer_handle_t);

// How many elements are unread/present?
size_t cbuffer_size(cbuffer_handle_t);

void cbuffer_enable_print_outs(cbuffer_handle_t);

void cbuffer_disable_print_outs(cbuffer_handle_t);
