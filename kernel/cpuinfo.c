#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/fs.h>
#include <linux/device.h>

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

static struct class *cpu_class = NULL;
static struct device *cpu_device = NULL;
static struct device *soc1_device = NULL;

static ssize_t type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%d\n", cur->type);
}

static ssize_t type_store(struct device *dev, struct device_attribute *attr,
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

static ssize_t cpu_freq_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%d\n", cur->freq_khz);
}

static ssize_t core_num_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%d\n", cur->cores);
}

static ssize_t cpu_set_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", cur->model);
}

static ssize_t cpu_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", cur->model);
}

static ssize_t user_cpu_freq_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s\n", cur->user_freq);
}

static DEVICE_ATTR(type, 0644, type_show, type_store);
static DEVICE_ATTR(cpu_freq, 0444, cpu_freq_show, NULL);
static DEVICE_ATTR(core_num, 0444, core_num_show, NULL);
static DEVICE_ATTR(cpu_set, 0444, cpu_set_show, NULL);
static DEVICE_ATTR(cpu_type, 0444, cpu_type_show, NULL);
static DEVICE_ATTR(user_cpu_freq, 0444, user_cpu_freq_show, NULL);

static int __init vivo_cpu_info_init(void)
{
    int ret;

    cpu_class = class_create(THIS_MODULE, "cpu_info");
    if (IS_ERR(cpu_class)) {
        return PTR_ERR(cpu_class);
    }

    cpu_device = device_create(cpu_class, NULL, MKDEV(0, 0), NULL, "info");
    if (IS_ERR(cpu_device)) {
        ret = PTR_ERR(cpu_device);
        goto err_cpu_device;
    }

    ret = device_create_file(cpu_device, &dev_attr_type);
    if (ret)
        goto err_type;

    ret = device_create_file(cpu_device, &dev_attr_cpu_freq);
    if (ret)
        goto err_freq;

    ret = device_create_file(cpu_device, &dev_attr_core_num);
    if (ret)
        goto err_core;

    ret = device_create_file(cpu_device, &dev_attr_cpu_set);
    if (ret)
        goto err_set;

    ret = device_create_file(cpu_device, &dev_attr_user_cpu_freq);
    if (ret)
        goto err_user;

    ret = device_create_file(cpu_device, &dev_attr_cpu_type);
    if (ret)
        goto err_cpu_type;

    device_remove_file(cpu_device, &dev_attr_cpu_type);
    device_remove_file(cpu_device, &dev_attr_user_cpu_freq);
    device_remove_file(cpu_device, &dev_attr_cpu_set);
    device_remove_file(cpu_device, &dev_attr_core_num);
    device_remove_file(cpu_device, &dev_attr_cpu_freq);
    device_remove_file(cpu_device, &dev_attr_type);
    device_destroy(cpu_class, MKDEV(0, 0));
    class_destroy(cpu_class);
    
    cpu_class = class_create(THIS_MODULE, "devices");
    if (IS_ERR(cpu_class)) {
        return PTR_ERR(cpu_class);
    }

    soc1_device = device_create(cpu_class, NULL, MKDEV(0, 0), NULL, "soc1");
    if (IS_ERR(soc1_device)) {
        ret = PTR_ERR(soc1_device);
        goto err_soc1_device;
    }

    ret = device_create_file(soc1_device, &dev_attr_type);
    if (ret)
        goto err_soc1_type;

    ret = device_create_file(soc1_device, &dev_attr_cpu_freq);
    if (ret)
        goto err_soc1_freq;

    ret = device_create_file(soc1_device, &dev_attr_core_num);
    if (ret)
        goto err_soc1_core;

    ret = device_create_file(soc1_device, &dev_attr_cpu_type);
    if (ret)
        goto err_soc1_cpu_type;

    ret = device_create_file(soc1_device, &dev_attr_user_cpu_freq);
    if (ret)
        goto err_soc1_user;

    return 0;

err_soc1_user:
    device_remove_file(soc1_device, &dev_attr_cpu_type);
err_soc1_cpu_type:
    device_remove_file(soc1_device, &dev_attr_core_num);
err_soc1_core:
    device_remove_file(soc1_device, &dev_attr_cpu_freq);
err_soc1_freq:
    device_remove_file(soc1_device, &dev_attr_type);
err_soc1_type:
    device_destroy(cpu_class, MKDEV(0, 0));
err_soc1_device:
    class_destroy(cpu_class);
    return ret;

err_cpu_type:
    device_remove_file(cpu_device, &dev_attr_user_cpu_freq);
err_user:
    device_remove_file(cpu_device, &dev_attr_cpu_set);
err_set:
    device_remove_file(cpu_device, &dev_attr_core_num);
err_core:
    device_remove_file(cpu_device, &dev_attr_cpu_freq);
err_freq:
    device_remove_file(cpu_device, &dev_attr_type);
err_type:
    device_destroy(cpu_class, MKDEV(0, 0));
err_cpu_device:
    class_destroy(cpu_class);
    return ret;
}

static void __exit vivo_cpu_info_exit(void)
{
    if (soc1_device) {
        device_remove_file(soc1_device, &dev_attr_user_cpu_freq);
        device_remove_file(soc1_device, &dev_attr_cpu_type);
        device_remove_file(soc1_device, &dev_attr_core_num);
        device_remove_file(soc1_device, &dev_attr_cpu_freq);
        device_remove_file(soc1_device, &dev_attr_type);
        device_destroy(cpu_class, MKDEV(0, 0));
    }
    if (cpu_class) {
        class_destroy(cpu_class);
    }
}

late_initcall(vivo_cpu_info_init);
module_exit(vivo_cpu_info_exit);

MODULE_LICENSE("GPL");
