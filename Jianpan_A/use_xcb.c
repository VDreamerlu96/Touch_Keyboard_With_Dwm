#include <xcb/xcb.h>
#include <string.h>
#include"error.h"

// 连接到 X 服务器
void Start_Xconnect(xcb_connection_t **xc, int *screen_nbr, const char *display_name) {
    *xc = xcb_connect(display_name, screen_nbr);
    if (*xc == NULL || xcb_connection_has_error(*xc)) {
        Error_Exit("Failed to connect to X server", 0);
    }
}

// 获取屏幕信息
void Get_Screen_Info(xcb_connection_t *conn, xcb_screen_t **screen_info, int screen_nbr) {
    // 获取 XCB 的设置结构体
    const xcb_setup_t *setup = xcb_get_setup(conn);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);

    // 检查 screen_nbr 是否有效
    int screen_count = 0;
    while (iter.rem) {
        screen_count++;
        xcb_screen_next(&iter);
    }

    if (screen_nbr >= screen_count) {
        Error_Exit("Invalid screen number", 0);
    }

    // 重新迭代并找到指定的屏幕
    iter = xcb_setup_roots_iterator(setup);
    for (int i = 0; iter.rem; xcb_screen_next(&iter), i++) {
        if (i == screen_nbr) {
            *screen_info = iter.data;
            break;
        }
    }

    if (!*screen_info) {
        Error_Exit("Failed to get screen info", 0);
    }
}

// 断开与 X 服务器的连接
void Stop_Xconnect(xcb_connection_t *xc) {
    if (xc) {
        xcb_disconnect(xc);
    }
}

