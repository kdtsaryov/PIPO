#include <sys/types.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("To run runsim program you shoud type the max number of processes.\n");
		return -1;
	}
	
	// Создаем область разделяемой памяти для максимального количества процессов
	int shmid_num = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
	if (shmid_num < 0) {
		printf("shmget num error\n");
		return -1;
	}
	int* num = (int*)shmat(shmid_num, NULL, 0);
	num[0] = atoi(argv[1]);
	
	while (!feof(stdin)) {
		char str[FILENAME_MAX] = {};
		scanf("%s", str);
		if (num[0] > 0) {
			pid_t pid = fork();
			if (pid == -1) {
				printf("Fork failed.\n");
				return -1;
			} 
			else if (pid == 0) {
				--num[0];
				system(str);
				++num[0];
				return 0;
			}
		}
		else {
			printf("Processes limit reached. Please, wait\n");	
		}
	}

	// Ждем завершения всех дочерних процессов
	while (wait(NULL) != -1) {};
	// Удаляем область разделяемой памяти
	shmdt(num);
	shmctl(shmid_num, IPC_RMID, NULL);

	return 0;
}
