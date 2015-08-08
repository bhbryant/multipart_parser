/* Based on http_parser.rb by Aman Gupta
 * Benjamin Bryant - https://github.com/bhbryant/multipart_parser - 2015
 * MIT License - http://www.opensource.org/licenses/mit-license.php
 */

#include "ruby.h"
#include "ext_help.h"

#include <stdio.h>
#include <string.h>
#include "multipart_parser_c.h"

#define GET_WRAPPER(N, from)  MultipartParserWrapper *N = (MultipartParserWrapper *)(from)->context;





/* structure  to hold all the state for calling the parser */
typedef struct MultipartParserWrapper {

  multipart_parser_c* parser;

  VALUE headers;

  /* callbacks */
  VALUE on_message_begin;
  VALUE on_part_begin;
  VALUE on_headers_complete;
  VALUE on_data;
  VALUE on_part_complete;
  VALUE on_message_complete;

  VALUE callback_object;

  VALUE header_value_type;


  VALUE curr_field_name;
  VALUE last_field_name;

} MultipartParserWrapper;



static VALUE cMultipartParser;


static ID Icall; 

static ID Ion_message_begin;
static ID Ion_part_begin;
static ID Ion_headers_complete;
static ID Ion_data; 
static ID Ion_part_complete;
static ID Ion_message_complete;


static VALUE Sarrays;
static VALUE Sstrings;
static VALUE Smixed;

/* CALLBACKS from multipart_parser */


int on_message_begin(multipart_parser_c *parser) {
  GET_WRAPPER(wrapper, parser);

  VALUE ret = Qnil;

  if (wrapper->callback_object != Qnil && rb_respond_to(wrapper->callback_object, Ion_message_begin)) {
    ret = rb_funcall(wrapper->callback_object, Ion_message_begin, 0);
  } else if (wrapper->on_message_begin != Qnil) {
    ret = rb_funcall(wrapper->on_message_begin, Icall, 0);
  }

  return 0;
}

int on_part_begin(multipart_parser_c *parser) {
  GET_WRAPPER(wrapper, parser);

  /* new part, reset headers hash */
  wrapper->headers = rb_hash_new();

  VALUE ret = Qnil;

  if (wrapper->callback_object != Qnil && rb_respond_to(wrapper->callback_object, Ion_part_begin)) {
    ret = rb_funcall(wrapper->callback_object, Ion_part_begin, 0);
  } else if (wrapper->on_part_begin != Qnil) {
    ret = rb_funcall(wrapper->on_part_begin, Icall, 0);
  }

  return 0;
}


/* @internal */
int on_header_field(multipart_parser_c *parser , const char *at, size_t length) {
 

  GET_WRAPPER(wrapper, parser);

  if (wrapper->curr_field_name == Qnil) {
    wrapper->last_field_name = Qnil;
    wrapper->curr_field_name = rb_str_new(at, length);
  } else {
    rb_str_cat(wrapper->curr_field_name, at, length);
  }


  return 0;
}


/* @internal */
int on_header_value(multipart_parser_c *parser , const char *at, size_t length) {


  GET_WRAPPER(wrapper, parser);

  int new_field = 0;
  VALUE current_value;

  if (wrapper->last_field_name == Qnil) {
    new_field = 1;
    wrapper->last_field_name = wrapper->curr_field_name;
    wrapper->curr_field_name = Qnil;
  }

  current_value = rb_hash_aref(wrapper->headers, wrapper->last_field_name);

  if (new_field == 1) {
    if (current_value == Qnil) {
      if (wrapper->header_value_type == Sarrays) {
        rb_hash_aset(wrapper->headers, wrapper->last_field_name, rb_ary_new3(1, rb_str_new2("")));
      } else {
        rb_hash_aset(wrapper->headers, wrapper->last_field_name, rb_str_new2(""));
      }
    } else {
      if (wrapper->header_value_type == Smixed) {
        if (TYPE(current_value) == T_STRING) {
          rb_hash_aset(wrapper->headers, wrapper->last_field_name, rb_ary_new3(2, current_value, rb_str_new2("")));
        } else {
          rb_ary_push(current_value, rb_str_new2(""));
        }
      } else if (wrapper->header_value_type == Sarrays) {
        rb_ary_push(current_value, rb_str_new2(""));
      } else {
        rb_str_cat(current_value, ", ", 2);
      }
    }
    current_value = rb_hash_aref(wrapper->headers, wrapper->last_field_name);
  }

  if (TYPE(current_value) == T_ARRAY) {
    current_value = rb_ary_entry(current_value, -1);
  }

  rb_str_cat(current_value, at, length);

  return 0;
}


int on_headers_complete(multipart_parser_c *parser) {
  GET_WRAPPER(wrapper, parser);

  VALUE ret = Qnil;

  if (wrapper->callback_object != Qnil && rb_respond_to(wrapper->callback_object, Ion_headers_complete)) {
    ret = rb_funcall(wrapper->callback_object, Ion_headers_complete, 1, wrapper->headers);
  } else if (wrapper->on_headers_complete != Qnil) {
    ret = rb_funcall(wrapper->on_headers_complete, Icall, 1, wrapper->headers);
  }

  return 0;
}


int on_data(multipart_parser_c *parser , const char *at, size_t length) {


  GET_WRAPPER(wrapper, parser);
  

  VALUE ret = Qnil;

  if (wrapper->callback_object != Qnil && rb_respond_to(wrapper->callback_object, Ion_data)) {
      ret = rb_funcall(wrapper->callback_object, Ion_data, 1, rb_str_new(at, length));
  } else if (wrapper->on_data != Qnil) {
    ret = rb_funcall(wrapper->on_data, Icall, 1, rb_str_new(at, length));
  }


  return 0;
}





int on_part_complete(multipart_parser_c *parser) {

    GET_WRAPPER(wrapper, parser);

  VALUE ret = Qnil;

  if (wrapper->callback_object != Qnil && rb_respond_to(wrapper->callback_object, Ion_part_complete)) {
    ret = rb_funcall(wrapper->callback_object, Ion_part_complete, 0);
  } else if (wrapper->on_part_complete != Qnil) {
    ret = rb_funcall(wrapper->on_part_complete, Icall, 0);
  }

  return 0;
}

int on_message_complete(multipart_parser_c *parser) {

  GET_WRAPPER(wrapper, parser);

  VALUE ret = Qnil;

  if (wrapper->callback_object != Qnil && rb_respond_to(wrapper->callback_object, Ion_message_complete)) {
    ret = rb_funcall(wrapper->callback_object, Ion_message_complete, 0);
  } else if (wrapper->on_message_complete != Qnil) {
    ret = rb_funcall(wrapper->on_message_complete, Icall, 0);
  }

  return 0;
}




static multipart_parser_c_settings settings =  {
  .on_header_field = on_header_field,
  .on_header_value = on_header_value,

  .on_message_begin = on_message_begin,
  .on_part_begin = on_part_begin,
  .on_headers_complete = on_headers_complete,
  .on_part_data = on_data,
  .on_part_complete = on_part_complete,
  .on_message_complete = on_message_complete
};



/* EXPOSED METHODS */

/*
   MultipartParser#new(boundary)
   MultipartParser#new(boundary, callback_object)
   MultipartParser#new(boundary, callback_object, default_header_value_type) */
VALUE MultipartParser_initialize(int argc, VALUE *argv, VALUE self) {

  MultipartParserWrapper *wrapper = NULL;
  DATA_GET(self, MultipartParserWrapper, wrapper);

  VALUE boundary;

  wrapper->header_value_type = rb_iv_get(CLASS_OF(self), "@default_header_value_type");


  if (argc == 0 ) {
    rb_raise(rb_eArgError, "wrong number of arguments (0 for 1)");
  }

  if (argc == 1) {
    boundary = argv[0];
  }
  if (argc == 2) {
    boundary = argv[0];
    wrapper->callback_object = argv[1];
  }

  if (argc == 3) {
    boundary = argv[0];
    wrapper->callback_object = argv[1];
    wrapper->header_value_type = argv[2];
  }

  Check_Type(boundary, T_STRING);
  char *ptr = RSTRING_PTR(boundary);
  long len = RSTRING_LEN(boundary);


  
  wrapper->parser = multipart_parser_c_init(ptr,len);
  wrapper->parser->context = wrapper;



  return self;
}


/* MultipartParser# << (data) */
VALUE MultipartParser_execute(VALUE self, VALUE data) {
  MultipartParserWrapper *wrapper = NULL;

  Check_Type(data, T_STRING);
  char *ptr = RSTRING_PTR(data);
  long len = RSTRING_LEN(data);

  DATA_GET(self, MultipartParserWrapper, wrapper);


  size_t nparsed = multipart_parser_c_execute(wrapper->parser, &settings, ptr,  len);


  return INT2FIX(nparsed);
}


/* MultipartParser#on_message_begin= */
VALUE MultipartParser_set_on_message_begin(VALUE self, VALUE callback) {
  MultipartParserWrapper *wrapper = NULL;
  DATA_GET(self, MultipartParserWrapper, wrapper);


  if (rb_class_of(callback) != rb_cProc) {
    rb_raise(rb_eTypeError, "Expected Proc callback");
  }         

  wrapper->on_message_begin = callback;

  return callback;
}



/* MultipartParser#on_part_begin= */
VALUE MultipartParser_set_on_part_begin(VALUE self, VALUE callback) {
  MultipartParserWrapper *wrapper = NULL;
  DATA_GET(self, MultipartParserWrapper, wrapper);


  if (rb_class_of(callback) != rb_cProc) {
    rb_raise(rb_eTypeError, "Expected Proc callback");
  }         

  wrapper->on_part_begin = callback;

  return callback;
}



/* MultipartParser#on_headers_complete= */
VALUE MultipartParser_set_on_headers_complete(VALUE self, VALUE callback) {
  MultipartParserWrapper *wrapper = NULL;
  DATA_GET(self, MultipartParserWrapper, wrapper);


  if (rb_class_of(callback) != rb_cProc) {
    rb_raise(rb_eTypeError, "Expected Proc callback");
  }         

  wrapper->on_headers_complete = callback;

  return callback;
}


/* MultipartParser#on_data= */
VALUE MultipartParser_set_on_data(VALUE self, VALUE callback) {
  MultipartParserWrapper *wrapper = NULL;
  DATA_GET(self, MultipartParserWrapper, wrapper);


  if (rb_class_of(callback) != rb_cProc) {
    rb_raise(rb_eTypeError, "Expected Proc callback");
  }         

  wrapper->on_data = callback;

  return callback;
}


/* MultipartParser#on_part_complete= */
VALUE MultipartParser_set_on_part_complete(VALUE self, VALUE callback) {
  MultipartParserWrapper *wrapper = NULL;
  DATA_GET(self, MultipartParserWrapper, wrapper);


  if (rb_class_of(callback) != rb_cProc) {
    rb_raise(rb_eTypeError, "Expected Proc callback");
  }         

  wrapper->on_part_complete = callback;

  return callback;
}

/* MultipartParser#on_message_complete= */
VALUE MultipartParser_set_on_message_complete(VALUE self, VALUE callback) {
  MultipartParserWrapper *wrapper = NULL;
  DATA_GET(self, MultipartParserWrapper, wrapper);


  if (rb_class_of(callback) != rb_cProc) {
    rb_raise(rb_eTypeError, "Expected Proc callback");
  }         

  wrapper->on_message_complete = callback;

  return callback;
}




/* MultipartParser#headers */
VALUE MultipartParser_headers(VALUE self) {
  MultipartParserWrapper *wrapper = NULL;
  DATA_GET(self, MultipartParserWrapper, wrapper);

  return wrapper->headers;
}



/* MultipartParser#header_value_type */
VALUE MultipartParser_header_value_type(VALUE self) {
  MultipartParserWrapper *wrapper = NULL;
  DATA_GET(self, MultipartParserWrapper, wrapper);

  return wrapper->header_value_type;
}


/* MultipartParser#header_value_type= */
VALUE MultipartParser_set_header_value_type(VALUE self, VALUE val) {
  if (val != Sarrays && val != Sstrings && val != Smixed) {
    rb_raise(rb_eArgError, "Invalid header value type");
  }

  MultipartParserWrapper *wrapper = NULL;
  DATA_GET(self, MultipartParserWrapper, wrapper);
  wrapper->header_value_type = val;
  return wrapper->header_value_type;
}




/* EXTENSION ALLOCATION AND MEMORY MANAGEMENT */




/* marks objects that are used by this function so GC doesn't remove them */
void MultipartParserWrapper_mark(void *data) {
  if(data) {
    MultipartParserWrapper *wrapper = (MultipartParserWrapper *) data;

    rb_gc_mark_maybe(wrapper->on_message_begin);
    rb_gc_mark_maybe(wrapper->on_part_begin);
    rb_gc_mark_maybe(wrapper->on_headers_complete);
    rb_gc_mark_maybe(wrapper->on_data);
    rb_gc_mark_maybe(wrapper->on_part_complete);
    rb_gc_mark_maybe(wrapper->on_message_complete);


    rb_gc_mark_maybe(wrapper->headers);

    
    rb_gc_mark_maybe(wrapper->callback_object);

    rb_gc_mark_maybe(wrapper->curr_field_name);
    rb_gc_mark_maybe(wrapper->last_field_name);

  }
}

/* release memory */
void MultipartParserWrapper_free(void *data) {
  if(data) {

    MultipartParserWrapper *wrapper = (MultipartParserWrapper *) data;

    if(wrapper->parser) {
      multipart_parser_c_free(wrapper->parser);
    }
    free(data);
  }
}



VALUE MultipartParser_alloc(VALUE klass) {

  MultipartParserWrapper *wrapper = ALLOC_N(MultipartParserWrapper,1);

  wrapper->parser = NULL;

  wrapper->headers = Qnil;

  wrapper->on_message_begin = Qnil;
  wrapper->on_part_begin = Qnil;
  wrapper->on_headers_complete = Qnil;
  wrapper->on_data = Qnil;
  wrapper->on_part_complete= Qnil;
  wrapper->on_message_complete= Qnil;


  wrapper->callback_object = Qnil;

  wrapper->curr_field_name = Qnil;
  wrapper->last_field_name = Qnil;

  

  /* Data_Wrap_Struct(VALUE class, void (*mark)(), void (*free)(), void *ptr") */
  return  Data_Wrap_Struct(klass, MultipartParserWrapper_mark, MultipartParserWrapper_free, wrapper);

}



/* The initialization method for this module */
void Init_multipart_parser() {


  cMultipartParser = rb_define_class("MultipartParser", rb_cObject);

  rb_define_alloc_func(cMultipartParser, MultipartParser_alloc);

  Icall = rb_intern("call");

  Ion_message_begin = rb_intern("on_message_begin");
  Ion_part_begin = rb_intern("on_part_begin");
  Ion_headers_complete = rb_intern("on_headers_complete");
  Ion_data = rb_intern("on_data");
  Ion_part_complete = rb_intern("on_part_complete");
  Ion_message_complete = rb_intern("on_message_complete");



  Sarrays = ID2SYM(rb_intern("arrays"));
  Sstrings = ID2SYM(rb_intern("strings"));
  Smixed = ID2SYM(rb_intern("mixed"));


  rb_define_method(cMultipartParser, "initialize", MultipartParser_initialize, -1);

  rb_define_method(cMultipartParser, "on_message_begin=", MultipartParser_set_on_message_begin, 1);
  rb_define_method(cMultipartParser, "on_part_begin=", MultipartParser_set_on_part_begin, 1);
  rb_define_method(cMultipartParser, "on_headers_complete=", MultipartParser_set_on_headers_complete, 1);
  rb_define_method(cMultipartParser, "on_data=", MultipartParser_set_on_data, 1);
  rb_define_method(cMultipartParser, "on_part_complete=", MultipartParser_set_on_part_complete, 1);
  rb_define_method(cMultipartParser, "on_message_complete=", MultipartParser_set_on_message_complete, 1);



  rb_define_method(cMultipartParser, "<<", MultipartParser_execute, 1);


  rb_define_method(cMultipartParser, "headers", MultipartParser_headers, 0);


  rb_define_method(cMultipartParser, "header_value_type", MultipartParser_header_value_type, 0);
  rb_define_method(cMultipartParser, "header_value_type=", MultipartParser_set_header_value_type, 1);

}













