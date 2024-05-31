#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

// Визначаємо типи та структури, які будемо використовувати
typedef LONG NTSTATUS;
typedef NTSTATUS(NTAPI* pNtQueryInformationProcess)(HANDLE, UINT, PVOID, ULONG, PULONG);

#define ProcessBasicInformation 0

typedef struct {
    LONG ExitStatus;
    PVOID PebBaseAddress;
    ULONG_PTR AffinityMask;
    LONG BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;

DWORD GetParentProcessId() {
    DWORD parentPid = -1;
    HMODULE hNtDll = LoadLibraryA("ntdll.dll");
    if (hNtDll) {
        pNtQueryInformationProcess NtQueryInformationProcess = (pNtQueryInformationProcess)GetProcAddress(hNtDll, "NtQueryInformationProcess");
        if (NtQueryInformationProcess) {
            PROCESS_BASIC_INFORMATION pbi;
            ULONG len;
            NTSTATUS status = NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &pbi, sizeof(pbi), &len);
            if (status == 0) {
                parentPid = (DWORD)pbi.InheritedFromUniqueProcessId;
            }
        }
        FreeLibrary(hNtDll);
    }
    return parentPid;
}

void printSessionIdDescription(DWORD sessionId) {
    switch (sessionId) {
    case 0:
        printf("Session ID 0: Сеанс консолі. Використовується для локальних входів.\n");
        break;
    case 1:
        printf("Session ID 1: Використовується для служб термінальних серверів.\n");
        break;
    default:
        printf("Session ID %lu: Користувацький сеанс.\n", sessionId);
        break;
    }
}

int main(int argc, char* argv[]) {
    // Встановлення кодування 1251 для консолі
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    if (argc > 1 && strcmp(argv[1], "child") == 0) {
        // Дочірній процес
        DWORD parentPid = GetParentProcessId();
        printf("Дочірній процес:\n");
        printf("PID: %lu\n", (unsigned long)GetCurrentProcessId());
        printf("PID батьківського процесу: %lu\n", (unsigned long)parentPid);

        // Перевірка, чи є процес лідером сеансу (у Windows немає чіткого поняття лідера сеансу)
        DWORD sessionId;
        if (ProcessIdToSessionId(GetCurrentProcessId(), &sessionId)) {
            printf("Session ID дочірнього процесу: %lu\n", (unsigned long)sessionId);
            printSessionIdDescription(sessionId);
        }
        else {
            printf("Не вдалося отримати Session ID (%d).\n", GetLastError());
        }

        // Перевірка, чи втратив процес управляючий термінал
        if (GetStdHandle(STD_INPUT_HANDLE) == NULL ||
            GetStdHandle(STD_OUTPUT_HANDLE) == NULL ||
            GetStdHandle(STD_ERROR_HANDLE) == NULL) {
            printf("Дочірній процес втратив управляючий термінал\n");
        }
        else {
            printf("Дочірній процес має управляючий термінал\n");
        }

        // Затримка для демонстрації
        Sleep(10000); // Спить 10 секунд

        return 0;
    }

    // Батьківський процес
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Команда для створення дочірнього процесу
    wchar_t commandLine[] = L"session_leader child";

    // Створюємо дочірній процес
    if (!CreateProcessW(NULL,            // ім'я модуля
        commandLine,     // командний рядок
        NULL,            // дескриптор процесу
        NULL,            // дескриптор потоку
        FALSE,           // успадкування дескрипторів
        CREATE_NEW_CONSOLE, // прапор створення
        NULL,            // нове значення змінних середовища
        NULL,            // новий робочий каталог
        &si,             // інформація про запуск
        &pi)             // інформація про процес
        ) {
        printf("CreateProcess не вдалося (%d).\n", GetLastError());
        return EXIT_FAILURE;
    }

    // Очікування завершення дочірнього процесу
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Отримання коду завершення процесу
    DWORD exitCode;
    if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
        printf("GetExitCodeProcess не вдалося (%d).\n", GetLastError());
        return EXIT_FAILURE;
    }

    printf("Батьківський процес:\n");
    printf("PID: %lu\n", (unsigned long)GetCurrentProcessId());
    printf("PID дочірнього процесу: %lu\n", (unsigned long)pi.dwProcessId);
    printf("Дочірній процес завершився з кодом: %lu\n", (unsigned long)exitCode);

    // Отримання Session ID для батьківського процесу
    DWORD parentSessionId;
    if (ProcessIdToSessionId(GetCurrentProcessId(), &parentSessionId)) {
        printf("Session ID батьківського процесу: %lu\n", (unsigned long)parentSessionId);
        printSessionIdDescription(parentSessionId);
    }
    else {
        printf("Не вдалося отримати Session ID батьківського процесу (%d).\n", GetLastError());
    }

    // Закриття дескрипторів процесу і потоку
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return EXIT_SUCCESS;
}