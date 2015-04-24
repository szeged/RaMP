#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

int main()
{
	void* mm;

	void* m = malloc(16);
	void* n = malloc(1024);
	free(m);

	mm = mmap(0, 10*1024, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

	printf("End 'test'\n");
	return 0;
}
