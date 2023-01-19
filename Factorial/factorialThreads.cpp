#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

int main(int argc, char* argv[]) {
	if (argc > 2) {
		printf("too many arguments\n");
		return -1;
	}
	
	int a;
	printf("Enter a number: ");
	scanf("%d", &a);
	if (a < 0) {
		printf("factorial is only for not negative numbers\n");
		return -1;
	}
	if (a == 0) {
		printf("0! = 1\n");
		return 0;
	}
	if (a == 1) {
		printf("1! = 1\n");
		return 0;
	}
	
	int num;
	if (argc == 2) {
		num = atoi(argv[1]);
	}
	else {
		num = sysconf(_SC_NPROCESSORS_ONLN) + 1;
	}
	
	long long res = 1;
	omp_set_dynamic(0);
	omp_set_num_threads(num);

	#pragma omp parallel for reduction(* : res)
	for (int i = 2; i <= a; i++) {
		res *= i;
	}
	
	printf("%d! = %lld\n", a, res);
	return 0;
}
