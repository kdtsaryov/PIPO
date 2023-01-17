#include <sys/types.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

// Обычная сортировка
void sort(int array[], int n) {
	for (int i = 1; i < n; i++) {
		int tmp = array[i];
		int j = i-1;
		while (j >= 0 && array[j] > tmp) {
			array[j+1] = array[j];
			j = j-1;
		}
		array[j+1] = tmp;
	}
}

// Слияние отсортированных частей
void merge(int array[], int l1, int r1, int r2) {
	int count = r2 - l1 + 1;
	int sorted[count];
	int i = l1;
	int k = r1 + 1;
	int m = 0;
	
	while (i <= r1 && k <= r2) {
		if (array[i] < array[k]) {
			sorted[m++] = array[i++];
		}
		else if (array[k] < array[i]) {
			sorted[m++] = array[k++];
		}
		else if (array[i] == array[k]) {
			sorted[m++] = array[i++];
			sorted[m++] = array[k++];
		}
	}
	while (i <= r1) {
		sorted[m++] = array[i++];
	}
	while (k <= r2) {
		sorted[m++] = array[k++];
	}
	for (i = 0; i < count; i++, l1++) {
		array[l1] = sorted[i];
	}
}

// Сортировка слиянием
void mergeSort(int array[], int l, int r, int num[]) {
	int i;
	int len = r - l + 1;
	
	// Маленькие части сортируем обычной сортировкой
	if (len <= 5) {
		sort(array + l, len);
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
			mergeSort(array, l, l + len / 2 - 1, num);
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
				mergeSort(array, l + len / 2, r, num);
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
		mergeSort(array, l, l + len / 2 - 1, num);
		mergeSort(array, l + len / 2, r, num);
	}
 
	merge(array, l, l + len / 2 - 1, r);
	return;
}

// Проверка на отсортированность
void isSorted(int array[], int length) {
	if (length == 1) {
		printf("Array is sorted.\n");
		return;
	}
	for (int i = 1; i < length; i++) {
		if (array[i] < array[i-1]) {
			printf("Array is NOT sorted.\n");
			return;
		}
	}
	printf("Array is sorted.\n");
	return;
}

int main() {
	int length;
 
	printf("Length of array: ");
	scanf("%d", &length);
	
	// Создаем область разделяемой памяти для кол-ва вычислителей
	int shmid_num = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
	if (shmid_num < 0) {
		printf("shmget num error\n");
		return -1;
	}
	int* num = (int*)shmat(shmid_num, NULL, 0);
	
	printf("Num of child processes: ");
	scanf("%d", &num[0]);
	if (num[0] > 4000) {
		num[0] = 4000;
	}
 
	// Создаем область разделяемой памяти для массива
	int shmid = shmget(IPC_PRIVATE, sizeof(int) * length, IPC_CREAT | 0666);
	if (shmid < 0) {
		printf("shmget array error\n");
		return -1;
	}
	int* array = (int*)shmat(shmid, NULL, 0);

	// Заполняем массив
	srand(time(NULL));
	for (int i = 0; i < length; i++) {
		array[i] = rand();
	}
	
	// Проверяем на отсортированность, сортируем и снова проверяем
	isSorted(array, length);
	mergeSort(array, 0, length-1, num);
	isSorted(array, length);

	// Удаляем области разделяемой памяти
	shmdt(array);
	shmctl(shmid, IPC_RMID, NULL);
	shmdt(num);
	shmctl(shmid_num, IPC_RMID, NULL);

	return 0;
}
