#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/file.h>
#include <linux/uaccess.h>

#define SRC_SOH "/sys/class/qcom-battery/soh"
#define SRC_CYCLE "/sys/class/power_supply/battery/cycle_count"

static struct class *fuel_class;
static struct device *fuel_device;


static ssize_t soh_show(struct class *cls, struct class_attribute *attr, char *buf)
{
    return read_source_file(SRC_SOH, buf, PAGE_SIZE);
}

static ssize_t cycle_show(struct class *cls, struct class_attribute *attr, char *buf)
{
    return read_source_file(SRC_CYCLE, buf, PAGE_SIZE);
}

static CLASS_ATTR_RO(soh);
static CLASS_ATTR_RO(cycle);

static struct attribute *fuel_attrs[] = {
    &class_attr_soh.attr,
    &class_attr_cycle.attr,
    NULL,
};
ATTRIBUTE_GROUPS(fuel);

static int __init fuelsummary_init(void)
{
    int ret;

    fuel_class = class_create(THIS_MODULE, "fuelsummary");
    if (IS_ERR(fuel_class))
        return PTR_ERR(fuel_class);

    fuel_device = device_create(fuel_class, NULL, MKDEV(0, 0), NULL, "summary");
    if (IS_ERR(fuel_device)) {
        ret = PTR_ERR(fuel_device);
        goto err_device;
    }

    fuel_class->dev_groups = fuel_groups;
    
    pr_info("fuelsummary: initialized\n");
    return 0;

err_device:
    class_destroy(fuel_class);
    return ret;
}

static void __exit fuelsummary_exit(void)
{
    if (fuel_device)
        device_destroy(fuel_class, MKDEV(0, 0));
    if (fuel_class)
        class_destroy(fuel_class);
    pr_info("fuelsummary: exited\n");
}

module_init(fuelsummary_init);
module_exit(fuelsummary_exit);

MODULE_LICENSE("GPL");
