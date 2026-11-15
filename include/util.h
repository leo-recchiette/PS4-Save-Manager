#ifndef _UTIL_H_
#define _UTIL_H_

#include <apollo.h>
#include "types.h"

#define NOTIFICATION_ICON_BAN           "cxml://psnotification/tex_icon_ban"
#define NOTIFICATION_ICON_DEFAULT       "cxml://psnotification/tex_default_icon_notification"

#define ALIGN(_value, _alignment) (((_value) + (_alignment) - 1) & ~((_alignment) - 1))

int get_file_size(const char *file_path, u64 *size);
int read_file(const char *file_path, u8 *data, u64 size);

void notify_popup(const char *p_Uri, const char *p_Format, ...);

#endif /* !_UTIL_H_ */
