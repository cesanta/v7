/*
 * Copyright (c) 2015 Cesanta Software Limited
 * All rights reserved
 */

#include "internal.h"
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
int64_t strtoll(const char *, char **, int);
#endif

/* ECMA5.1 defines "extended years", -+285,426 years from 01 January, 1970 UTC
 * As result we cannot use standart break & make time functions */

struct timeparts {
  int year; /* can be negative */
  int month; /* 1-12 */
  int day; /* 1-31 */
  int hour; /* 0-23 */
  int min; /* 0-59 */
  int sec; /* 0-59 */
  int msec; 
};

/* TODO(alashkin): replace mktime etc to something with extyears support */

static uint64_t d_mktime(struct timeparts* tp) {
  struct tm t;
  memset(&t, 0, sizeof(t));
  
  t.tm_year = tp->year - 1900;
  t.tm_mon = tp->month;
  t.tm_mday = tp->day;
  t.tm_hour = tp->hour;
  t.tm_min = tp->min;
  t.tm_sec = tp->sec;
  
  return (uint64_t)mktime(&t)*1000 + tp->msec;
}

static void d_gmtime(uint64_t* time, struct timeparts* tp) {
  time_t sec = *time/1000;
  time_t msec = *time%1000;
  struct tm t;
  gmtime_r(&sec, &t); 

  tp->year = t.tm_year + 1900;
  tp->month = t.tm_mon;
  tp->day = t.tm_mday;
  tp->hour = t.tm_hour;
  tp->min = t.tm_min;
  tp->sec = t.tm_sec;
  tp->msec = msec;
}

static int d_timetoUTCstr(uint64_t* time, char* buf, size_t buf_size) {
  /* ISO format: "+XXYYYY-MM-DDTHH:mm:ss.sssZ"; */
  struct timeparts tp;
  char use_ext = 0;
  const char* ey_frm="%06d-%02d-%02dT%02d:%02d:%02d.%03dZ";
  const char* simpl_frm="%04d-%02d-%02dT%02d:%02d:%02d.%03dZ";
  
  (void)buf_size;
  
  d_gmtime(time, &tp);

  if(abs(tp.year) > 9999) {
    *buf = tp.year>0?'+':'-';
    use_ext = 1;
  }
  
  return snprintf(buf + use_ext, buf_size - use_ext, 
        use_ext? ey_frm: simpl_frm, tp.year, tp.month + 1, tp.day, tp.hour, tp.min, tp.sec, tp.msec) + use_ext;
}

static void d_gettime(uint64_t* time) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  *time = (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;  
}

static val_t Date_ctor(struct v7 *v7, val_t this_obj, val_t args) {
  uint64_t ret_time = 0;   
  if(v7_is_object(this_obj) && this_obj != v7->global_object) {
    int cargs = v7_array_length(v7, args);
    if(cargs <=0 ) {
      /* no parameters - return current date & time */
      d_gettime(&ret_time);
    } else if(cargs == 1) {
      /* one parameter - should be another Date object */
      val_t obj = v7_array_at(v7, args, 0);
      /* TODO(alashkin): check this statement */
      if(!v7_is_object(obj) || (v7_is_object(obj) && v7_to_object(obj)->prototype != v7_to_object(v7->date_prototype))) {
        throw_exception(v7, "TypeError","%s","Date expected");
      }
      ret_time = v7_to_double(i_value_of(v7, obj));
    } else {
      /* 2+ paramaters - should be parts of a date */
      struct timeparts tp;
      int i, cvt_args[7];
      
      memset(&tp, 0, sizeof(tp));
      tp.day = 1; /* If date is supplied then let dt be ToNumber(date); else let dt be 1. */
      

      for(i=0; i< cargs; i++) {
        val_t val = v7_array_at(v7, args, i);
        if(v7_is_double(val)) {
          cvt_args[i] = v7_to_double(val);
        } else if(v7_is_string(val)) {
          size_t size;
          const char* str = v7_to_string(v7, &val, &size);
          char* endptr;
          cvt_args[i] = strtoll(str, &endptr, 10);
          if(endptr != str + size) {
            throw_exception(v7, "Range error", "Invalid Date: %s", str);
          }
        } else {
          throw_exception(v7, "TypeError", "%s", "Number or String expected");
        }
      }
      
      switch (cargs) {
        case 7: 
          tp.msec = cvt_args[6];
        case 6: 
          tp.sec = cvt_args[5];
        case 5: 
          tp.min = cvt_args[4];
        case 4:
          tp.hour = cvt_args[3];
        case 3: 
          tp.day = cvt_args[2];
        case 2: 
          tp.month = cvt_args[1];
          tp.year = cvt_args[0];
          if(tp.year >=0 && tp.year <=99) {
            tp.year += 1900; /* If y is not NaN and 0 <= ToInteger(y) <= 99, then let yr be 1900+ToInteger(y); otherwise, let yr be y. */
          }
      }
      
      ret_time = (uint64_t)d_mktime(&tp);
    }
  
    v7_to_object(this_obj)->prototype = v7_to_object(v7->date_prototype);
    v7_set_property(v7, this_obj, "", 0, V7_PROPERTY_HIDDEN, v7_create_number(ret_time));
    return this_obj;
  } else {
    /* according to 15.9.2.1 we should ignore all parameters in case of function-call */    
    char buf[30];
    int len;
    
    d_gettime(&ret_time);    
    len = d_timetoUTCstr(&ret_time, buf, sizeof(buf));
    return v7_create_string(v7, buf, len, 1);
  }
}

static val_t Date_toISOString(struct v7 *v7, val_t this_obj, val_t args) {
  char buf[30];
  uint64_t time; int len;
  (void)args;
  
  time = v7_to_double(i_value_of(v7, this_obj));
  len = d_timetoUTCstr(&time, buf, sizeof(buf));
  
  return v7_create_string(v7, buf, len, 1);
}

static val_t Date_getDate(struct v7 *v7, val_t this_obj, val_t args) {
  struct timeparts tp;
  uint64_t time;
  (void)args;
  
  time = v7_to_double(i_value_of(v7, this_obj));
  d_gmtime(&time, &tp);
  
  return v7_create_number(tp.day);
}

static val_t Date_valueOf(struct v7 *v7, val_t this_obj, val_t args) {
  (void)args;
  if(!v7_is_object(this_obj) || (v7_is_object(this_obj) && v7_to_object(this_obj)->prototype != v7_to_object(v7->date_prototype))) {
    throw_exception(v7, "TypeError",
                    "Date.valueOf called on non-number object");
  }
  
  return Obj_valueOf(v7, this_obj, args);
}

V7_PRIVATE void init_date(struct v7 *v7) { 
  val_t date = v7_create_cfunction(Date_ctor);
  v7_set_property(v7, v7->global_object, "Date", 4, 0, date);
  v7_set(v7, v7->date_prototype, "constructor", 11, date);
  set_cfunc_prop(v7, v7->date_prototype, "getDate", Date_getDate);
  set_cfunc_prop(v7, v7->date_prototype, "toISOString", Date_toISOString);
  set_cfunc_prop(v7, v7->date_prototype, "valueOf", Date_valueOf);  
}
