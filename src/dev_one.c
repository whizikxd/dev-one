#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>

MODULE_AUTHOR("Timotej Milcak");
MODULE_DESCRIPTION("A Simple chrdev driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

#define DEVICE_NAME "one"

#define pr_fmt(fmt) DEVICE_NAME ": " fmt

static int major = -1;
static struct class *one_class = 0;
static struct device *one_device = 0;

static ssize_t one_read(struct file *file, char __user *buf, size_t len,
			loff_t *offset)
{
	int ret = len;

	char *buff = kmalloc(len + 1, GFP_KERNEL);
	if (!buff) {
		return -ENOMEM;
	}

	pr_info("got a read: len = %lu\n", len);

	memset(buff, '1', len);
	buff[len] = '\0';

	if (copy_to_user(buf, buff, len)) {
		ret = -EFAULT;
		goto cleanup;
	}

cleanup:
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
		ret = major;
		goto end;
	}

	one_class = class_create(DEVICE_NAME);
	if (IS_ERR(one_class)) {
		ret = -EFAULT;
		goto error1;
	}

	one_device = device_create(one_class, 0, MKDEV(major, 0), 0, DEVICE_NAME);
	if (IS_ERR(one_device)) {
		ret = -EFAULT;
		goto error2;
	}

	pr_info("sucessfully initialized, major = %d\n", major);
	goto end;
error2:
	class_destroy(one_class);
error1:
	unregister_chrdev(major, DEVICE_NAME);
end:
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
