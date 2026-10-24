#ifndef APP_OLED_H
#define APP_OLED_H

/* Draw the greeting; oled_init() must succeed first. Returns 0 or an error. */
int write_oled(const char *text);
int oled_init(void);

#endif
