#pragma once

#include <linux/input.h>

extern int open_uinput_device(void);
extern void setup_uinput_keyboard(int fd);
extern void setup_uinput_mouse(int fd);
extern void setup_uinput_mouse_and_keyboard(int fd);
extern void send_event(int fd, int type, int code, int value);
extern void send_key_event(int fd, int key_code, int value);
extern void touch_key_event(int fd,int key_code);
extern void touch_alt_key_event(int fd, int key_code);
extern void touch_ctrl_key_event(int fd, int key_code);
extern void touch_shift_key_event(int fd, int key_code);
extern void send_mouse_event(int fd, int rel_x, int rel_y, int btn_left, int btn_right);
extern void destroy_uinput_device(int fd);
extern void mouse_scroll(int fd, int scroll_count);    /*模拟鼠标滚动，count>0向上，count<0向下*/