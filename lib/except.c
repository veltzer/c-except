#define _GNU_SOURCE
#include <dlfcn.h>  // for dlsym(3), dlerror(3)
#include <stdbool.h> // for type bool
#include <stdio.h> // for printf(3), perror(3)

#include <stdlib.h> // for malloc(3)
#include <sys/ioctl.h> // for ioctl(2)

#include "except.h"
#include "exceptcc.h"

// stringify macros
#define __stringify_1(x) # x
#define __stringify(x) __stringify_1(x)

// ISO C has no conversion from the object pointer dlsym(3) returns to a
// function pointer: store it through a void**, as POSIX documents.
#define register_name(name) *(void **)(&p_ ## name)=dlsym(RTLD_NEXT, __stringify(name));\
	if(p_ ## name==NULL) {\
		fprintf(stderr,"error in dlsym on symbol [%s], [%s]\n",__stringify(name),dlerror());\
		exit(1);\
	}

// do you want to throw exceptions?
static const bool throw=false;
// do you want to debug the library?
static const bool debug=false;
//static const bool debug=true;

// the functions
static int (*p_ioctl)(int,unsigned long int,...);
static void* (*p_malloc)(size_t);

static inline void except_debug(const char* msg) {
	if(debug) {
		printf("[%s]\n",msg);
	}
}

void except_init(void) __attribute__((constructor));
void except_init(void) {
	except_debug("in except_init");
	register_name(ioctl);
	register_name(malloc);
}

/*
 * Right now we don't need a destructor
 */
/*
void except_fini(void) __attribute__((destructor));
void except_fini(void) {
	except_debug("in except_fini");
}
*/

static inline void except_error(const char* name) {
	if(throw) {
		except_throw(name);
	} else {
		perror(name);
		//fprintf(stderr,"error occured in syscall [%s]\n",name);
		exit(1);
	}
}

int ioctl(int d,unsigned long int request,...) {
	// BUG!!! pass the extra argument as well.
	except_debug("in ioctl");
	int ret=p_ioctl(d,request);
	if(ret==-1) {
		except_error("ioctl");
	}
	return ret;
}

void* malloc(size_t size) {
	except_debug("in malloc");
	void* ret=p_malloc(size);
	if(ret==NULL) {
		except_error("malloc");
	}
	return ret;
}
