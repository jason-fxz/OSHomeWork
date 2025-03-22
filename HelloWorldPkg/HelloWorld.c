#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Guid/Acpi.h>
#include <IndustryStandard/Acpi.h>


VOID
PrintGuid(
  IN EFI_GUID *Guid
)
{
  Print(L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
    Guid->Data1,
    Guid->Data2,
    Guid->Data3,
    Guid->Data4[0],
    Guid->Data4[1],
    Guid->Data4[2],
    Guid->Data4[3],
    Guid->Data4[4],
    Guid->Data4[5],
    Guid->Data4[6],
    Guid->Data4[7]);
}

UINT32
TS(
  IN CHAR8 *Signature
)
{
  UINT32 Result = 0;
  for (UINTN i = 0; i < 4; i++)
  {
    Result |= (UINT32)Signature[i] << (i * 8);
  }
  return Result;
}


VOID
PrintAcpiHeader(
  IN EFI_ACPI_DESCRIPTION_HEADER *Header
)
{
  Print(L"---------- %c%c%c%c HEADER ----------\n", Header->Signature & 0xff, (Header->Signature >> 8) & 0xff, (Header->Signature >> 16) & 0xff, (Header->Signature >> 24) & 0xff);
  Print(L"Address      : 0x%p\n", Header);
  Print(L"Length       : %d\n", Header->Length);
  Print(L"Revision     : %d\n", Header->Revision);
  Print(L"Checksum     : %d\n", Header->Checksum);
  Print(L"OEM ID       : ");
  for (UINTN i = 0; i < 6; i++) {
    Print(L"%c", Header->OemId[i]);
  }
  Print(L"\n");
  Print(L"OEM Table ID : %d\n", Header->OemTableId);
  Print(L"OEM Revision : %d\n", Header->OemRevision);
  Print(L"Creator ID   : %d\n", Header->CreatorId);
  Print(L"Creator Rev  : %d\n", Header->CreatorRevision);
  Print(L"\n");
}


VOID
PrintAllACPITables(
  VOID
)
{
  UINTN i, j;
  EFI_CONFIGURATION_TABLE *configTab = NULL;

  EFI_ACPI_6_3_ROOT_SYSTEM_DESCRIPTION_POINTER  *Root;

  
  
  // PrintGuid(&gEfiAcpiTableGuid);
  // PrintGuid(&gEfiAcpi20TableGuid);
  Print(L"Print ACPI\n");

  configTab = gST->ConfigurationTable;
  //遍历系统配置表
  for (i = 0; i < gST->NumberOfTableEntries; i++)
  {
    if ((CompareGuid(&configTab->VendorGuid, &gEfiAcpiTableGuid) == TRUE) ||
      (CompareGuid(&configTab->VendorGuid, &gEfiAcpi20TableGuid) == TRUE))
    {	
      Print(L"Found ACPI table\n");
      Print(L"Address : 0x%p\n\n",configTab);

      Root = configTab->VendorTable;

      // Print RSDP
      Print(L"---------- RSDP TABLE ----------\n");
      Print(L"Address      : 0x%p\n", Root);
      Print(L"Length       : %d\n", Root->Length);
      Print(L"Signature    : ");
      for (j = 0; j < 8; j++) {
        Print(L"%c", (Root->Signature >> (j * 8)) & 0xff);
      }
      Print(L"\n");
      Print(L"OEM ID       : ");
      for (j = 0; j < 6; j++) {
        Print(L"%c", Root->OemId[j]);
      }
      Print(L"\n");
      Print(L"Checksum     : %d\n", Root->Checksum);

      Print(L"RSDT address : 0x%p\n", Root->RsdtAddress);
      Print(L"XSDT address : 0x%p\n", Root->XsdtAddress);

      Print(L"\n");
      
      // Print XSDT (Get XSDT address from RSDP) 
      EFI_ACPI_DESCRIPTION_HEADER * XSDT = (EFI_ACPI_DESCRIPTION_HEADER *)Root->XsdtAddress;

      UINTN N = (XSDT->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);

      Print(L"---------- XSDT TABLE ----------\n");
      Print(L"Address      : 0x%p\n", XSDT);
      Print(L"Length       : %d\n", XSDT->Length);
      Print(L"Signature    : ");
      for (j = 0; j < 4; j++) {
        Print(L"%c", (XSDT->Signature >> (j * 8)) & 0xff);
      }
      Print(L"\n");
      Print(L"OEM ID       : ");
      for (j = 0; j < 6; j++) {
        Print(L"%c", XSDT->OemId[j]);
      }
      Print(L"\n");
      Print(L"Checksum     : %d\n", XSDT->Checksum);
      Print(L"\n");

      UINT64 *EntryPtr = (UINT64 *)(XSDT + 1);
      for (j = 0; j < N; j++, EntryPtr++)
      {
        EFI_ACPI_DESCRIPTION_HEADER *Entry = (EFI_ACPI_DESCRIPTION_HEADER *)(*EntryPtr);
        if (Entry != NULL)
        {
          PrintAcpiHeader(Entry);
          if (Entry->Signature == EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
            // FADT (Signature: FACP) has sub-tables
            EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *FADT = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *)Entry;
            EFI_ACPI_DESCRIPTION_HEADER *DSDT = NULL;
            EFI_ACPI_DESCRIPTION_HEADER *FACS = NULL;
            
            if (FADT->Dsdt != 0) {
              DSDT = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)FADT->Dsdt;
              PrintAcpiHeader(DSDT);
            }
            if (FADT->FirmwareCtrl != 0) {
              FACS = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)FADT->FirmwareCtrl;
              PrintAcpiHeader(FACS);
            }
          }
        }
      }


    }
    configTab++;
  }
}


VOID
ModifyACPITable
(
  IN EFI_ACPI_DESCRIPTION_HEADER* entry,
  IN EFI_ACPI_DESCRIPTION_HEADER* NewTable
)
{
  /*
  typedef struct {
    UINT32    Signature;
    UINT32    Length;
    UINT8     Revision;
    UINT8     Checksum;
    UINT8     OemId[6];
    UINT64    OemTableId;
    UINT32    OemRevision;
    UINT32    CreatorId;
    UINT32    CreatorRevision;
  } EFI_ACPI_DESCRIPTION_HEADER;
  */
  CopyMem(entry, NewTable, NewTable->Length);
  entry->Checksum = 0;
  for (UINTN i = 0; i < entry->Length; i++)
  {
    entry->Checksum ^= ((UINT8*)entry)[i];
  }
}


EFI_ACPI_DESCRIPTION_HEADER*
GetACPITable
(
  IN UINT32  Signature
)
{
  UINTN i, j;
  EFI_CONFIGURATION_TABLE *configTab = NULL;

  EFI_ACPI_6_3_ROOT_SYSTEM_DESCRIPTION_POINTER  *Root;


  configTab = gST->ConfigurationTable;
  //遍历系统配置表
  for (i = 0; i < gST->NumberOfTableEntries; i++)
  {
    if ((CompareGuid(&configTab->VendorGuid, &gEfiAcpiTableGuid) == TRUE) ||
      (CompareGuid(&configTab->VendorGuid, &gEfiAcpi20TableGuid) == TRUE))
    {	
      Root = configTab->VendorTable;      
      // Print XSDT (Get XSDT address from RSDP) 
      EFI_ACPI_DESCRIPTION_HEADER * XSDT = (EFI_ACPI_DESCRIPTION_HEADER *)Root->XsdtAddress;

      UINTN N = (XSDT->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);

      UINT64 *EntryPtr = (UINT64 *)(XSDT + 1);
      for (j = 0; j < N; j++, EntryPtr++)
      {
        EFI_ACPI_DESCRIPTION_HEADER *Entry = (EFI_ACPI_DESCRIPTION_HEADER *)(*EntryPtr);
        if (Entry != NULL)
        {
          if (Entry->Signature == Signature)
          {
            return Entry;
          }
          
          if (Entry->Signature == EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
            // FADT (Signature: FACP) has sub-tables
            EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *FADT = (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE *)Entry;
            EFI_ACPI_DESCRIPTION_HEADER *DSDT = NULL;
            EFI_ACPI_DESCRIPTION_HEADER *FACS = NULL;
            
            if (FADT->Dsdt != 0) {
              DSDT = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)FADT->Dsdt;
              if (DSDT->Signature == Signature)
              {
                return DSDT;
              }
            }
            if (FADT->FirmwareCtrl != 0) {
              FACS = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN)FADT->FirmwareCtrl;
              if (FACS->Signature == Signature)
              {
                return FACS;
              }
            }
          }
        }
      }


    }
    configTab++;
  }
  return NULL;
}


VOID
ChangeACPITable
(
  IN UINT32  Signature,
  IN EFI_ACPI_DESCRIPTION_HEADER* NewTable
)
{
  EFI_ACPI_DESCRIPTION_HEADER* entry = GetACPITable(Signature);
  if (entry != NULL)
  {
    ModifyACPITable(entry, NewTable);
  }
}


EFI_STATUS
EFIAPI
UefiMain(
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
)
{
  PrintAllACPITables();
  // Modify APIC
  EFI_ACPI_DESCRIPTION_HEADER* entry = GetACPITable(EFI_ACPI_6_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE);
  if (entry != NULL)
  {
    EFI_ACPI_DESCRIPTION_HEADER NewTable = {
      EFI_ACPI_6_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE,
      sizeof(EFI_ACPI_DESCRIPTION_HEADER),
      0,
      0,
      {'M','Y','O','E','M','I'},
      0,
      0,
      0,
      0
    };
    ModifyACPITable(entry, &NewTable);
  }
  PrintAcpiHeader(entry);
  return EFI_SUCCESS;
}