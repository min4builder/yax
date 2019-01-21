#include <string.h>

char *basename(char *path)
{
	char *cursor;
	if(!path || !*path)
		return (char *) ".";
	cursor = path + strlen(path) - 1;
	while(cursor > path && *cursor == '/')
		*cursor-- = '\0';
	while(cursor > path && *cursor != '/')
		cursor--;
	return cursor;
}

