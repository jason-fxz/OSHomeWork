[Defines]
    PLATFORM_NAME           = HelloWorldPkg
    PLATFORM_GUID           = 6df5828a-4c9c-4996-9091-884e86455767
    PLATFORM_VERSION        = 0.1
    DSC_SPECIFICATION       = 0x00010005
    SUPPORTED_ARCHITECTURES = X64
    BUILD_TARGETS           = DEBUG|RELEASE

    


[LibraryClasses]
#   如何选择正确的 library class
#   先留空无脑 build
#   如果报错，再添加对应的 library class
#   某些 Lib 可能有多种实现，比如 DebugLib 就有 UefiDebugLibConOut / BaseDebugLibNull /BaseDebugLibSerialPort …… 
#   按需选择一个，不会就问 GPT，或者看其描述

    UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
    UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
    PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
    PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
    MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
    DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
    BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
    BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
    UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
    DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
    UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
    RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
    DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
    StackCheckLib|MdePkg/Library/StackCheckLib/StackCheckLib.inf
    StackCheckFailureHookLib|MdePkg/Library/StackCheckFailureHookLibNull/StackCheckFailureHookLibNull.inf


[Components]
    HelloWorldPkg/HelloWorld.inf
