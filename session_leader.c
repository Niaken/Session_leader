#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main() {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Команда для створення дочірнього процесу
    char commandLine[] = "child_process";

    // Створюємо дочірній процес
    if (!CreateProcess(NULL,            // ім'я модуля
        commandLine,                    // командний рядок
        NULL,                           // дескриптор процесу
        NULL,                           // дескриптор потоку
        FALSE,                          // успадкування дескрипторів
        CREATE_NEW_CONSOLE,             // прапор створення
        NULL,                           // нове значення змінних середовища
        NULL,                           // новий робочий каталог
        &si,                            // інформація про запуск
        &pi)                            // інформація про процес
    ) {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return EXIT_FAILURE;
    }

    // Дочірній процес
    if (pi.dwProcessId == GetCurrentProcessId()) {
        // Створення нового сеансу
        printf("Child process:\n");
        printf("PID: %lu\n", GetCurrentProcessId());

        // Затримка для демонстрації
        Sleep(10000); // спить 10 секунд

        return 0;
    }

    // Батьківський процес

    // Очікування завершення дочірнього процесу
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Отримання коду завершення процесу
    DWORD exitCode;
    if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
        printf("GetExitCodeProcess failed (%d).\n", GetLastError());
        return EXIT_FAILURE;
    }

    printf("Parent process:\n");
    printf("Child PID: %lu\n", pi.dwProcessId);
    printf("Child exited with code: %lu\n", exitCode);

    // Закриття дескрипторів процесу і потоку
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return EXIT_SUCCESS;
}