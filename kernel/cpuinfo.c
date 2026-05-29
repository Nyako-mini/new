#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/kobject.h>

#define DRIVER_NAME "vivo_cpu_info"

#define NEW_PATH "cpu_info"
#define OLD_PATH "soc1"
#define DEVICES_PATH "devices"

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

static struct kobject *new_kobj = NULL;
static struct kobject *old_kobj = NULL;
static struct kobject *dev_kobj = NULL;

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
        pr_info("%s: high performance mode (2.96GHz)\n", DRIVER_NAME);
    } else if (val == 0) {
        cur = &normal;
        pr_info("%s: normal mode (2.84GHz)\n", DRIVER_NAME);
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

static struct attribute *new_attrs[] = {
    &type_attr.attr,
    &cpu_freq_attr.attr,
    &core_num_attr.attr,
    &cpu_set_attr.attr,
    &user_cpu_freq_attr.attr,
    NULL,
};

static const struct attribute_group new_group = {
    .attrs = new_attrs,
};

static struct attribute *old_attrs[] = {
    &type_attr.attr,
    &cpu_freq_attr.attr,
    &core_num_attr.attr,
    &cpu_type_attr.attr,
    &user_cpu_freq_attr.attr,
    NULL,
};

static const struct attribute_group old_group = {
    .attrs = old_attrs,
};

static int __init vivo_cpu_info_init(void)
{
    int ret;

    new_kobj = kobject_create_and_add(NEW_PATH, NULL);
    if (!new_kobj) {
        ret = -ENOMEM;
        goto err;
    }

    ret = sysfs_create_group(new_kobj, &new_group);
    if (ret)
        goto err_new;

    dev_kobj = kobject_create_and_add(DEVICES_PATH, NULL);
    if (!dev_kobj) {
        ret = -ENOMEM;
        goto err_new_group;
    }

    old_kobj = kobject_create_and_add(OLD_PATH, dev_kobj);
    if (!old_kobj) {
        ret = -ENOMEM;
        goto err_dev;
    }

    ret = sysfs_create_group(old_kobj, &old_group);
    if (ret)
        goto err_old;

    pr_info("%s: initialized (type=%d, %s)\n", DRIVER_NAME, cur->type, cur->user_freq);
    return 0;

err_old:
    kobject_put(old_kobj);
    old_kobj = NULL;
err_dev:
    kobject_put(dev_kobj);
    dev_kobj = NULL;
err_new_group:
    sysfs_remove_group(new_kobj, &new_group);
err_new:
    kobject_put(new_kobj);
    new_kobj = NULL;
err:
    return ret;
}

static void __exit vivo_cpu_info_exit(void)
{
    if (old_kobj) {
        sysfs_remove_group(old_kobj, &old_group);
        kobject_put(old_kobj);
    }
    if (dev_kobj)
        kobject_put(dev_kobj);
    if (new_kobj) {
        sysfs_remove_group(new_kobj, &new_group);
        kobject_put(new_kobj);
    }
    pr_info("%s: removed\n", DRIVER_NAME);
}

late_initcall(vivo_cpu_info_init);
module_exit(vivo_cpu_info_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vivo");
MODULE_DESCRIPTION("Vivo CPU Info Driver");
