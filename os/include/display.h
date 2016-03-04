#ifndef __OS_DISPLAY_H__
#define __OS_DISPLAY_H__

void displayInit(void);
void task_display_line(const char *fmt, ...);
void os_display_line_at(int line, const char* fmt, ...);

#endif  /* __OS_DISPLAY_H__ */
