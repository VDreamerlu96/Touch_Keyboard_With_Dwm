#include"use_uinput.h"
#include"use_xcb.h"
#include"error.h"
#include<string.h>
#include<stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../config.h"

// 全局连接变量
static xcb_connection_t *conn = NULL;
static int ufd = 0;
static int uufd = 0;
// 清理函数
static void board_out(int no) {
    if (conn) {
        xcb_disconnect(conn);
        conn = NULL;
    }
    if(ufd){
        destroy_uinput_device(ufd);
        ufd = 0;
    }
    if(uufd){
        destroy_uinput_device(uufd);
        uufd = 0;
    }
}

static void setup_signal_handler(void) {
    signal(SIGINT, board_out);
}


/*xlsfonts or fc-list look*/
const char* global_font_name = "6x13";         /*字体,系统要有，不然会打开失败，而且最好根据当前屏幕选一个合适大小的*/

static uint32_t global_foreground_color = 0;    /*前景色*/
static uint32_t global_background_color = 0;    /*背景色*/

#define Board_Height_Num 5
#define Board_Wight_Num 7

#define Shift_Key_y 3   /*根据shift的数组索引填*/
#define Shift_Key_x 0
#define Ctrl_Key_y 4
#define Ctrl_Key_x 0
#define Alt_Key_y 4
#define Alt_Key_x 1

const char* keytype_left[Board_Height_Num][Board_Wight_Num] = {
    {"Esc", "1", "2", "3", "4", "5", "6"},
    {"`", "q", "w", "e", "r", "t", "y"},
    {"Tab", "a", "s", "d", "f", "g", "h"},
    {"Sft", "z", "x", "c", "v", "b", "n"},
    {"Ctl", "Alt", "Win", "Fn", "", "", ""}
};

const char* keytype_right[Board_Height_Num][Board_Wight_Num] = {
    {"7", "8", "9", "0", "-", "=", "Back"},
    {"u", "i", "o", "p", "[", "]", "\\"},
    {"j", "k", "l", ";", "'", "-^-", "Ent"},
    {"m", ",", ".", "/", "<-", "_|_", "->"},
    {"Hide", "FChange", "", "Spa->", "", "", "<-Spa"}
};

const char* Keytype_left[Board_Height_Num][Board_Wight_Num] = {
    {"Esc", "!", "@", "#", "$", "\%", "^"},
    {"`", "Q", "W", "E", "R", "T", "Y"},
    {"Tab", "A", "S", "D", "F", "G", "H"},
    {"Sft*", "Z", "X", "C", "V", "B", "N"},
    {"Ctl", "Alt", "Win", "Fn", "", "", ""}
};

const char* Keytype_right[Board_Height_Num][Board_Wight_Num] = {
    {"&", "*", "(", ")", "_", "+", "Back"},
    {"U", "I", "O", "P", "{", "}", "|"},
    {"J", "K", "L", ":", "\"", "-^-", "Ent"},
    {"M", ",", ".", "/", "<-", "_|_", "->"},
    {"Hide", "FChange", "", "Spa->", "", "", "Spa"}
};

uint32_t keymap_left[Board_Height_Num][Board_Wight_Num] = {
    {KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6},
    {KEY_GRAVE, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y},
    {KEY_TAB, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H},
    {KEY_LEFTSHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N},
    {KEY_LEFTCTRL, KEY_LEFTALT, KEY_LEFTMETA, KEY_FN, 0, 0, 0}
};

uint32_t keymap_right[Board_Height_Num][Board_Wight_Num] = {
    {KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE},
    {KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_BACKSLASH},
    {KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_UP, KEY_ENTER},
    {KEY_M, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_LEFT, KEY_DOWN, KEY_RIGHT},
    {0, 0, 0, KEY_SPACE, KEY_SPACE, KEY_SPACE, KEY_SPACE}
};

char ctrl_type = 0;
char alt_type = 0;
//这是use_uinput函数的再次封装
static inline void touch_key_event_win(int fd,int key_code){
    if(key_code){ /*使key_code等于0时跳过*/
        if(ctrl_type){
            touch_ctrl_key_event(fd,key_code);
            ctrl_type = 0;
        }else if(alt_type){
            touch_alt_key_event(fd,key_code);
            alt_type = 0;
        }else{
            touch_key_event(fd,key_code);
        }
    }
}

static inline void touch_shift_key_event_win(int fd, int key_code){
    if(key_code){ /*使key_code等于0时跳过*/
        if(ctrl_type){
            touch_ctrl_key_event(fd,key_code);
            ctrl_type = 0;
        }else if(alt_type){
            touch_alt_key_event(fd,key_code);
            alt_type = 0;
        }else{
            touch_shift_key_event(fd,key_code);
        }
    }
}

// 获取字体并创建图形上下文
static xcb_gc_t gc_font_get(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window, const char *font_name) {
    uint32_t value_list[3];
    xcb_void_cookie_t cookie_font;
    xcb_void_cookie_t cookie_gc;
    xcb_generic_error_t *error;
    xcb_font_t font;
    xcb_gcontext_t gc;
    uint32_t mask;

    font = xcb_generate_id(c);
    cookie_font = xcb_open_font_checked(c, font, strlen(font_name), font_name);
    error = xcb_request_check(c, cookie_font);
    if (error) {
        Error_Exit("can't open font", board_out);
    }

    gc = xcb_generate_id(c);
    mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
    value_list[0] = global_foreground_color;
    value_list[1] = global_background_color;
    value_list[2] = font;
    cookie_gc = xcb_create_gc_checked(c, gc, window, mask, value_list);
    error = xcb_request_check(c, cookie_gc);
    if (error) {
        Error_Exit("can't create gc", board_out);
    }

    cookie_font = xcb_close_font_checked(c, font);
    error = xcb_request_check(c, cookie_font);
    if (error) {
        Error_Exit("can't close font", board_out);
    }

    return gc;
}

// 绘制文本
static void text_draw(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window, int16_t x1, int16_t y1, const char *label) {
    xcb_void_cookie_t cookie_text;
    xcb_generic_error_t *error;
    xcb_gcontext_t gc;
    uint8_t length = strlen(label);

    gc = gc_font_get(c, screen, window, global_font_name);

    cookie_text = xcb_image_text_8_checked(c, length, window, gc, x1, y1, label);
    error = xcb_request_check(c, cookie_text);
    if (error) {
        Error_Exit("can't paste text", board_out);
    }

    xcb_free_gc(c, gc);
}

static void draw_button(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window,
    int16_t start_x, int16_t start_y, uint16_t width, uint16_t height, const char *label) {
    xcb_void_cookie_t cookie_rect;
    xcb_generic_error_t *error;

    // 获取一个图形上下文 (GC) 用来设置绘制属性
    xcb_gcontext_t gc = gc_font_get(c, screen, window, global_font_name);

    // 绘制矩形的边框（按钮的外边框）
    xcb_rectangle_t rect = {start_x, start_y, width, height};
    cookie_rect = xcb_poly_rectangle_checked(c, window, gc, 1, &rect); // 使用获取的GC绘制矩形边框
    error = xcb_request_check(c, cookie_rect);
    if (error) {
        Error_Exit("can't draw button rectangle border", board_out);
    }    
        // 使用 text_draw 来绘制按钮上的文字
    int text_x = start_x + width / 3;  // 计算文字位置
    int text_y = start_y + height / 2;
    text_draw(c, screen, window, text_x, text_y, label); // 调用 text_draw 来绘制文字

    // 释放图形上下文
    xcb_free_gc(c, gc);

    xcb_flush(c);
}

static void draw_frame(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window,
    int16_t start_x, int16_t start_y, uint16_t width, uint16_t height) {
    xcb_void_cookie_t cookie_rect;
    xcb_generic_error_t *error;
    
    // 获取一个图形上下文 (GC) 用来设置绘制属性
    xcb_gcontext_t gc = xcb_generate_id(c);
    uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_LINE_WIDTH;
    uint32_t values[] = {global_foreground_color, 1}; // 设置线条颜色为全局前景色，线宽为1
    
    xcb_create_gc(c, gc, window, mask, values);
    
    // 绘制矩形的边框（按钮的外边框）
    xcb_rectangle_t rect = {start_x, start_y, width, height};
    cookie_rect = xcb_poly_rectangle_checked(c, window, gc, 1, &rect); // 使用获取的GC绘制矩形边框
    error = xcb_request_check(c, cookie_rect);
    if (error) {
        Error_Exit("can't draw button rectangle border", board_out);
    }
 
    // 释放图形上下文
    xcb_free_gc(c, gc);

    xcb_flush(c);
}

static void Draw_Left_Board(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window, char* str[], uint16_t button_width, uint16_t button_height) {
    int start_x = 0;   // 从窗口左上角开始
    int start_y = 0;   // 从窗口左上角开始
    
    for (int row = 0; row < Board_Height_Num; row++) {
        for (int col = 0; col < Board_Wight_Num; col++) {
            // 确保按键不为空
            if (keytype_left[row][col][0] != '\0') {  
                // 使用 str[] 来设置按键标签
                draw_button(c, screen, window, 
                    start_x + col * (button_width),  // 按列排列
                    start_y + row * (button_height), // 按行排列
                    button_width, button_height, 
                    str[row * Board_Wight_Num + col]);  // 使用 str[] 中的标签
            }
        }
    }
}

static void Draw_Right_Board(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window, char* str[], uint16_t button_width, uint16_t button_height) {
    int start_x = 0;   // 右侧键盘窗口从 (0,0) 开始
    int start_y = 0;   // 右侧键盘窗口从 (0,0) 开始
    
    for (int row = 0; row < Board_Height_Num; row++) {
        for (int col = 0; col < Board_Wight_Num; col++) {
            // 确保按键不为空
            if (keytype_right[row][col][0] != '\0') {  
                // 使用 str[] 来设置按键标签
                draw_button(c, screen, window, 
                    start_x + col * (button_width),  // 按列排列
                    start_y + row * (button_height), // 按行排列
                    button_width, button_height, 
                    str[row * Board_Wight_Num + col]);  // 使用 str[] 中的标签
            }
        }
    }
}

void Board_Main(){
    ufd = open_uinput_device();
    setup_uinput_keyboard(ufd);
    xcb_screen_t *screen;
    int screen_nbr = 0;
    conn = xcb_connect(NULL, &screen_nbr);
    if (xcb_connection_has_error(conn)) {
        Error_Exit("can't connect to X server", NULL);
    }

    screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
    if (!screen) {
        Error_Exit("can't get the current screen", board_out);
    }

    setup_signal_handler();
    int screen_width = screen->width_in_pixels;
    int screen_height = screen->height_in_pixels;

    global_foreground_color = screen->white_pixel;
    global_background_color = screen->black_pixel;

    uint16_t left_board_width = screen_width/ Width_Cut_Num * Left_Board_Num;                   /*宽度*/
    uint16_t left_board_height = screen_height / Height_Cut_Num * Down_Board_Num;               /*高*/
    uint16_t left_board_x = 0;                                                                  /*屏幕边缘*/
    uint16_t left_board_y = screen_height / Height_Cut_Num *(Height_Cut_Num - Down_Board_Num);  /*屏幕高start*/
    
    uint16_t right_board_width = left_board_width;            /*宽度同上*/
    uint16_t right_board_height = left_board_height;                          /*高度同上*/
    uint16_t right_board_x = screen_width - right_board_width;
    uint16_t right_board_y = left_board_y;

    uint32_t window_border_width = 0;
    uint32_t window_class = XCB_WINDOW_CLASS_INPUT_OUTPUT;
    uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[] = {global_background_color, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE};
    // 创建窗口
    xcb_window_t left_board = xcb_generate_id(conn);
    xcb_window_t right_board = xcb_generate_id(conn);
    xcb_window_t Left_board = xcb_generate_id(conn);
    xcb_window_t Right_board = xcb_generate_id(conn);

    xcb_create_window(conn, XCB_COPY_FROM_PARENT, left_board, screen->root,
        left_board_x, left_board_y, left_board_width, left_board_height,
        window_border_width, window_class, screen->root_visual,
        value_mask, values);

    xcb_create_window(conn, XCB_COPY_FROM_PARENT, right_board, screen->root,
        right_board_x, right_board_y, right_board_width, right_board_height,
        window_border_width, window_class, screen->root_visual,
        value_mask, values);

    xcb_create_window(conn, XCB_COPY_FROM_PARENT, Left_board, screen->root,
        left_board_x, left_board_y, left_board_width, left_board_height,
        window_border_width, window_class, screen->root_visual,
        value_mask, values);
    
    xcb_create_window(conn, XCB_COPY_FROM_PARENT, Right_board, screen->root,
        right_board_x, right_board_y, right_board_width, right_board_height,
        window_border_width, window_class, screen->root_visual,
        value_mask, values);
    
    uint32_t override_redirect = 1;
    xcb_change_window_attributes(conn, left_board, XCB_CW_OVERRIDE_REDIRECT, &override_redirect);
    xcb_change_window_attributes(conn, right_board, XCB_CW_OVERRIDE_REDIRECT, &override_redirect);
    xcb_change_window_attributes(conn, Left_board, XCB_CW_OVERRIDE_REDIRECT, &override_redirect);
    xcb_change_window_attributes(conn, Right_board, XCB_CW_OVERRIDE_REDIRECT, &override_redirect);
    // 计算按钮的宽度和高度
    uint16_t button_width = left_board_width / Board_Wight_Num;
    uint16_t button_height = left_board_height / Board_Height_Num;
    
    /*
    // 修正高度，使其填满整个窗口
    uint16_t height_remainder = left_board_height % Board_Height_Num;
    if (height_remainder > 0) {
        button_height += height_remainder / Board_Height_Num;  // 均摊多余高度
    }
    // 宽度补偿
    uint16_t width_remainder = left_board_width % Board_Wight_Num;
    if (width_remainder > 0) {
        button_width += width_remainder / Board_Wight_Num;  // 均摊剩余宽度
    }
    */

    Draw_Left_Board(conn, screen, left_board, (char**)keytype_left, button_width, button_height);
    Draw_Right_Board(conn, screen, right_board, (char**)keytype_right, button_width, button_height);
    Draw_Left_Board(conn, screen, Left_board, (char**)Keytype_left, button_width, button_height);
    Draw_Right_Board(conn, screen, Right_board, (char**)Keytype_right, button_width, button_height);
    
    xcb_map_window(conn, left_board);
    xcb_map_window(conn, right_board);
    
    xcb_flush(conn);
    xcb_generic_event_t *event;
    xcb_button_press_event_t *button_event;
    while ((event = xcb_wait_for_event(conn))) {//阻塞，所以这个事件获取无法实现长按，长按需要另外实现
        switch (event->response_type & ~0x80){
            case XCB_EXPOSE:
            Draw_Left_Board(conn, screen, left_board, (char**)keytype_left, button_width, button_height);
            Draw_Right_Board(conn, screen, right_board, (char**)keytype_right, button_width, button_height);
            Draw_Left_Board(conn, screen, Left_board, (char**)Keytype_left, button_width, button_height);
            Draw_Right_Board(conn, screen, Right_board, (char**)Keytype_right, button_width, button_height);
            xcb_flush(conn);
            break;
            case XCB_BUTTON_PRESS:
            button_event = (xcb_button_press_event_t *)event;
            int16_t x = button_event->event_x;
            int16_t y = button_event->event_y;
            uint32_t key_numx = x / button_width;
            uint32_t key_numy = y / button_height;

            if (button_event->event == left_board){
                if(key_numx == Shift_Key_x && key_numy == Shift_Key_y){/*触摸到shift*/
                    xcb_map_window(conn, Left_board);
                    xcb_map_window(conn, Right_board);
                    xcb_flush(conn);
                }else if(key_numx == Ctrl_Key_x && key_numy == Ctrl_Key_y){
                    ctrl_type = !ctrl_type;
                }else if(key_numx == Alt_Key_x && key_numy == Alt_Key_y){
                    alt_type = !alt_type;
                }else{
                    if (key_numx < Board_Wight_Num && key_numy < Board_Height_Num) {
                        touch_key_event_win(ufd, keymap_left[key_numy][key_numx]);
                    }
                }
            }else if(button_event->event == right_board){
                if (key_numx < Board_Wight_Num && key_numy < Board_Height_Num) {
                    touch_key_event_win(ufd, keymap_right[key_numy][key_numx]);
                }
            }else if(button_event->event == Left_board){
                if(key_numx == Shift_Key_x && key_numy == Shift_Key_y){/*触摸到shift*/
                    xcb_unmap_window(conn, Left_board);
                    xcb_unmap_window(conn, Right_board);

                    xcb_flush(conn);
                }else if(key_numx == Ctrl_Key_x && key_numy == Ctrl_Key_y){
                    ctrl_type = !ctrl_type;
                }else if(key_numx == Alt_Key_x && key_numy == Alt_Key_y){
                    alt_type = !alt_type;
                }else{
                    if (key_numx < Board_Wight_Num && key_numy < Board_Height_Num) {
                        touch_shift_key_event_win(ufd, keymap_left[key_numy][key_numx]);
                    }
                }
            }else if(button_event->event == Right_board){
                if (key_numx < Board_Wight_Num && key_numy < Board_Height_Num) {
                    touch_shift_key_event_win(ufd, keymap_right[key_numy][key_numx]);
                }
            }else{
                
            }
        }
    }
    board_out(0);
}