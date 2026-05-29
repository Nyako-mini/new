#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>

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

static struct kobject *cpu_info_kobj = NULL;
static struct kobject *devices_kobj = NULL;
static struct kobject *soc1_kobj = NULL;

static ssize_t type_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%d\n", cur->type);
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
    return scnprintf(buf, PAGE_SIZE, "%d\n", cur->freq_khz);
}

static ssize_t core_num_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%d\n", cur->cores);
}

static ssize_t cpu_set_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", cur->model);
}

static ssize_t cpu_type_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", cur->model);
}

static ssize_t user_cpu_freq_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", cur->user_freq);
}

static struct kobj_attribute type_attr = __ATTR(type, 0644, type_show, type_store);
static struct kobj_attribute cpu_freq_attr = __ATTR(cpu_freq, 0444, cpu_freq_show, NULL);
static struct kobj_attribute core_num_attr = __ATTR(core_num, 0444, core_num_show, NULL);
static struct kobj_attribute cpu_set_attr = __ATTR(cpu_set, 0444, cpu_set_show, NULL);
static struct kobj_attribute cpu_type_attr = __ATTR(cpu_type, 0444, cpu_type_show, NULL);
static struct kobj_attribute user_cpu_freq_attr = __ATTR(user_cpu_freq, 0444, user_cpu_freq_show, NULL);

static int __init vivo_cpu_info_init(void)
{
    int ret;

    cpu_info_kobj = kobject_create_and_add("cpu_info", NULL);
    if (!cpu_info_kobj) {
        return -ENOMEM;
    }

    ret = sysfs_create_file(cpu_info_kobj, &type_attr.attr);
    if (ret)
        goto err_cpu_info_type;

    ret = sysfs_create_file(cpu_info_kobj, &cpu_freq_attr.attr);
    if (ret)
        goto err_cpu_info_freq;

    ret = sysfs_create_file(cpu_info_kobj, &core_num_attr.attr);
    if (ret)
        goto err_cpu_info_core;

    ret = sysfs_create_file(cpu_info_kobj, &cpu_set_attr.attr);
    if (ret)
        goto err_cpu_info_set;

    ret = sysfs_create_file(cpu_info_kobj, &user_cpu_freq_attr.attr);
    if (ret)
        goto err_cpu_info_user;

    devices_kobj = kobject_create_and_add("devices", NULL);
    if (!devices_kobj) {
        ret = -ENOMEM;
        goto err_devices;
    }

    soc1_kobj = kobject_create_and_add("soc1", devices_kobj);
    if (!soc1_kobj) {
        ret = -ENOMEM;
        goto err_soc1;
    }

    ret = sysfs_create_file(soc1_kobj, &type_attr.attr);
    if (ret)
        goto err_soc1_type;

    ret = sysfs_create_file(soc1_kobj, &cpu_freq_attr.attr);
    if (ret)
        goto err_soc1_freq;

    ret = sysfs_create_file(soc1_kobj, &core_num_attr.attr);
    if (ret)
        goto err_soc1_core;

    ret = sysfs_create_file(soc1_kobj, &cpu_type_attr.attr);
    if (ret)
        goto err_soc1_cpu_type;

    ret = sysfs_create_file(soc1_kobj, &user_cpu_freq_attr.attr);
    if (ret)
        goto err_soc1_user;

    return 0;

err_soc1_user:
    sysfs_remove_file(soc1_kobj, &cpu_type_attr.attr);
err_soc1_cpu_type:
    sysfs_remove_file(soc1_kobj, &core_num_attr.attr);
err_soc1_core:
    sysfs_remove_file(soc1_kobj, &cpu_freq_attr.attr);
err_soc1_freq:
    sysfs_remove_file(soc1_kobj, &type_attr.attr);
err_soc1_type:
    kobject_put(soc1_kobj);
    soc1_kobj = NULL;
err_soc1:
    kobject_put(devices_kobj);
    devices_kobj = NULL;
err_devices:
    sysfs_remove_file(cpu_info_kobj, &user_cpu_freq_attr.attr);
err_cpu_info_user:
    sysfs_remove_file(cpu_info_kobj, &cpu_set_attr.attr);
err_cpu_info_set:
    sysfs_remove_file(cpu_info_kobj, &core_num_attr.attr);
err_cpu_info_core:
    sysfs_remove_file(cpu_info_kobj, &cpu_freq_attr.attr);
err_cpu_info_freq:
    sysfs_remove_file(cpu_info_kobj, &type_attr.attr);
err_cpu_info_type:
    kobject_put(cpu_info_kobj);
    cpu_info_kobj = NULL;
    return ret;
}

static void __exit vivo_cpu_info_exit(void)
{
    if (soc1_kobj) {
        sysfs_remove_file(soc1_kobj, &user_cpu_freq_attr.attr);
        sysfs_remove_file(soc1_kobj, &cpu_type_attr.attr);
        sysfs_remove_file(soc1_kobj, &core_num_attr.attr);
        sysfs_remove_file(soc1_kobj, &cpu_freq_attr.attr);
        sysfs_remove_file(soc1_kobj, &type_attr.attr);
        kobject_put(soc1_kobj);
    }
    if (devices_kobj) {
        kobject_put(devices_kobj);
    }
    if (cpu_info_kobj) {
        sysfs_remove_file(cpu_info_kobj, &user_cpu_freq_attr.attr);
        sysfs_remove_file(cpu_info_kobj, &cpu_set_attr.attr);
        sysfs_remove_file(cpu_info_kobj, &core_num_attr.attr);
        sysfs_remove_file(cpu_info_kobj, &cpu_freq_attr.attr);
        sysfs_remove_file(cpu_info_kobj, &type_attr.attr);
        kobject_put(cpu_info_kobj);
    }
}

pure_initcall(vivo_cpu_info_init);
module_exit(vivo_cpu_info_exit);

MODULE_LICENSE("GPL");
