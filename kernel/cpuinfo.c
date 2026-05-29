#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/fs.h>

#define DRIVER_NAME "vivo_cpu_info"

struct cpu_config {
    int type;
    const char *user_freq;
    const char *model;
    int cores;
    int freq_khz;
};

static struct cpu_config high_perf = {
    .type = 1,
    .user_freq = "2.96GHz",
    .model = "855",
    .cores = 8,
    .freq_khz = 2960000,
};

static struct cpu_config normal = {
    .type = 0,
    .user_freq = "2.84GHz",
    .model = "855",
    .cores = 8,
    .freq_khz = 2840000,
};

static struct cpu_config *cur = &high_perf;

static struct kobject *cpu_info_kobj;
static struct kobject *devices_kobj;
static struct kobject *soc1_kobj;

static ssize_t type_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", cur->type);
}

static ssize_t type_store(struct kobject *kobj, struct kobj_attribute *attr,
                          const char *buf, size_t count)
{
    int ret, val;
    ret = kstrtoint(buf, 10, &val);
    if (ret < 0)
        return ret;
    
    if (val == 1) {
        cur = &high_perf;
    } else if (val == 0) {
        cur = &normal;
    } else {
        return -EINVAL;
    }
    return count;
}

static ssize_t cpu_freq_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", cur->freq_khz);
}

static ssize_t core_num_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", cur->cores);
}

static ssize_t cpu_set_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", cur->model);
}

static ssize_t cpu_type_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", cur->model);
}

static ssize_t user_cpu_freq_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", cur->user_freq);
}

static struct kobj_attribute type_attribute = __ATTR(type, 0644, type_show, type_store);
static struct kobj_attribute cpu_freq_attribute = __ATTR(cpu_freq, 0444, cpu_freq_show, NULL);
static struct kobj_attribute core_num_attribute = __ATTR(core_num, 0444, core_num_show, NULL);
static struct kobj_attribute cpu_set_attribute = __ATTR(cpu_set, 0444, cpu_set_show, NULL);
static struct kobj_attribute cpu_type_attribute = __ATTR(cpu_type, 0444, cpu_type_show, NULL);
static struct kobj_attribute user_cpu_freq_attribute = __ATTR(user_cpu_freq, 0444, user_cpu_freq_show, NULL);

static int __init vivo_cpu_info_init(void)
{
    printk(KERN_INFO "%s: init started\n", DRIVER_NAME);

    cpu_info_kobj = kobject_create_and_add("cpu_info", NULL);
    if (!cpu_info_kobj) {
        printk(KERN_ERR "%s: failed to create cpu_info kobject\n", DRIVER_NAME);
        return -ENOMEM;
    }
    printk(KERN_INFO "%s: created cpu_info kobject at %p\n", DRIVER_NAME, cpu_info_kobj);

    if (sysfs_create_file(cpu_info_kobj, &type_attribute.attr)) {
        printk(KERN_ERR "%s: failed to create type file\n", DRIVER_NAME);
        goto err_cpu_info;
    }
    if (sysfs_create_file(cpu_info_kobj, &cpu_freq_attribute.attr)) {
        printk(KERN_ERR "%s: failed to create cpu_freq file\n", DRIVER_NAME);
        goto err_cpu_info;
    }
    if (sysfs_create_file(cpu_info_kobj, &core_num_attribute.attr)) {
        printk(KERN_ERR "%s: failed to create core_num file\n", DRIVER_NAME);
        goto err_cpu_info;
    }
    if (sysfs_create_file(cpu_info_kobj, &cpu_set_attribute.attr)) {
        printk(KERN_ERR "%s: failed to create cpu_set file\n", DRIVER_NAME);
        goto err_cpu_info;
    }
    if (sysfs_create_file(cpu_info_kobj, &user_cpu_freq_attribute.attr)) {
        printk(KERN_ERR "%s: failed to create user_cpu_freq file\n", DRIVER_NAME);
        goto err_cpu_info;
    }

    devices_kobj = kobject_create_and_add("devices", NULL);
    if (!devices_kobj) {
        printk(KERN_ERR "%s: failed to create devices kobject\n", DRIVER_NAME);
        goto err_cpu_info;
    }
    printk(KERN_INFO "%s: created devices kobject at %p\n", DRIVER_NAME, devices_kobj);

    soc1_kobj = kobject_create_and_add("soc1", devices_kobj);
    if (!soc1_kobj) {
        printk(KERN_ERR "%s: failed to create soc1 kobject\n", DRIVER_NAME);
        goto err_devices;
    }
    printk(KERN_INFO "%s: created soc1 kobject at %p\n", DRIVER_NAME, soc1_kobj);

    if (sysfs_create_file(soc1_kobj, &type_attribute.attr)) {
        printk(KERN_ERR "%s: failed to create type file in soc1\n", DRIVER_NAME);
        goto err_soc1;
    }
    if (sysfs_create_file(soc1_kobj, &cpu_freq_attribute.attr)) {
        printk(KERN_ERR "%s: failed to create cpu_freq file in soc1\n", DRIVER_NAME);
        goto err_soc1;
    }
    if (sysfs_create_file(soc1_kobj, &core_num_attribute.attr)) {
        printk(KERN_ERR "%s: failed to create core_num file in soc1\n", DRIVER_NAME);
        goto err_soc1;
    }
    if (sysfs_create_file(soc1_kobj, &cpu_type_attribute.attr)) {
        printk(KERN_ERR "%s: failed to create cpu_type file in soc1\n", DRIVER_NAME);
        goto err_soc1;
    }
    if (sysfs_create_file(soc1_kobj, &user_cpu_freq_attribute.attr)) {
        printk(KERN_ERR "%s: failed to create user_cpu_freq file in soc1\n", DRIVER_NAME);
        goto err_soc1;
    }

    printk(KERN_INFO "%s: init completed successfully\n", DRIVER_NAME);
    return 0;

err_soc1:
    kobject_put(soc1_kobj);
    soc1_kobj = NULL;
err_devices:
    kobject_put(devices_kobj);
    devices_kobj = NULL;
err_cpu_info:
    if (cpu_info_kobj) {
        sysfs_remove_file(cpu_info_kobj, &user_cpu_freq_attribute.attr);
        sysfs_remove_file(cpu_info_kobj, &cpu_set_attribute.attr);
        sysfs_remove_file(cpu_info_kobj, &core_num_attribute.attr);
        sysfs_remove_file(cpu_info_kobj, &cpu_freq_attribute.attr);
        sysfs_remove_file(cpu_info_kobj, &type_attribute.attr);
        kobject_put(cpu_info_kobj);
        cpu_info_kobj = NULL;
    }
    return -ENOMEM;
}

static void __exit vivo_cpu_info_exit(void)
{
    if (soc1_kobj) {
        sysfs_remove_file(soc1_kobj, &user_cpu_freq_attribute.attr);
        sysfs_remove_file(soc1_kobj, &cpu_type_attribute.attr);
        sysfs_remove_file(soc1_kobj, &core_num_attribute.attr);
        sysfs_remove_file(soc1_kobj, &cpu_freq_attribute.attr);
        sysfs_remove_file(soc1_kobj, &type_attribute.attr);
        kobject_put(soc1_kobj);
    }
    if (devices_kobj) {
        kobject_put(devices_kobj);
    }
    if (cpu_info_kobj) {
        sysfs_remove_file(cpu_info_kobj, &user_cpu_freq_attribute.attr);
        sysfs_remove_file(cpu_info_kobj, &cpu_set_attribute.attr);
        sysfs_remove_file(cpu_info_kobj, &core_num_attribute.attr);
        sysfs_remove_file(cpu_info_kobj, &cpu_freq_attribute.attr);
        sysfs_remove_file(cpu_info_kobj, &type_attribute.attr);
        kobject_put(cpu_info_kobj);
    }
    printk(KERN_INFO "%s: exit completed\n", DRIVER_NAME);
}

subsys_initcall(vivo_cpu_info_init);
module_exit(vivo_cpu_info_exit);

MODULE_LICENSE("GPL");
