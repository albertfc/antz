#pragma once

#define STATIC_ARRAY_SIZE_CHECK( _array, _size ) \
	static_assert( _size == (sizeof _array / sizeof _array[0]), "wrong size!" );

#ifndef HAVE_CXXSTDLIB

#define assert

__extension__ typedef int __guard __attribute__((mode (__DI__)));

#endif //HAVE_CXXSTDLIB
