#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("To run useless program you shoud type name of the input file in arguments.\n");
		return -1;
	}

	FILE *f;
	f = fopen(argv[1], "r");

	if (f == NULL) {
		printf("No such input file.\n");
		return -1;
	}

	char str[FILENAME_MAX] = {};
	int t = 0;

	while (fscanf(f, "%d %s", &t, str) == 2) {
		printf("%s will be executed in %d seconds\n", str, t);

		if (t < 0) {
			printf("Incorrect time of delay in the input file.\n");
			return -1;
		}

		pid_t pid = fork();
		if (pid == -1) {
			printf("Fork failed.\n");
			return -1;
		} 
		else if (pid == 0) {
			sleep(t);
			// Если это системный вызов, то выполнится следующая строка
			execlp(str, str, NULL);
			// Если предыдущая строка не выполнилась, значит это не системный вызов, и выполнятся следующие строки
			char s[FILENAME_MAX] = "./";
			system(strcat(s, str));
			return 0;
		}
	}

	// Ждем завершения всех дочерних процессов
	while (wait(NULL) != -1) {};
	fclose(f);
	return 0;
}
