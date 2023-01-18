#include <sys/types.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Перемножение
void multiply(long long array[], int n) {
	for (int i = 1; i < n; i++) {
		array[0] *= array[i];
	}
}

// Вычисление факториала
void factorial(long long array[], int l, int r, int num[]) {
	int i;
	int len = r - l + 1;
	
	// Маленькие части просто перемножаем
	if (len <= 2) {
		multiply(array + l, len);
		return;
	}
	// Если осталось больше трех свободных вычислителей (1 на родительский и 2 на дочерние)
	if (num[0] >= 3) {
		pid_t lpid, rpid;
		lpid = fork();
		if (lpid < 0) {
			printf("left fork error\n");
			exit(-1);
		}
		else if (lpid == 0) {
			--num[0];
			factorial(array, l, l + len / 2 - 1, num);
			++num[0];
			exit(0);
		}
		else {
			rpid = fork();
			if (rpid < 0) {
				printf("right fork error\n");
				exit(-1);
			}
			else if(rpid == 0) {
				--num[0];
				factorial(array, l + len / 2, r, num);
				++num[0];
				exit(0);
			}
		}

		int status;
		waitpid(lpid, &status, 0);
		waitpid(rpid, &status, 0);
	}
	// Иначе запускаем просто реккурсивно
	else {
		factorial(array, l, l + len / 2 - 1, num);
		factorial(array, l + len / 2, r, num);
	}

	array[l] = array[l] * array[l + len / 2];
	return;
}

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
	
	// Создаем область разделяемой памяти для кол-ва вычислителей
	int shmid_num = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
	if (shmid_num < 0) {
		printf("shmget num error\n");
		return -1;
	}
	int* num = (int*)shmat(shmid_num, NULL, 0);
	
	if (argc == 2) {
		num[0] = atoi(argv[1]);
	}
	else {
		num[0] = 4000;
	}

	// Создаем область разделяемой памяти для массива чисел
	int shmid = shmget(IPC_PRIVATE, sizeof(long long) * a, IPC_CREAT | 0666);
	if (shmid < 0) {
		printf("shmget array error\n");
		return -1;
	}
	long long* array = (long long*)shmat(shmid, NULL, 0);

	// Заполняем массив
	for (int i = 0; i < a; i++) {
		array[i] = i + 1;
	}
	
	factorial(array, 0, a - 1, num);
	printf("%d! = %lld\n", a, array[0]);

	// Удаляем области разделяемой памяти
	shmdt(array);
	shmctl(shmid, IPC_RMID, NULL);
	shmdt(num);
	shmctl(shmid_num, IPC_RMID, NULL);

	return 0;
}
