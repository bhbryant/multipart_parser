/* Based on node-formidable by Felix Geisendörfer 
 * Igor Afonov - afonov@gmail.com - 2012
 * MIT License - http://www.opensource.org/licenses/mit-license.php
 * Modified Benjamin Bryant - https://github.com/bhbryant/multipart_parser - 2015
 */
#ifndef _multipart_parser_c_h
#define _multipart_parser_c_h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <ctype.h>

typedef struct multipart_parser_c multipart_parser_c;
typedef struct multipart_parser_c_settings multipart_parser_c_settings;
typedef struct multipart_parser_c_state multipart_parser_c_state;

typedef int (*multipart_data_cb) (multipart_parser_c*, const char *at, size_t length);
typedef int (*multipart_notify_cb) (multipart_parser_c*);

struct multipart_parser_c_settings {
  multipart_data_cb on_header_field;
  multipart_data_cb on_header_value;
  multipart_data_cb on_part_data;

  multipart_notify_cb on_message_begin;
  multipart_notify_cb on_part_begin;
  multipart_notify_cb on_headers_complete;
  multipart_notify_cb on_part_complete;
  multipart_notify_cb on_message_complete;
};

struct multipart_parser_c {
  void * data;

  size_t index;
  size_t boundary_length;

  unsigned char state;


  void * context; /* modified from original code to allow pointer wrapper to be passed to callbacks */

  char* lookbehind;
  char multipart_boundary[1];

};

/*modified from original code to move settings as an agument to execute fuction */
multipart_parser_c* multipart_parser_c_init(const char *boundary, size_t boundary_length);

void multipart_parser_c_free(multipart_parser_c* p);

/* modified from original code to allow take settings as argument */
size_t multipart_parser_c_execute(multipart_parser_c* p, const multipart_parser_c_settings* settings, const char *buf, size_t len);

void multipart_parser_c_set_data(multipart_parser_c* p, void* data);
void * multipart_parser_c_get_data(multipart_parser_c* p);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
