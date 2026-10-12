#include <pebble.h>
#include "format.h"
#include "config.h"

void format_hms(char *buf, size_t len, uint32_t total_s)
{
	if (total_s > MAX_ELAPSED_S)
		total_s = MAX_ELAPSED_S;
	int h = (int)(total_s / 3600);
	int m = (int)((total_s / 60) % 60);
	int s = (int)(total_s % 60);
	snprintf(buf, len, "%02d:%02d:%02d", h, m, s);
}
