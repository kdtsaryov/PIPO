#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

// Копирование
int cp(char* path_in, char* path_out){
	int fd_in = open(path_in, O_RDONLY);
	if (fd_in == -1) {
		return -1;
	}
	int fd_out = open(path_out, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd_out == -2) {
		return -2;
	}
	char buf[4096];
	int read_bytes = 0;
	int wrote_bytes = 0;
	while ((read_bytes = read(fd_in, &buf, sizeof(buf))) > 0) {
		wrote_bytes = write(fd_out, &buf, read_bytes);
		if (wrote_bytes != read_bytes) {
			return wrote_bytes - read_bytes;
		}
	}
	close(fd_in);
	close(fd_out);
	return 0;
}

// Сравнение
int cmp(char* path1, char* path2){
	char buf1[4096];
	memset(buf1, 0, sizeof(buf1));
	char buf2[4096];
	memset(buf2, 0, sizeof(buf2));
	int fd1 = open(path1, O_RDONLY);
	if (fd1 == -1) {
		return -1;
	}
	int fd2 = open(path2, O_RDONLY);
	if (fd2 == -1) {
		return -2;
	}
	int read_bytes1, read_bytes2;
	while ((read_bytes1 = read(fd1, &buf1, sizeof(buf1))) > 0  && (read_bytes2 = read(fd2, &buf2, sizeof(buf2))) > 0){	
		if (memcmp(buf1, buf2, 4096) != 0 || read_bytes1 != read_bytes2) {
			return 1;
		}
	}
	close(fd1);
	close(fd2);
	return 0;
}

// Резервное копирование
void backup(const char* f, const char* in, const char* out) {
    DIR *dir = opendir(in);
    struct dirent *entry = readdir(dir);
    if (!dir || !entry) {
		printf("Can't open directory \"%s\".\n", in);
		return;
	}

	do {
		char path_in[FILENAME_MAX];
		int len = snprintf(path_in, sizeof(path_in)-1, "%s/%s", in, entry->d_name);
		path_in[len] = 0;
		char path_out[FILENAME_MAX];
		len = snprintf(path_out, sizeof(path_out)-1, "%s/%s", out, entry->d_name);
		path_out[len] = 0;
        char path_gz[FILENAME_MAX];
		len = snprintf(path_gz, sizeof(path_gz)-1, "%s/%s.gz", out, entry->d_name);
 		path_gz[len] = 0;

		// Если директория
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
				continue;
			}

			if (mkdir(path_out, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
				printf("Can't create directory \"%s\".\n", in);
			}
			backup(f, path_in, path_out);
		}
		// Если файл
		else {
			// Если еще не копировался
			if (access(path_gz, F_OK)) {
				pid_t pid = fork();
				if (pid < 0) {
					printf("Fork failed.\n");
					return;
				} 
				else if (pid == 0) {
					if (cp(path_in, path_out) == 0) {
						execlp("gzip", "gzip", path_out, NULL);
					}
				}
				else {
					wait(NULL);
				}
			}
			// Копирование со сравнением
			else {
				pid_t pid = fork();
				if (pid < 0) {
					printf("Fork failed.\n");
					return;
				} 
				else if (pid == 0) {
					pid_t pid2 = fork();
					if (pid2 < 0) {
						printf("Fork failed.\n");
						return;
					} 
					else if (pid2 == 0) {
						execlp("gzip", "gzip", "-d", "-k", "-f", path_gz, NULL);
					} 
					else {
						wait(NULL);
						if (cmp(path_in, path_out) != 0) {
							if (cp(path_in, path_out) == 0){
								execlp("gzip", "gzip", "-f", path_out, NULL);
							}
						}
						else {
							remove(path_out);
						}
					}
				}
				else {
					wait(NULL);
				}
			}
		}
	} 
	while (entry = readdir(dir));
	closedir(dir);
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		printf("To run backup program you shoud type in and out directories.\n");
		return -1;
	}
	// Создаем выходную директорию
	mkdir(argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	// Запускаем резервное копирование
	backup(argv[0], argv[1], argv[2]);

	// Ждем завершения всех дочерних процессов
	while (wait(NULL) != -1) {};
	return 0;
}
