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

static ssize_t read_source_file(const char *path, char *buf, size_t count)
{
    struct file *file;
    ssize_t ret;
    loff_t pos = 0;
    char *tmp_buf;
    mm_segment_t old_fs;

    file = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(file))
        return PTR_ERR(file);

    tmp_buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!tmp_buf) {
        filp_close(file, NULL);
        return -ENOMEM;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    ret = file->f_op->read(file, tmp_buf, PAGE_SIZE - 1, &pos);
    set_fs(old_fs);

    if (ret >= 0) {
        tmp_buf[ret] = '\0';
        snprintf(buf, count, "%s", tmp_buf);
    }

    kfree(tmp_buf);
    filp_close(file, NULL);
    return ret;
}

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
    fuel_class = class_create(THIS_MODULE, "fuelsummary");
    if (IS_ERR(fuel_class))
        return PTR_ERR(fuel_class);

    fuel_class->dev_groups = fuel_groups;
    return 0;
}

static void __exit fuelsummary_exit(void)
{
    if (fuel_class)
        class_destroy(fuel_class);
}

module_init(fuelsummary_init);
module_exit(fuelsummary_exit);

MODULE_LICENSE("GPL");
