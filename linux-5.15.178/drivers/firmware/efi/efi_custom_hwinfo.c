#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/efi.h>
#include <linux/io.h>

// 自定义 ACPI 表的签名
#define ZHWI_SIGNATURE    "ZHWI"  // ACPI 表签名

// 函数指针类型定义
typedef uint64_t (*get_fan_speed_t)(void);
typedef uint64_t (*get_cpu_temp_t)(void);
typedef uint64_t (*get_system_load_t)(void);
typedef uint64_t (*get_timestamp_t)(void);

// 运行时函数结构
struct zhwi_runtime_functions {
    get_fan_speed_t get_fan_speed;
    get_cpu_temp_t get_cpu_temp;
    get_system_load_t get_system_load;
    get_timestamp_t get_timestamp;
};

// 自定义 ACPI 表的结构 (与UEFI端保持一致)
struct zhwi_table {
    struct acpi_table_header header;
    u32 revision;
    u32 fan_speed;
    u32 cpu_temp;
    u32 system_load;
    u64 timestamp;
    u64 runtime_functions_ptr; // 新增：指向运行时函数表的指针
} __packed;

// 全局变量
static struct zhwi_table *zhwi_tbl = NULL;
static struct kobject *hw_info_kobj = NULL;
static struct zhwi_runtime_functions __iomem *runtime_funcs = NULL;

// sysfs 属性展示函数
static ssize_t fan_speed_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    u64 speed;
    
    // 优先使用运行时函数获取最新数据
    if (runtime_funcs && runtime_funcs->get_fan_speed) {
        speed = runtime_funcs->get_fan_speed();
        return sysfs_emit(buf, "%llu\n", speed);
    }
    
    // 回退到表中的静态值
    if (!zhwi_tbl)
        return -ENODEV;
    
    return sysfs_emit(buf, "%u\n", zhwi_tbl->fan_speed);
}

static ssize_t cpu_temp_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    u64 temp;
    
    // 优先使用运行时函数获取最新数据
    if (runtime_funcs && runtime_funcs->get_cpu_temp) {
        temp = runtime_funcs->get_cpu_temp();
        return sysfs_emit(buf, "%llu.%llu\n", temp / 10, temp % 10);
    }
    
    // 回退到表中的静态值
    if (!zhwi_tbl)
        return -ENODEV;
    
    // 处理温度的小数部分 (0.1°C 格式)
    return sysfs_emit(buf, "%u.%u\n", zhwi_tbl->cpu_temp / 10, zhwi_tbl->cpu_temp % 10);
}

static ssize_t system_load_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    u64 load;
    
    // 优先使用运行时函数获取最新数据
    if (runtime_funcs && runtime_funcs->get_system_load) {
        load = runtime_funcs->get_system_load();
        return sysfs_emit(buf, "%llu\n", load);
    }
    
    // 回退到表中的静态值
    if (!zhwi_tbl)
        return -ENODEV;
    
    return sysfs_emit(buf, "%u\n", zhwi_tbl->system_load);
}

static ssize_t timestamp_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    u64 timestamp;
    
    // 优先使用运行时函数获取最新数据
    if (runtime_funcs && runtime_funcs->get_timestamp) {
        timestamp = runtime_funcs->get_timestamp();
        return sysfs_emit(buf, "%llu\n", timestamp);
    }
    
    // 回退到表中的静态值
    if (!zhwi_tbl)
        return -ENODEV;
    
    return sysfs_emit(buf, "%llu\n", zhwi_tbl->timestamp);
}

// 定义 sysfs 属性
static struct kobj_attribute fan_speed_attribute = 
    __ATTR(fan_speed, 0444, fan_speed_show, NULL);
static struct kobj_attribute cpu_temp_attribute = 
    __ATTR(cpu_temp, 0444, cpu_temp_show, NULL);
static struct kobj_attribute system_load_attribute = 
    __ATTR(system_load, 0444, system_load_show, NULL);
static struct kobj_attribute timestamp_attribute = 
    __ATTR(timestamp, 0444, timestamp_show, NULL);

// 属性组
static struct attribute *hw_info_attrs[] = {
    &fan_speed_attribute.attr,
    &cpu_temp_attribute.attr,
    &system_load_attribute.attr,
    &timestamp_attribute.attr,
    NULL,
};

static struct attribute_group hw_info_attr_group = {
    .attrs = hw_info_attrs,
    .name = "sensors"
};

/**
 * 读取并解析 ZHWI ACPI 表
 */
static int __init read_zhwi_acpi_table(void)
{
    acpi_status status;
    
    // 尝试获取 ZHWI ACPI 表
    status = acpi_get_table(ZHWI_SIGNATURE, 0, (struct acpi_table_header **)&zhwi_tbl);
    if (ACPI_FAILURE(status)) {
        pr_info("ZHWI HW Info: 无法找到 ZHWI ACPI 表 (status: 0x%x)，跳过初始化\n", status);
        return -ENODEV;
    }
    
    // 验证表结构
    if (zhwi_tbl->header.length < sizeof(struct zhwi_table)) {
        pr_warn("ZHWI HW Info: ACPI 表大小不匹配。期望 %zu，实际 %u\n",
               sizeof(struct zhwi_table), zhwi_tbl->header.length);
        // 仍继续使用，但记录警告
    }
    
    pr_info("ZHWI HW Info: 成功获取 ZHWI ACPI 表\n");
    pr_info("ZHWI HW Info: 风扇转速: %u RPM, CPU温度: %u.%u°C, 系统负载: %u%%\n",
            zhwi_tbl->fan_speed, 
            zhwi_tbl->cpu_temp / 10, zhwi_tbl->cpu_temp % 10,
            zhwi_tbl->system_load);
            
    // 映射运行时函数表
    if (zhwi_tbl->runtime_functions_ptr && FALSE) {
        runtime_funcs = ioremap(zhwi_tbl->runtime_functions_ptr, 
                               sizeof(*runtime_funcs));
        if (runtime_funcs) {
            pr_info("ZHWI HW Info: 成功映射运行时函数表 @ 0x%llx\n", 
                   (unsigned long long)zhwi_tbl->runtime_functions_ptr);
            
            // 验证函数指针
            if (runtime_funcs->get_fan_speed)
                pr_info("ZHWI HW Info: 风扇转速运行时函数可用\n");
            if (runtime_funcs->get_cpu_temp)
                pr_info("ZHWI HW Info: CPU温度运行时函数可用\n");
            if (runtime_funcs->get_system_load)
                pr_info("ZHWI HW Info: 系统负载运行时函数可用\n");
            if (runtime_funcs->get_timestamp)
                pr_info("ZHWI HW Info: 时间戳运行时函数可用\n");
        } else {
            pr_warn("ZHWI HW Info: 无法映射运行时函数表\n");
        }
    } else {
        pr_info("ZHWI HW Info: 运行时函数表不可用，将使用静态数据\n");
    }
    
    return 0;
}

/**
 * 模块初始化函数
 */
static int __init hw_info_init(void)
{
    int ret;
    
    // 读取 ACPI 表
    ret = read_zhwi_acpi_table();
    if (ret) {
        return ret; // 找不到表，直接返回
    }
    // 创建 sysfs 接口 - 在 /sys/firmware/ 下
    hw_info_kobj = kobject_create_and_add("zhwi", firmware_kobj);
    if (!hw_info_kobj) {
        pr_warn("ZHWI HW Info: 无法创建 kobject\n");
        return -ENOMEM;
    }
    
    // 创建属性组
    ret = sysfs_create_group(hw_info_kobj, &hw_info_attr_group);
    if (ret) {
        pr_warn("ZHWI HW Info: 无法创建 sysfs 属性组\n");
        kobject_put(hw_info_kobj);
        return ret;
    }
    
    // 创建到 efi_kobj 的符号链接，以保持与原有接口的兼容性
    if (efi_kobj) { // 确保 EFI 子系统已初始化
        ret = sysfs_create_link(efi_kobj, hw_info_kobj, "hw_sensors");
        if (ret)
            pr_warn("ZHWI HW Info: 无法创建到 efi_kobj 的符号链接\n");
    }
    
    pr_info("ZHWI HW Info: sysfs 接口已创建于 /sys/firmware/acpi/zhwi/sensors/\n");
    return 0;
}



// 使用 late_initcall 确保 ACPI 子系统已初始化
late_initcall(hw_info_init);
