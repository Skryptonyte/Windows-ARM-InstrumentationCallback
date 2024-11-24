
#include "instrumentation.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <processthreadsapi.h>
#include <libloaderapi.h>
#include <DbgHelp.h>

extern "C" void instrumentationCallbackOuter(void);

extern "C" void instrumentationCallback(uint64_t PC) {
    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    PSYMBOL_INFO pSymbol = (PSYMBOL_INFO) buffer;
    DWORD64 displacement = 0;

    /* instrumentationCallbackDisabled at offset 0x2ec of TEB */
    uint8_t *instrumentationCallbackDisabled = ((uint8_t *) NtCurrentTeb() + 0x2ec);

    /* Avoid infinite recursion */
    if (*instrumentationCallbackDisabled) {
        return;
    }

    *instrumentationCallbackDisabled = 1;
    
    pSymbol->MaxNameLen = MAX_SYM_NAME;
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    if (SymFromAddr(GetCurrentProcess(), PC, &displacement, pSymbol)) {
        printf_s("System call '%s' @ %016x\n", pSymbol->Name, pSymbol->Address);
    }
    else {
        printf_s("Unknown system call returning @ %016x\n", PC);
    }

    *instrumentationCallbackDisabled = 0;
}

int main()
{
    InstrumentationCallbackInfo_t InstrumentationInfo;
    HMODULE ntdll = LoadLibrary(L"ntdll.dll");
    NTSTATUS (*NtSetInformationProcess)(HANDLE, ULONG, PVOID, ULONG)
        = (NTSTATUS(*)(HANDLE, ULONG, PVOID, ULONG)) GetProcAddress(ntdll, "NtSetInformationProcess");
    FILE* file;


    /* Need to run this after LoadLibrary or SymFromAddr fails later */
    SymInitialize(GetCurrentProcess(), NULL, true);

    printf_s("Program begin!\n");

    InstrumentationInfo.version = 0;
    InstrumentationInfo.reserved = 0;
    InstrumentationInfo.callback = instrumentationCallbackOuter;

    NtSetInformationProcess(GetCurrentProcess(), 40,
                            &InstrumentationInfo,
                            sizeof(InstrumentationInfo));

    printf_s("Instrumentation Callback hooked!\n");

    /* Do some file ops to exercise some NT calls */
    fopen_s(&file, "testfile", "wb+");
    
    if (file) {
        char* buf = (char*)malloc(0x100);

        fprintf(file, "Testing various system calls :D");

        fclose(file);

        fopen_s(&file, "testfile", "rb");

        fseek(file, 0, SEEK_SET);
        
        memset(buf, 0, 0x100);
        fgets(buf, 256, file);

        printf_s("File content: %s\n", buf);

        free(buf);

        fclose(file);
    }
    
    printf_s("Program end!\n");
}

