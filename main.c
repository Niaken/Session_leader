#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <termios.h>

void printProcessInfo(const char* message) {
    printf("PID %s: %d\n", message, getpid());
    printf("PGID %s: %d\n", message, getpgid(0));
    printf("SID %s: %d\n", message, getsid(0));

    pid_t tty_pgrp = tcgetpgrp(STDIN_FILENO);
    if (tty_pgrp == -1) {
        if (errno == ENOTTY) {
            printf("Процес не має керуючого терміналу %s.\n", message);
        } else {
            perror("tcgetpgrp");
        }
    } else {
        printf("PID керуючого терміналу %s: %d\n", message, tty_pgrp);
    }
}

int main() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Дочірній процес
        printf("Дочірній процес:\n");
        printProcessInfo("до setsid()");

        // Створюємо новий сеанс
        if (setsid() < 0) {
            perror("setsid");
            exit(EXIT_FAILURE);
        }

        printProcessInfo("після setsid()");

        // Перевіряємо, чи втратив процес керуючий термінал
        if (tcgetpgrp(STDIN_FILENO) == -1 && errno == ENOTTY) {
            printf("Дочірній процес втратив керуючий термінал.\n");
        } else {
            printf("Дочірній процес НЕ втратив керуючий термінал.\n");
        }

        // Завершення дочірнього процесу
        exit(EXIT_SUCCESS);
    } else {
        // Батьківський процес
        wait(NULL); // Очікуємо завершення дочірнього процесу
        printf("Батьківський процес завершив очікування.\n");
    }

    return 0;
}