#include "utils.hpp"

#ifndef HAVE_CXXSTDLIB
// http://stackoverflow.com/questions/920500/what-is-the-purpose-of-cxa-pure-virtual
extern "C" void __cxa_pure_virtual() { while (1); }

// http://stackoverflow.com/questions/7015285/undefined-reference-to-operator-deletevoid
#include <stdlib.h>
void * operator new(size_t n)
{
	void * const p = malloc(n);
	// handle p == 0
	return p;
}

void operator delete(void * p) // or delete(void *, std::size_t)
{
	free(p);
}

// http://www.avrfreaks.net/comment/341297#comment-341297
extern "C" int __cxa_guard_acquire(__guard *g) {return !*(char *)(g);};
extern "C" void __cxa_guard_release (__guard *g) {*(char *)g = 1;};
extern "C" void __cxa_guard_abort (__guard *) {};

#endif //HAVE_CXXSTDLIB
