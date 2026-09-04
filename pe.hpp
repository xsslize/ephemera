#pragma once

#include <windows.h>

#include "obfuscation/immediate.hpp"

#ifndef _DEBUG
#define _FORCEINLINE __forceinline
#else
#define _FORCEINLINE 
#endif

struct ClientIdT
{
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
};

struct UnicodeStringT
{
    uint16_t Length;
    uint16_t MaximumLength;
    wchar_t* Buffer;
};

struct ObjectAttributesT
{
    uint32_t Length;
    HANDLE RootDirectory;
    UnicodeStringT* ObjectName;
    uint32_t Attributes;
    void* SecurityDescriptor;
    void* SecurityQualityOfService;
};

struct ListEntryT
{
    ListEntryT* Flink;
    ListEntryT* Blink;
};

struct PebLdrDataT
{
    uint32_t Length;
    uint32_t Initialized;
    uint64_t SsHandle;
    ListEntryT InLoadOrderModuleList;
    ListEntryT InMemoryOrderModuleList;
    ListEntryT InInitializationOrderModuleList;
    uint64_t EntryInProgress;
    uint64_t ShutdownInProgress;
    uint64_t ShutdownThreadId;
};

struct PebT
{
    uint8_t Reserved1[ 2 ];
    uint8_t BeingDebugged;
    uint8_t Reserved2[ 1 ];
    void* Reserved3[ 2 ];
    PebLdrDataT* Ldr;
    void* ProcessParameters;
    void* SubSystemData;
    void* ProcessHeap;

    _FORCEINLINE static PebT* Current( )
    {
        PebT* Peb = nullptr;
        uint64_t Offset = ImmediateVal( 0x60 );
        asm volatile( "movq %%gs:(%[off]), %[val]" : [val] "=r" ( Peb ) : [off] "r" ( Offset ) );
        return Peb;
    }
};

struct LdrDataTableEntryT
{
    ListEntryT InLoadOrderLinks;
    ListEntryT InMemoryOrderLinks;
    ListEntryT InInitializationOrderLinks;
    uintptr_t DllBase;
    uintptr_t EntryPoint;
    uint64_t SizeOfImage;
    UnicodeStringT FullDllName;
    UnicodeStringT BaseDllName;
};

struct SystemBasicInformationT
{
    uint32_t Reserved;
    uint32_t TimerResolution;
    uint32_t PageSize;
    uint32_t NumberOfPhysicalPages;
    uint32_t LowestPhysicalPageNumber;
    uint32_t HighestPhysicalPageNumber;
    uint32_t AllocationGranularity;
    uint32_t MinimumUsermodeAddress;
    uint32_t MaximumUsermodeAddress;
    uint64_t ActiveProcessorsAffinityMask;
    int8_t NumberOfProcessors;
    uint8_t Reserved1[ 8 ];
};

struct SystemTimeT
{
    uint32_t LowPart = 0;
    int32_t High1Time = 0;
    int32_t High2Time = 0;
};