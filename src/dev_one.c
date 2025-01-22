/*
 * Copyright (C) 2024  Timotej Milcak <timotejmk94@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

static int major;
static struct class *one_class;
static struct device *one_device;
static atomic_t current_char = ATOMIC_INIT('1');

static ssize_t one_read(struct file *file, char __user *buf, size_t len,
			loff_t *offset)
{
	ssize_t ret = len;

	char *buff = kmalloc(len, GFP_KERNEL);
	if (!buff)
		return -ENOMEM;

	pr_debug("got a read: len = %lu\n", len);

	memset(buff, atomic_read(&current_char), len);

	unsigned long b_failed = copy_to_user(buf, buff, len);
	if (b_failed) {
		pr_alert("failed to copy %lu bytes!\n", b_failed);
		ret = -EFAULT;
	}

	kfree(buff);
	return ret;
}

#define IOCTL_SET_OUTPUT 'S'
#define IOCTL_GET_OUTPUT 'G'

static long one_unl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	switch (cmd) {
		case IOCTL_SET_OUTPUT:
			atomic_set(&current_char, arg);
			pr_info("set output to %c\n", (char)arg);
			break;
		case IOCTL_GET_OUTPUT:
			ret = atomic_read(&current_char);
			break;
		default:
			break;
	}
	return ret;
}

static struct file_operations fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= one_unl_ioctl,
	.read		= one_read,
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
