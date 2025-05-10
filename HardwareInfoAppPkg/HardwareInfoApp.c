#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <IndustryStandard/Acpi.h>
#include <Protocol/AcpiTable.h>

// 自定义 ACPI 表的签名
#define ZHWI_SIGNATURE  SIGNATURE_32('Z', 'H', 'W', 'I')  // Zhwi = 自定义硬件信息

// 简化的 ACPI 表结构 - 移除了动态函数指针
#pragma pack(1)
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;     // 标准 ACPI 表头
  UINT32                      Revision;    // 我们表格的版本
  UINT32                      FanSpeed;    // 风扇转速 (RPM)
  UINT32                      CpuTemp;     // CPU 温度 (0.1 摄氏度)
  UINT32                      SystemLoad;  // 系统负载 (百分比)
  UINT64                      Timestamp;   // 更新时间戳（秒）
} ZHWI_TABLE;
#pragma pack()

// 全局表实例
ZHWI_TABLE mZhwiTable = {
  // 标准 ACPI 头初始化
  {
    ZHWI_SIGNATURE,                  // Signature
    sizeof (ZHWI_TABLE),             // Length
    1,                               // Revision
    0,                               // Checksum - 将在安装时计算
    {'J', 'A', 'S', 'O', 'N'},       // OEMID
    114514,                          // OEM Table ID
    1,                               // OEM Revision
    19260817,                        // Creator ID
    1                                // Creator Revision
  },
  // 其他字段初始值
  1,                                 // 表版本
  2500,                              // 风扇转速 - 静态值
  420,                               // CPU温度 - 静态值 (42.0°C)
  30,                                // 系统负载 - 静态值 (30%)
  1650000000                         // 时间戳 - 静态值
};

/**
  计算 ACPI 表的校验和。
  表的校验和字节应让整个表的和为零。

  @param[in]      Buffer          指向 ACPI 表的指针
  @param[in]      Size            表的大小
  @param[in,out]  ChecksumOffset  校验和字段在表中的偏移
**/
VOID
AcpiCalculateChecksum (
  IN     UINT8  *Buffer,
  IN     UINTN   Size,
  IN OUT UINTN   ChecksumOffset
  )
{
  UINT8 Sum;
  UINTN Index;

  // 将校验和字段清零
  Buffer[ChecksumOffset] = 0;

  // 计算整个表的校验和
  Sum = 0;
  for (Index = 0; Index < Size; Index++) {
    Sum = (UINT8)(Sum + Buffer[Index]);
  }

  // 设置校验和字段，使总和为零
  Buffer[ChecksumOffset] = (UINT8)(0x100 - Sum);
}

/**
  安装 ACPI 表。
  
  @retval EFI_SUCCESS    表格成功安装
  @retval 其他           安装表格时发生错误
**/
EFI_STATUS
InstallAcpiTable (
  VOID
  )
{
  EFI_STATUS              Status;
  EFI_ACPI_TABLE_PROTOCOL *AcpiTable;
  UINTN                   TableKey = 0;
  
  // 查找 ACPI 表协议
  Status = gBS->LocateProtocol (
                  &gEfiAcpiTableProtocolGuid,
                  NULL,
                  (VOID **) &AcpiTable
                  );
                  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "未找到 ACPI 表协议: %r\n", Status));
    return Status;
  }
  
  // 计算表的校验和
  AcpiCalculateChecksum (
    (UINT8 *) &mZhwiTable,
    mZhwiTable.Header.Length,
    OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum)
    );
  
  // 安装 ACPI 表
  Status = AcpiTable->InstallAcpiTable (
                        AcpiTable,
                        &mZhwiTable,
                        mZhwiTable.Header.Length,
                        &TableKey
                        );
                        
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "安装 ACPI 表失败: %r\n", Status));
    return Status;
  }
  
  DEBUG ((DEBUG_INFO, "ACPI ZHWI 表已成功安装，风扇转速: %d RPM\n", mZhwiTable.FanSpeed));
  return EFI_SUCCESS;
}

/**
  此 DXE_DRIVER 的入口点。
  注意：修改为普通 DXE_DRIVER，不再是 RUNTIME_DRIVER

  @param[in] ImageHandle  固件为 EFI 映像分配的句柄。
  @param[in] SystemTable  指向 EFI 系统表的指针。

  @retval EFI_SUCCESS     入口点成功执行。
  @retval other           发生了一些错误。
**/
EFI_STATUS
EFIAPI
RuntimeInfoEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;

  Print (L"硬件信息 ACPI 驱动: 初始化中...\n");
  DEBUG ((DEBUG_INFO, "硬件信息 ACPI 驱动: 入口点已到达\n"));
  
  // 安装静态 ACPI 表
  Status = InstallAcpiTable();
  if (EFI_ERROR(Status)) {
    Print(L"安装ACPI表失败: %r\n", Status);
    return Status;
  }
  
  Print (L"硬件信息 ACPI 表已成功安装\n");
  Print (L"风扇转速: %d RPM\n", mZhwiTable.FanSpeed);
  Print (L"CPU温度: %d.%d°C\n", mZhwiTable.CpuTemp / 10, mZhwiTable.CpuTemp % 10);
  Print (L"系统负载: %d%%\n", mZhwiTable.SystemLoad);

  return EFI_SUCCESS;
}