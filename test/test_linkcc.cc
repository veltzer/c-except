#include <sys/ioctl.h> // for ioctl(2)
#include <stdlib.h> // for malloc(3)

int main() {
// the next pragmas remove warnings about not using the return values of the functions...
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
	// cppcheck-suppress leakReturnValNotUsed ; the library reports the failure, the result is deliberately dropped
	malloc(70000000);
	ioctl(6,9);
#pragma GCC diagnostic pop
	return 0;
}
