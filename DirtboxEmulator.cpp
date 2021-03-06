// Main entry point

#include "DirtboxEmulator.h"
#include "DirtboxKernel.h"
#include <stdio.h>

namespace Dirtbox
{
    CRITICAL_SECTION PrintLock;
}

BOOL WINAPI DllMain(
    HINSTANCE hInstDll, DWORD fdwReason, PVOID fImpLoad
)
{
    return TRUE;
}

VOID WINAPI Dirtbox::Initialize()
{
    // Initialize locks
    InitializeCriticalSection(&PrintLock);

    InitializeException();
    InitializeDummyKernel();
    InitializeDrives();
    InitializeThreading();
    __writefsword(NT_TIB_USER_POINTER, GetFS());
    InitializeGraphics();
    InitializeKernel();

    DebugPrint("Initialize: All initialized successfully, starting app.");
    SwapTibs();

    PMAIN_ROUTINE MainRoutine = (PMAIN_ROUTINE)(XBE_ENTRY_POINT ^ XBE_ENTRY_POINT_KEY);
    MainRoutine();

    SwapTibs();
    FreeTib();
}

VOID Dirtbox::InitializeKernel()
{
    // Initialize kernel globals
    ExEventObjectType.AllocateProcedure = &ExAllocatePoolWithTag;
    ExEventObjectType.FreeProcedure = &ExFreePool;
    ExEventObjectType.CloseProcedure = NULL;
    ExEventObjectType.DeleteProcedure = NULL;
    ExEventObjectType.ParseProcedure = NULL;
    ExEventObjectType.DefaultObject = NULL;
    ExEventObjectType.PoolTag = 0x76657645;

    HalDiskCachePartitionCount = 3;

    HalDiskModelNumber.Length = 0;
    HalDiskModelNumber.MaximumLength = 0;
    HalDiskModelNumber.Buffer = NULL;

    HalDiskSerialNumber.Length = 0;
    HalDiskSerialNumber.MaximumLength = 0;
    HalDiskSerialNumber.Buffer = NULL;

    IoFileObjectType.AllocateProcedure = &ExAllocatePoolWithTag;
    IoFileObjectType.FreeProcedure = &ExFreePool;
    IoFileObjectType.CloseProcedure = NULL; // TODO
    IoFileObjectType.DeleteProcedure = NULL; // TODO
    IoFileObjectType.ParseProcedure = NULL; // TODO
    IoFileObjectType.DefaultObject = (PVOID)0x38;
    IoFileObjectType.PoolTag = 0x656C6946;

    KeTickCount.LowPart = 0;
    KeTickCount.High1Time = 0;
    KeTickCount.High2Time = 0;

    LaunchDataPage = 0;

    PsThreadObjectType.AllocateProcedure = &ExAllocatePoolWithTag;
    PsThreadObjectType.FreeProcedure = &ExFreePool;
    PsThreadObjectType.CloseProcedure = NULL;
    PsThreadObjectType.DeleteProcedure = NULL;
    PsThreadObjectType.ParseProcedure = NULL;
    PsThreadObjectType.DefaultObject = NULL;
    PsThreadObjectType.PoolTag = 0x65726854;

    XboxHardwareInfo.Flags = 0x202;
    XboxHardwareInfo.GpuRevision = 0;
    XboxHardwareInfo.McpRevision = 0;

    memset(XboxHDKey, 0, 16);

    XboxKrnlVersion.Major = 1;
    XboxKrnlVersion.Minor = 0;
    XboxKrnlVersion.Build = 0x154F;
    XboxKrnlVersion.Qfe = 0x8001;

    HalBootSMCVideoMode = 7;

    IdexChannelObject = 0;

    // replace kernel import ordinals with pointer to our functions
    PDWORD KernelImageThunks = (PDWORD)(XBE_KERNEL_THUNK ^ XBE_KERNEL_THUNK_KEY);
    DWORD Thunk;
    for (int i = 0; Thunk = KernelImageThunks[i] & 0x7FFFFFFF, Thunk != NULL; i++)
    {
        switch (Thunk)
        {
        case 1:
            KernelImageThunks[i] = (DWORD)&AvGetSavedDataAddress;
            break;
        case 2:
            KernelImageThunks[i] = (DWORD)&AvSendTVEncoderOption;
            break;
        case 3:
            KernelImageThunks[i] = (DWORD)&AvSetDisplayMode;
            break;
        case 4:
            KernelImageThunks[i] = (DWORD)&AvSetSavedDataAddress;
            break;
        case 8:
            KernelImageThunks[i] = (DWORD)&DbgPrint;
            break;
        case 14:
            KernelImageThunks[i] = (DWORD)&ExAllocatePool;
            break;
        case 15:
            KernelImageThunks[i] = (DWORD)&ExAllocatePoolWithTag;
            break;
        case 16:
            KernelImageThunks[i] = (DWORD)&ExEventObjectType;
            break;
        case 17:
            KernelImageThunks[i] = (DWORD)&ExFreePool;
            break;
        case 23:
            KernelImageThunks[i] = (DWORD)&ExQueryPoolBlockSize;
            break;
        case 24:
            KernelImageThunks[i] = (DWORD)&ExQueryNonVolatileSetting;
            break;
        case 40:
            KernelImageThunks[i] = (DWORD)&HalDiskCachePartitionCount;
            break;
        case 41:
            KernelImageThunks[i] = (DWORD)&HalDiskModelNumber;
            break;
        case 42:
            KernelImageThunks[i] = (DWORD)&HalDiskSerialNumber;
            break;
        case 44:
            KernelImageThunks[i] = (DWORD)&HalGetInterruptVector;
            break;
        case 46:
            KernelImageThunks[i] = (DWORD)&HalReadWritePCISpace;
            break;
        case 47:
            KernelImageThunks[i] = (DWORD)&HalRegisterShutdownNotification;
            break;
        case 49:
            KernelImageThunks[i] = (DWORD)&HalReturnToFirmware;
            break;
        case 62:
            KernelImageThunks[i] = (DWORD)&IoBuildSynchronousFsdRequest;
            break;
        case 65:
            KernelImageThunks[i] = (DWORD)&IoCreateDevice;
            break;
        case 67:
            KernelImageThunks[i] = (DWORD)&IoCreateSymbolicLink;
            break;
        case 69:
            KernelImageThunks[i] = (DWORD)&IoDeleteSymbolicLink;
            break;
        case 71:
            KernelImageThunks[i] = (DWORD)&IoFileObjectType;
            break;
        case 74:
            KernelImageThunks[i] = (DWORD)&IoInvalidDeviceRequest;
            break;
        case 81:
            KernelImageThunks[i] = (DWORD)&IoStartNextPacket;
            break;
        case 83:
            KernelImageThunks[i] = (DWORD)&IoStartPacket;
            break;
        case 84:
            KernelImageThunks[i] = (DWORD)&IoSynchronousDeviceIoControlRequest;
            break;
        case 85:
            KernelImageThunks[i] = (DWORD)&IoSynchronousFsdRequest;
            break;
        case 86:
            KernelImageThunks[i] = (DWORD)&IofCallDriver;
            break;
        case 87:
            KernelImageThunks[i] = (DWORD)&IofCompleteRequest;
            break;
        case 95:
            KernelImageThunks[i] = (DWORD)&KeBugCheck;
            break;
        case 97:
            KernelImageThunks[i] = (DWORD)&KeCancelTimer;
            break;
        case 98:
            KernelImageThunks[i] = (DWORD)&KeConnectInterrupt;
            break;
        case 99:
            KernelImageThunks[i] = (DWORD)&KeDelayExecutionThread;
            break;
        case 100:
            KernelImageThunks[i] = (DWORD)&KeDisconnectInterrupt;
            break;
        case 107:
            KernelImageThunks[i] = (DWORD)&KeInitializeDpc;
            break;
        case 108:
            KernelImageThunks[i] = (DWORD)&KeInitializeEvent;
            break;
        case 109:
            KernelImageThunks[i] = (DWORD)&KeInitializeInterrupt;
            break;
        case 113:
            KernelImageThunks[i] = (DWORD)&KeInitializeTimerEx;
            break;
        case 119:
            KernelImageThunks[i] = (DWORD)&KeInsertQueueDpc;
            break;
        case 124:
            KernelImageThunks[i] = (DWORD)&KeQueryBasePriorityThread;
            break;
        case 125:
            KernelImageThunks[i] = (DWORD)&KeQueryInterruptTime;
            break;
        case 128:
            KernelImageThunks[i] = (DWORD)&KeQuerySystemTime;
            break;
        case 129:
            KernelImageThunks[i] = (DWORD)&KeRaiseIrqlToDpcLevel;
            break;
        case 137:
            KernelImageThunks[i] = (DWORD)&KeRemoveQueueDpc;
            break;
        case 139:
            KernelImageThunks[i] = (DWORD)&KeRestoreFloatingPointState;
            break;
        case 142:
            KernelImageThunks[i] = (DWORD)&KeSaveFloatingPointState;
            break;
        case 143:
            KernelImageThunks[i] = (DWORD)&KeSetBasePriorityThread;
            break;
        case 144:
            KernelImageThunks[i] = (DWORD)&KeSetDisableBoostThread;
            break;
        case 145:
            KernelImageThunks[i] = (DWORD)&KeSetEvent;
            break;
        case 149:
            KernelImageThunks[i] = (DWORD)&KeSetTimer;
            break;
        case 150:
            KernelImageThunks[i] = (DWORD)&KeSetTimerEx;
            break;
        case 151:
            KernelImageThunks[i] = (DWORD)&KeStallExecutionProcessor;
            break;
        case 153:
            KernelImageThunks[i] = (DWORD)&KeSynchronizeExecution;
            break;
        case 156:
            KernelImageThunks[i] = (DWORD)&KeTickCount;
            break;
        case 158:
            KernelImageThunks[i] = (DWORD)&KeWaitForMultipleObjects;
            break;
        case 159:
            KernelImageThunks[i] = (DWORD)&KeWaitForSingleObject;
            break;
        case 160:
            KernelImageThunks[i] = (DWORD)&KfRaiseIrql;
            break;
        case 161:
            KernelImageThunks[i] = (DWORD)&KfLowerIrql;
            break;
        case 164:
            KernelImageThunks[i] = (DWORD)&LaunchDataPage;
            break;
        case 165:
            KernelImageThunks[i] = (DWORD)&MmAllocateContiguousMemory;
            break;
        case 166:
            KernelImageThunks[i] = (DWORD)&MmAllocateContiguousMemoryEx;
            break;
        case 168:
            KernelImageThunks[i] = (DWORD)&MmClaimGpuInstanceMemory;
            break;
        case 171:
            KernelImageThunks[i] = (DWORD)&MmFreeContiguousMemory;
            break;
        case 173:
            KernelImageThunks[i] = (DWORD)&MmGetPhysicalAddress;
            break;
        case 175:
            KernelImageThunks[i] = (DWORD)&MmLockUnlockBufferPages;
            break;
        case 176:
            KernelImageThunks[i] = (DWORD)&MmLockUnlockPhysicalPage;
            break;
        case 178:
            KernelImageThunks[i] = (DWORD)&MmPersistContiguousMemory;
            break;
        case 179:
            KernelImageThunks[i] = (DWORD)&MmQueryAddressProtect;
            break;
        case 180:
            KernelImageThunks[i] = (DWORD)&MmQueryAllocationSize;
            break;
        case 181:
            KernelImageThunks[i] = (DWORD)&MmQueryStatistics;
            break;
        case 182:
            KernelImageThunks[i] = (DWORD)&MmSetAddressProtect;
            break;
        case 184:
            KernelImageThunks[i] = (DWORD)&NtAllocateVirtualMemory;
            break;
        case 187:
            KernelImageThunks[i] = (DWORD)&NtClose;
            break;
        case 189:
            KernelImageThunks[i] = (DWORD)&NtCreateEvent;
            break;
        case 190:
            KernelImageThunks[i] = (DWORD)&NtCreateFile;
            break;
        case 193:
            KernelImageThunks[i] = (DWORD)&NtCreateSemaphore;
            break;
        case 196:
            KernelImageThunks[i] = (DWORD)&NtDeviceIoControlFile;
            break;
        case 198:
            KernelImageThunks[i] = (DWORD)&NtFlushBuffersFile;
            break;
        case 199:
            KernelImageThunks[i] = (DWORD)&NtFreeVirtualMemory;
            break;
        case 200:
            KernelImageThunks[i] = (DWORD)&NtFsControlFile;
            break;
        case 202:
            KernelImageThunks[i] = (DWORD)&NtOpenFile;
            break;
        case 203:
            KernelImageThunks[i] = (DWORD)&NtOpenSymbolicLinkObject;
            break;
        case 205:
            KernelImageThunks[i] = (DWORD)&NtPulseEvent;
            break;
        case 207:
            KernelImageThunks[i] = (DWORD)&NtQueryDirectoryFile;
            break;
        case 211:
            KernelImageThunks[i] = (DWORD)&NtQueryInformationFile;
            break;
        case 215:
            KernelImageThunks[i] = (DWORD)&NtQuerySymbolicLinkObject;
            break;
        case 217:
            KernelImageThunks[i] = (DWORD)&NtQueryVirtualMemory;
            break;
        case 218:
            KernelImageThunks[i] = (DWORD)&NtQueryVolumeInformationFile;
            break;
        case 219:
            KernelImageThunks[i] = (DWORD)&NtReadFile;
            break;
        case 222:
            KernelImageThunks[i] = (DWORD)&NtReleaseSemaphore;
            break;
        case 224:
            KernelImageThunks[i] = (DWORD)&NtResumeThread;
            break;
        case 225:
            KernelImageThunks[i] = (DWORD)&NtSetEvent;
            break;
        case 226:
            KernelImageThunks[i] = (DWORD)&NtSetInformationFile;
            break;
        case 231:
            KernelImageThunks[i] = (DWORD)&NtSuspendThread;
            break;
        case 233:
            KernelImageThunks[i] = (DWORD)&NtWaitForSingleObject;
            break;
        case 234:
            KernelImageThunks[i] = (DWORD)&NtWaitForSingleObjectEx;
            break;
        case 236:
            KernelImageThunks[i] = (DWORD)&NtWriteFile;
            break;
        case 238:
            KernelImageThunks[i] = (DWORD)&NtYieldExecution;
            break;
        case 246:
            KernelImageThunks[i] = (DWORD)&ObReferenceObjectByHandle;
            break;
        case 250:
            KernelImageThunks[i] = (DWORD)&ObfDereferenceObject;
            break;
        case 255:
            KernelImageThunks[i] = (DWORD)&PsCreateSystemThreadEx;
            break;
        case 258:
            KernelImageThunks[i] = (DWORD)&PsTerminateSystemThread;
            break;
        case 259:
            KernelImageThunks[i] = (DWORD)&PsThreadObjectType;
            break;
        case 269:
            KernelImageThunks[i] = (DWORD)&RtlCompareMemoryUlong;
            break;
        case 277:
            KernelImageThunks[i] = (DWORD)&RtlEnterCriticalSection;
            break;
        case 279:
            KernelImageThunks[i] = (DWORD)&RtlEqualString;
            break;
        case 289:
            KernelImageThunks[i] = (DWORD)&RtlInitAnsiString;
            break;
        case 291:
            KernelImageThunks[i] = (DWORD)&RtlInitializeCriticalSection;
            break;
        case 294:
            KernelImageThunks[i] = (DWORD)&RtlLeaveCriticalSection;
            break;
        case 301:
            KernelImageThunks[i] = (DWORD)&RtlNtStatusToDosError;
            break;
        case 302:
            KernelImageThunks[i] = (DWORD)&RtlRaiseException;
            break;
        case 304:
            KernelImageThunks[i] = (DWORD)&RtlTimeFieldsToTime;
            break;
        case 305:
            KernelImageThunks[i] = (DWORD)&RtlTimeToTimeFields;
            break;
        case 306:
            KernelImageThunks[i] = (DWORD)&RtlTryEnterCriticalSection;
            break;
        case 312:
            KernelImageThunks[i] = (DWORD)&RtlUnwind;
            break;
        case 322:
            KernelImageThunks[i] = (DWORD)&XboxHardwareInfo;
            break;
        case 323:
            KernelImageThunks[i] = (DWORD)XboxHDKey; // Array is already pointer
            break;
        case 324:
            KernelImageThunks[i] = (DWORD)&XboxKrnlVersion;
            break;
        case 326:
            KernelImageThunks[i] = (DWORD)&XeImageFileName;
            break;
        case 327:
            KernelImageThunks[i] = (DWORD)&XeLoadSection;
            break;
        case 328:
            KernelImageThunks[i] = (DWORD)&XeUnloadSection;
            break;
        case 335:
            KernelImageThunks[i] = (DWORD)&XcSHAInit;
            break;
        case 336:
            KernelImageThunks[i] = (DWORD)&XcSHAUpdate;
            break;
        case 337:
            KernelImageThunks[i] = (DWORD)&XcSHAFinal;
            break;
        case 340:
            KernelImageThunks[i] = (DWORD)&XcHMAC;
            break;
        case 356:
            KernelImageThunks[i] = (DWORD)&HalBootSMCVideoMode;
            break;
        case 357:
            KernelImageThunks[i] = (DWORD)&IdexChannelObject;
            break;
        case 360:
            KernelImageThunks[i] = (DWORD)&HalInitiateShutdown;
            break;
        default:
            FatalPrint("Initialize: Unimplemented kernel function %i.", Thunk);
        }
    }
}

// NOTE: Do we need to call fflush(stdout) here?
VOID Dirtbox::DebugPrint(PSTR Format, ...)
{
    EnterCriticalSection(&PrintLock);

    va_list Args;
    va_start(Args, Format);
    vprintf(Format, Args);
    va_end(Args);
    putchar('\n');

    fflush(stdout);

    LeaveCriticalSection(&PrintLock);
}

VOID Dirtbox::FatalPrint(PSTR Format, ...)
{
    EnterCriticalSection(&PrintLock);

    printf("Error: ");
    va_list Args;
    va_start(Args, Format);
    vprintf(Format, Args);
    va_end(Args);
    putchar('\n');

    fflush(stdout);

    LeaveCriticalSection(&PrintLock);

    exit(1);
}
