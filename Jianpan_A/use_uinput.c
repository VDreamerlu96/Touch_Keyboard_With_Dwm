#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <string.h>

const char* linux_input_device = "/dev/uinput"; /*linux input注册设备*/

int open_uinput_device() {
    int fd = open(linux_input_device, O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        printf("Unable to open %s",linux_input_device);
        exit(EXIT_FAILURE);
    }
    return fd;
}

void setup_uinput_keyboard(int fd) {
    struct uinput_user_dev uidev;

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-keyboard");

    // 启用按键事件
    ioctl(fd, UI_SET_EVBIT, EV_KEY);

    // 支持所有US键盘布局的按键
    for (int i = KEY_ESC; i <= KEY_DELETE; i++) {
        ioctl(fd, UI_SET_KEYBIT, i);
    }
    for (int i = KEY_F1; i <= KEY_F12; i++) {
        ioctl(fd, UI_SET_KEYBIT, i);
    }
    for (int i = KEY_KP0; i <= KEY_KPEQUAL; i++) {
        ioctl(fd, UI_SET_KEYBIT, i);
    }
    ioctl(fd, UI_SET_KEYBIT, KEY_SPACE);
    ioctl(fd, UI_SET_KEYBIT, KEY_ENTER);
    ioctl(fd, UI_SET_KEYBIT, KEY_TAB);
    ioctl(fd, UI_SET_KEYBIT, KEY_BACKSPACE);
    ioctl(fd, UI_SET_KEYBIT, KEY_CAPSLOCK);
    ioctl(fd, UI_SET_KEYBIT, KEY_NUMLOCK);
    ioctl(fd, UI_SET_KEYBIT, KEY_SCROLLLOCK);
    ioctl(fd, UI_SET_KEYBIT, KEY_LEFTCTRL);
    ioctl(fd, UI_SET_KEYBIT, KEY_LEFTSHIFT);
    ioctl(fd, UI_SET_KEYBIT, KEY_LEFTALT);
    ioctl(fd, UI_SET_KEYBIT, KEY_LEFTMETA);
    ioctl(fd, UI_SET_KEYBIT, KEY_RIGHTCTRL);
    ioctl(fd, UI_SET_KEYBIT, KEY_RIGHTSHIFT);
    ioctl(fd, UI_SET_KEYBIT, KEY_RIGHTALT);
    ioctl(fd, UI_SET_KEYBIT, KEY_RIGHTMETA);

    // 创建设备
    write(fd, &uidev, sizeof(uidev));
    ioctl(fd, UI_DEV_CREATE);
}

void setup_uinput_mouse(int fd) {
    struct uinput_user_dev uidev;
    
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-mouse");

    // 启用鼠标事件
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_REL);

    // 启用鼠标按键
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);

    // 启用鼠标相对移动
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);

    // 创建设备
    write(fd, &uidev, sizeof(uidev));
    ioctl(fd, UI_DEV_CREATE);
}

void send_event(int fd, int type, int code, int value) {
    struct input_event ev;

    memset(&ev, 0, sizeof(ev));
    ev.type = type;
    ev.code = code;
    ev.value = value;

    write(fd, &ev, sizeof(ev));
}

void send_key_event(int fd, int key_code, int value) {
    struct input_event ev;

    // 发送按键事件
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_KEY;
    ev.code = key_code;
    ev.value = value;
    write(fd, &ev, sizeof(ev));

    // 发送同步事件
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(ev));
}

void touch_key_event(int fd,int key_code){
    send_key_event(fd, key_code, 1);
    send_key_event(fd, key_code, 0);
}

void touch_shift_key_event(int fd, int key_code) {
    send_key_event(fd, KEY_LEFTSHIFT, 1);
    send_key_event(fd, key_code, 1);
    send_key_event(fd, key_code, 0);
    send_key_event(fd, KEY_LEFTSHIFT, 0);
}

void send_mouse_event(int fd, int rel_x, int rel_y, int btn_left, int btn_right) {
    if (rel_x != 0) send_event(fd, EV_REL, REL_X, rel_x);
    if (rel_y != 0) send_event(fd, EV_REL, REL_Y, rel_y);
    if (btn_left != -1) send_event(fd, EV_KEY, BTN_LEFT, btn_left);
    if (btn_right != -1) send_event(fd, EV_KEY, BTN_RIGHT, btn_right);
    send_event(fd, EV_SYN, SYN_REPORT, 0);
}

void destroy_uinput_device(int fd) {
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
}