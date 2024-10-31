#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>

MODULE_AUTHOR("Timotej Milcak");
MODULE_DESCRIPTION("A Simple chrdev driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

#define DEVICE_NAME "one"
#define DEVICE_CLASS DEVICE_NAME

#undef pr_fmt
#define pr_fmt(fmt) DEVICE_NAME ": " fmt

static int major = -1;
static struct class *one_class = 0;
static struct device *one_device = 0;

static ssize_t one_read(struct file *file, char __user *buf, size_t len,
			loff_t *offset)
{
	ssize_t ret = len;

	char *buff = kmalloc(len, GFP_KERNEL);
	if (!buff)
		return -ENOMEM;

	pr_debug("got a read: len = %lu\n", len);

	memset(buff, '1', len);

	unsigned long b_failed = copy_to_user(buf, buff, len);
	if (b_failed) {
		pr_alert("failed to copy %lu bytes!\n", b_failed);
		ret = -EFAULT;
	}

	kfree(buff);
	return len;
}

static struct file_operations fops = {
	.owner	= THIS_MODULE,
	.read	= one_read,
};

static int __init one_init(void)
{
	int ret = 0;

	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) {
		pr_alert("failed to register chrdev!\n");
		return major;
	}

	one_class = class_create(DEVICE_CLASS);
	if (IS_ERR(one_class)) {
		pr_alert("failed to create class!\n");

		ret = PTR_ERR(one_class);
		goto error1;
	}

	one_device = device_create(one_class, 0, MKDEV(major, 0), 0, DEVICE_NAME);
	if (IS_ERR(one_device)) {
		pr_alert("failed to create device!\n");
		
		ret = PTR_ERR(one_device);
		goto error2;
	}

	pr_info("sucessfully initialized, major = %d\n", major);
	return 0;
error2:
	class_destroy(one_class);
error1:
	unregister_chrdev(major, DEVICE_NAME);
	return ret;
}

static void __exit one_exit(void)
{
	device_destroy(one_class, MKDEV(major, 0));
	class_destroy(one_class);
	unregister_chrdev(major, DEVICE_NAME);

	pr_info("module exit\n");
}

module_init(one_init);
module_exit(one_exit);
