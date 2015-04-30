#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

int main()
{
	int ret;
	void* p;
	char* mm;

	void* m = malloc(16);
	void* n = malloc(1024);
	free(m);

	free(calloc(10, 120));

	m = __libc_calloc(5,555);

	mm = mmap(0, 10*1024, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

	mm[0] = 'X';

	free(m);

	munmap(mm, 10*1024);

	p = memalign (11, 1234);

	ret = posix_memalign(&m, 32, 500);

	free(n);
	free(m);
	free(p);

	printf("End 'test'\n");
	exit(0);
	//return 0;
}
