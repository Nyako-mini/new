#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

MODULE_LICENSE("GPL");

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
    int ret = 0;
    
    cpu_info_kobj = kobject_create_and_add("cpu_info", NULL);
    if (!cpu_info_kobj) {
        ret = -ENOMEM;
        goto out;
    }

    if (sysfs_create_file(cpu_info_kobj, &type_attr.attr))
        pr_warn("vivo_cpu_info: failed to create type file\n");
    if (sysfs_create_file(cpu_info_kobj, &cpu_freq_attr.attr))
        pr_warn("vivo_cpu_info: failed to create cpu_freq file\n");
    if (sysfs_create_file(cpu_info_kobj, &core_num_attr.attr))
        pr_warn("vivo_cpu_info: failed to create core_num file\n");
    if (sysfs_create_file(cpu_info_kobj, &cpu_set_attr.attr))
        pr_warn("vivo_cpu_info: failed to create cpu_set file\n");
    if (sysfs_create_file(cpu_info_kobj, &user_cpu_freq_attr.attr))
        pr_warn("vivo_cpu_info: failed to create user_cpu_freq file\n");

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

    if (sysfs_create_file(soc1_kobj, &type_attr.attr))
        pr_warn("vivo_cpu_info: failed to create type file in soc1\n");
    if (sysfs_create_file(soc1_kobj, &cpu_freq_attr.attr))
        pr_warn("vivo_cpu_info: failed to create cpu_freq file in soc1\n");
    if (sysfs_create_file(soc1_kobj, &core_num_attr.attr))
        pr_warn("vivo_cpu_info: failed to create core_num file in soc1\n");
    if (sysfs_create_file(soc1_kobj, &cpu_type_attr.attr))
        pr_warn("vivo_cpu_info: failed to create cpu_type file in soc1\n");
    if (sysfs_create_file(soc1_kobj, &user_cpu_freq_attr.attr))
        pr_warn("vivo_cpu_info: failed to create user_cpu_freq file in soc1\n");

    pr_info("vivo_cpu_info: initialized successfully\n");
    return 0;

err_soc1:
    kobject_put(devices_kobj);
    devices_kobj = NULL;
err_devices:
    if (cpu_info_kobj) {
        sysfs_remove_file(cpu_info_kobj, &user_cpu_freq_attr.attr);
        sysfs_remove_file(cpu_info_kobj, &cpu_set_attr.attr);
        sysfs_remove_file(cpu_info_kobj, &core_num_attr.attr);
        sysfs_remove_file(cpu_info_kobj, &cpu_freq_attr.attr);
        sysfs_remove_file(cpu_info_kobj, &type_attr.attr);
        kobject_put(cpu_info_kobj);
        cpu_info_kobj = NULL;
    }
out:
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
    pr_info("vivo_cpu_info: exited\n");
}

late_initcall(vivo_cpu_info_init);
module_exit(vivo_cpu_info_exit);
