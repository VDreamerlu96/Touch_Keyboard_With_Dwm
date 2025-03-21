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

/*xlsfonts or fc-list look*/
const char* global_font_name = "9x15";         /*字体,系统要有，不然会打开失败，而且最好根据当前屏幕选一个合适大小的*/

static uint32_t global_foreground_color = 0;    /*前景色*/
static uint32_t global_background_color = 0;    /*背景色*/

static char* Left_Key_Def[] = {"esc","caps","tab","space","lcrtl","lshift","lalt","ent","bspace"};                        /*9个*/
static char* Left_Key_A[] = {"=","-","/","\\","'","[","]","ent",";"};
static char* Left_Key_B[] = {"+","_","\"",0,0,0,0,0,0};
static char* Left_Key_C[] = {"*","&","|","!","~","{","}","(",")"};
 
static char* Right_Key_Def[] = {"a","b","c","d","e","f","g","h","i","j"                                        /*10个*/
,"k","l","m","n","del","bspace","o","p","q"                                                                                /*9个*/
};
static char* Right_Key_A[] = {"r","s","t","u","v","w","x","y","z","' / \""
,", / <",". / >","/ / ?","; / :","[ / {"," ] / }","\\ / |","- / _","= / +"
};
static char* Right_Key_B[] = {"1","2","3","4","5","left","6","7","8","9"
,"0",0,0,0,"up","right","down",0,0,
};
static char* Right_Key_C[] = {"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11"
,"F12",0,0,0,0,0,0,0,0
};

#define LEFT_KEY_LEN 9          /*左键盘数组长度*/
#define RIGHT_KEY_LEN_A 10      /*右键盘数组第一列长度*/
#define RIGHT_KEY_LEN_B 9       /*右键盘数组第二列长度*/

uint32_t Left_Key_Def_Var[] = {
    KEY_ESC,     // esc
    KEY_CAPSLOCK,  // caps
    KEY_TAB,     // tab
    KEY_SPACE,   // space
    KEY_LEFTCTRL,// lcrtl
    KEY_LEFTSHIFT, // lshift
    KEY_LEFTALT, // lalt
    KEY_ENTER,   // ent
    KEY_BACKSPACE
};

uint32_t Left_Key_A_Var[] = {
    KEY_EQUAL,     // =
    KEY_MINUS,     // -
    KEY_SLASH,     // /
    KEY_BACKSLASH, 
    KEY_APOSTROPHE,// '
    KEY_LEFTBRACE, // [
    KEY_RIGHTBRACE,// ]
    KEY_ENTER,     // ent
    KEY_SEMICOLON  // ;
};

uint32_t Left_Key_B_Var[] = {
    KEY_KPPLUS,  // +（可能需要Shift）
    KEY_MINUS,   // -
    KEY_APOSTROPHE, // "（需要Shift）
    0,
    0,
    0,
    0,
    0,
    0
};

uint32_t Left_Key_C_Var[] = {
    KEY_8,        // * 需要Shift
    KEY_7,        // & 需要Shift
    KEY_BACKSLASH,// | 需要Shift
    KEY_1,        // !
    KEY_GRAVE,    // ~ 需要Shift
    KEY_LEFTBRACE, // {
    KEY_RIGHTBRACE,// }
    KEY_9,        // (
    KEY_0         // )
};

uint32_t Right_Key_Def_Var[] = {
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
    KEY_K, KEY_L, KEY_M, KEY_N, KEY_DELETE, KEY_BACKSPACE, KEY_O, KEY_P, KEY_Q
};

uint32_t Right_Key_A_Var[] = {
    KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_APOSTROPHE,  // ' / "
    KEY_COMMA,  // , / <
    KEY_DOT,    // . / >
    KEY_SLASH,  // / / ?
    KEY_SEMICOLON,  // ; / :
    KEY_LEFTBRACE,  // [ / {
    KEY_RIGHTBRACE, // ] / }
    KEY_BACKSLASH,  // \ / |
    KEY_MINUS   // - / _
};

uint32_t Right_Key_B_Var[] = {
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_LEFT, KEY_6, KEY_7, KEY_8, KEY_9,
    KEY_0, 0, 0, 0, KEY_UP, KEY_RIGHT, KEY_DOWN, 0, 0
};

uint32_t Right_Key_C_Var[] = {
    KEY_F1,  // F1
    KEY_F2,  // F2
    KEY_F3,  // F3
    KEY_F4,  // F4
    KEY_F5,  // F5
    KEY_F6,  // F6
    KEY_F7,  // F7
    KEY_F8,  // F8
    KEY_F9,  // F9
    KEY_F10, // F10
    KEY_F11, // F11
    KEY_F12, // F12
    0, 0, 0, 0, 0, 0, 0, 0
};

static char* left_super = "[~_~]--";
static char* right_super = "--[~^~]";
static char* hide_super = "[*^*]";

static char* left_super_type[] = {"L_Hide","L_C","L_B","L_A","L_Def"};
static char* right_super_type[] = {"R_C","R_B","R_A","R_Def"};
#define LEFT_SUPER_LEN 5
#define RIGHT_SUPER_LEN 4

// 函数声明
static void board_out(int);
static xcb_gc_t gc_font_get(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window, const char *font_name);
static void text_draw(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window, int16_t x1, int16_t y1, const char *label);
static void draw_button(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window,
        int16_t start_x, int16_t start_y, uint16_t width, uint16_t height, const char *label);
static void draw_frame(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window,
        int16_t start_x, int16_t start_y, uint16_t width, uint16_t height);
static void Draw_Left_Window(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window,char* str[],char stl,uint16_t button_width,uint16_t button_height);
static void Draw_Right_Window(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window,char* str[],char stla,char stlb,uint16_t button_width,uint16_t button_height);

//这是use_uinput函数的再次封装
static inline void touch_key_event_win(int fd,int key_code){
    if(key_code){ /*使key_code等于0时跳过*/
        touch_key_event(fd,key_code);
    }
}

static inline void touch_shift_key_event_win(int fd, int key_code){
    if(key_code){ /*使key_code等于0时跳过*/
        touch_shift_key_event(fd,key_code);
    }
}

void touch_longpress_key_event_win(int fd, uint32_t key_code) {
    if(key_code){ /*使key_code等于0时跳过*/
        send_key_event(fd,key_code,1);
        usleep(300000); // 300ms 延迟
        send_key_event(fd,key_code,0);
    }
}

typedef struct {
    int is_pressed;
    struct timespec press_time;
    uint32_t key_code;
} KeyPressState;

KeyPressState key_state = {0, {0, 0}, 0};
#define LONG_PRESS_THRESHOLD 500 // 长按时间（毫秒）

static char is_shift_press = 0;         /*依托*/
static inline void touch_shift(int fd){
    if(is_shift_press){
        send_key_event(fd,KEY_LEFTSHIFT,0);
        is_shift_press = 0;
    }else{
        send_key_event(fd,KEY_LEFTSHIFT,1);
        is_shift_press = 1;
    }
}

static char is_ctrl_press = 0;         /*依托*/
static inline void touch_ctrl(int fd){
    if(is_ctrl_press){
        send_key_event(fd,KEY_LEFTCTRL,0);
        is_shift_press = 0;
    }else{
        send_key_event(fd,KEY_LEFTCTRL,1);
        is_ctrl_press = 1;
    }
}

static char is_alt_press = 0;         /*依托*/
static inline void touch_alt(int fd){
    if(is_alt_press){
        send_key_event(fd,KEY_LEFTALT,0);
        is_shift_press = 0;
    }else{
        send_key_event(fd,KEY_LEFTALT,1);
        is_alt_press = 1;
    }
}

// 全局连接变量
static xcb_connection_t *conn = NULL;
static int ufd = 0;
// 清理函数
static void board_out(int) {
    if (conn) {
        xcb_disconnect(conn);
        conn = NULL;
    }
    if(ufd){
        destroy_uinput_device(ufd);
        ufd = 0;
    }
}

static void setup_signal_handler(void) {
    signal(SIGINT, board_out);
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
    int text_x = start_x + width / 2;  // 计算文字位置
    int text_y = start_y + height / 4*3;
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


static void Draw_Left_Window(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window,char* str[],char stl,uint16_t button_width,uint16_t button_height){
    // 起始坐标，左上角位置
    int16_t start_x = 0;  // x 坐标
    int16_t start_y = 0;  // y 坐标，开始绘制的纵向位置

    int y_pos = 0;
    for (char i = 0; i < stl; i++) {
        // 计算每个按钮的纵向位置
        y_pos = start_y + (i * button_height);
        
        if (str[i] == NULL || strlen(str[i]) == 0) {
            draw_frame(c,screen,window,start_x,y_pos,button_width,button_height);
        }else{
        // 绘制按钮
            draw_button(c, screen, window, start_x, y_pos, button_width, button_height, str[i]);
        }
    }
        y_pos = y_pos + button_height;
        draw_button(c, screen, window, start_x, y_pos, button_width, button_height, left_super);
    xcb_flush(c);  // 刷新显示，确保绘制的内容显示出来
}

static void Draw_Right_Window(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window,char* str[],char stla,char stlb,uint16_t button_width,uint16_t button_height){
    // 起始坐标，左上角位置
    int16_t start_x = 0;  // x 坐标
    int16_t start_y = 0;  // y 坐标，开始绘制的纵向位置

    int y_pos;
    for (char i = 0; i < stla; i++) {
        // 计算每个按钮的纵向位置
        y_pos = start_y + (i * button_height);
        if (str[i] == NULL || strlen(str[i]) == 0) {
            draw_frame(c,screen,window,start_x,y_pos,button_width,button_height);
        }else{
        // 绘制按钮
            draw_button(c, screen, window, start_x, y_pos, button_width, button_height, str[i]);
        }
    }
    start_x = 0 + button_width;
    start_y = 0;
    y_pos = 0;
    for (char i = 0; i < stlb; i++) {
        y_pos = start_y + (i * button_height);
        if (str[i+stla] == NULL || strlen(str[i+stla]) == 0) {
            draw_frame(c,screen,window,start_x,y_pos,button_width,button_height);
        }else{
        // 绘制按钮
            draw_button(c, screen, window, start_x, y_pos, button_width, button_height, str[i+stla]);
        }
    }
    y_pos = y_pos + button_height;
    draw_button(c, screen, window, start_x, y_pos, button_width, button_height, right_super);
    xcb_flush(c);  // 刷新显示，确保绘制的内容显示出来
}


static void Draw_Left_Super(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window,char* str[],char stl,uint16_t button_width,uint16_t button_height){
    int16_t start_x = 0;  // x 坐标
    int16_t start_y = 0;  // y 坐标，开始绘制的纵向位置
    int y_pos = 0;
    for(char i = 0;i < stl;i++){
        y_pos = start_y + (i * button_height);
        draw_button(c, screen, window, start_x, y_pos, button_width, button_height, str[i]);
    }
}

static void Draw_Right_Super(xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window,char* str[],char stl,uint16_t button_width,uint16_t button_height){
    int16_t start_x = 0;  // x 坐标
    int16_t start_y = 0;  // y 坐标，开始绘制的纵向位置
    int y_pos = 0;
    for(char i = 0;i < stl;i++){
        y_pos = start_y + (i * button_height);
        draw_button(c, screen, window, start_x, y_pos, button_width, button_height, str[i]);
    }
}

void Board_Main() {
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

    // 创建窗口
    xcb_window_t left_board = xcb_generate_id(conn);
    xcb_window_t right_board = xcb_generate_id(conn);
    xcb_window_t left_super = xcb_generate_id(conn);
    xcb_window_t right_super = xcb_generate_id(conn);
    xcb_window_t hide_win = xcb_generate_id(conn);

    uint16_t left_board_width = screen_width/ Width_Cut_Num * Left_Board_Num;   /*1/10宽度*/
    uint16_t left_board_height = screen_height;                                 /*3/4高*/
    uint16_t left_board_x = 0;                                                  /*屏幕边缘*/
    uint16_t left_board_y = Board_Start_Height;                                                  /*屏幕高start*/
    
    uint16_t right_board_width = left_board_width*Right_Board_Num;            /*2/10宽度*/
    uint16_t right_board_height = left_board_height;                          /*高度同上*/
    uint16_t right_board_x = screen_width - right_board_width;  
    uint16_t right_board_y = left_board_y;

    uint16_t button_height = left_board_height/10;
    uint16_t button_width = left_board_width - 2;

    uint16_t left_super_width = left_board_width;
    uint16_t left_super_height = button_height * 5;
    uint16_t left_super_x = right_board_x - left_board_width;
    uint16_t left_super_y = left_board_y + button_height * 5;

    uint16_t right_super_width = left_board_width;
    uint16_t right_super_height = button_height * 4;
    uint16_t right_super_x = left_board_width;
    uint16_t right_super_y = left_board_y + button_height * 6;
    
    uint16_t hide_width = button_width;
    uint16_t hide_height = button_height;
    uint16_t hide_x = screen_width - button_width;
    uint16_t hide_y = screen_height - button_height;

    uint32_t window_border_width = 0;
    uint32_t window_class = XCB_WINDOW_CLASS_INPUT_OUTPUT;
    uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[] = {global_background_color, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE};
    
    xcb_create_window(conn, XCB_COPY_FROM_PARENT, left_board, screen->root,
                      left_board_x, left_board_y, left_board_width, left_board_height,
                      window_border_width, window_class, screen->root_visual,
                      value_mask, values);

    xcb_create_window(conn, XCB_COPY_FROM_PARENT, right_board, screen->root,
                      right_board_x, right_board_y, right_board_width, right_board_height,
                      window_border_width, window_class, screen->root_visual,
                      value_mask, values);
    xcb_create_window(conn, XCB_COPY_FROM_PARENT, left_super, screen->root,
                        left_super_x, left_super_y, left_super_width, left_super_height,
                        window_border_width, window_class, screen->root_visual,
                        value_mask, values);
  
    xcb_create_window(conn, XCB_COPY_FROM_PARENT, right_super, screen->root,
                        right_super_x, right_super_y, right_super_width, right_super_height,
                        window_border_width, window_class, screen->root_visual,
                        value_mask, values);
    
    xcb_create_window(conn, XCB_COPY_FROM_PARENT, hide_win, screen->root,
                        hide_x, hide_y, hide_width, hide_height,
                        window_border_width, window_class, screen->root_visual,
                        value_mask, values);
    // 让窗口管理器忽略这个窗口

    uint32_t override_redirect = 1;
    xcb_change_window_attributes(conn, left_board, XCB_CW_OVERRIDE_REDIRECT, &override_redirect);
    xcb_change_window_attributes(conn, right_board, XCB_CW_OVERRIDE_REDIRECT, &override_redirect);
    xcb_change_window_attributes(conn, left_super, XCB_CW_OVERRIDE_REDIRECT, &override_redirect);
    xcb_change_window_attributes(conn, right_super, XCB_CW_OVERRIDE_REDIRECT, &override_redirect);
    xcb_change_window_attributes(conn, hide_win, XCB_CW_OVERRIDE_REDIRECT, &override_redirect);

    xcb_map_window(conn, left_board);
    xcb_map_window(conn, right_board);

    char if_left_super_is = 0;
    char if_right_super_is = 0;
    char** left_now = Left_Key_Def;     //存储当前键盘布局
    char** right_now = Right_Key_Def;
    uint32_t* left_key_var_now = Left_Key_Def_Var;
    uint32_t* right_key_var_now = Right_Key_Def_Var;

    Draw_Left_Window(conn,screen,left_board,left_now,LEFT_KEY_LEN,button_width,button_height);
    Draw_Right_Window(conn,screen,right_board,right_now,RIGHT_KEY_LEN_A,RIGHT_KEY_LEN_B,button_width,button_height);
    // 事件循环
    xcb_generic_event_t *event;
    while ((event = xcb_wait_for_event(conn))) {
        switch (event->response_type & ~0x80) {
            case XCB_EXPOSE:
                Draw_Left_Window(conn,screen,left_board,left_now,LEFT_KEY_LEN,button_width,button_height);
                Draw_Right_Window(conn,screen,right_board,right_now,RIGHT_KEY_LEN_A,RIGHT_KEY_LEN_B,button_width,button_height);
                xcb_flush(conn);
                break;
            case XCB_BUTTON_PRESS: 
                xcb_button_press_event_t *button_event = (xcb_button_press_event_t *)event;
                int16_t x = button_event->event_x;
                int16_t y = button_event->event_y;
                clock_gettime(CLOCK_MONOTONIC, &key_state.press_time);
                key_state.is_pressed = 1;
                if (button_event->event == left_board) {
                    uint32_t kry_num = y / button_height;
                    if(kry_num >= LEFT_KEY_LEN){    /*触摸到left super键*/
                        if(if_left_super_is){
                            xcb_unmap_window(conn, left_super);
                            xcb_flush(conn);
                            if_left_super_is = 0;
                        }else{
                            xcb_map_window(conn, left_super);
                            Draw_Left_Super(conn,screen,left_super,left_super_type,LEFT_SUPER_LEN,button_width,button_height);
                            xcb_flush(conn);
                            if_left_super_is = 1;
                        }
                    }else{
                        if(left_key_var_now == Left_Key_C_Var){
                            touch_shift_key_event_win(ufd,left_key_var_now[kry_num]);
                        }else if(left_key_var_now == Left_Key_B_Var){
                            touch_shift_key_event_win(ufd,left_key_var_now[kry_num]);
                        }else if(left_key_var_now == Left_Key_Def_Var){
                            if( kry_num == 5){
                                touch_shift(ufd);
                            }else if(kry_num == 4){
                                touch_ctrl(ufd);
                            }else if(kry_num == 6){
                                touch_alt(ufd);
                            }else{
                                touch_key_event_win(ufd,left_key_var_now[kry_num]);
                                key_state.key_code = left_key_var_now[kry_num];
                            }
                        }else{
                            touch_key_event_win(ufd,left_key_var_now[kry_num]);
                            key_state.key_code = left_key_var_now[kry_num];
                        }
                    }
                }else if(button_event->event == right_board){
                    uint32_t kry_numa = x/button_width;
                    uint32_t kry_num = 0;
                    if(kry_numa){
                        kry_num = y/button_height + RIGHT_KEY_LEN_A;
                    }else{
                        kry_num = y/button_height;
                    }
                    if(kry_num >= RIGHT_KEY_LEN_A + RIGHT_KEY_LEN_B){   /*触摸到right super键*/
                        if(if_right_super_is){
                            xcb_unmap_window(conn, right_super);
                            xcb_flush(conn);
                            if_right_super_is = 0;
                        }else{
                            xcb_map_window(conn, right_super);
                            Draw_Right_Super(conn,screen,right_super,right_super_type,RIGHT_SUPER_LEN,button_width,button_height);
                            xcb_flush(conn);
                            if_right_super_is = 1;
                        }
                    }else{
                        touch_key_event_win(ufd,right_key_var_now[kry_num]);
                        key_state.key_code = left_key_var_now[kry_num];
                    }
                }else if(button_event->event == left_super){
                    uint32_t kry_num = y / button_height;
                    switch(kry_num){
                        case 0:
                            xcb_unmap_window(conn,left_board);
                            xcb_unmap_window(conn,right_board);
                            xcb_unmap_window(conn,left_super);
                            xcb_unmap_window(conn,right_super);
                            xcb_map_window(conn,hide_win);
                            draw_button(conn,screen,hide_win,0,0,button_width,button_height,hide_super);
                            xcb_flush(conn);
                        break;
                        case 1:
                            left_now = Left_Key_C;
                            left_key_var_now = Left_Key_C_Var;
                            xcb_clear_area(conn, 1, left_board, 0, 0, 0, 0);
                            Draw_Left_Window(conn,screen,left_board,left_now,LEFT_KEY_LEN,button_width,button_height);
                        break;
                        case 2:
                            left_now = Left_Key_B;
                            left_key_var_now = Left_Key_B_Var;
                            xcb_clear_area(conn, 1, left_board, 0, 0, 0, 0);
                            Draw_Left_Window(conn,screen,left_board,left_now,LEFT_KEY_LEN,button_width,button_height);
                        break;
                        case 3:
                            left_now = Left_Key_A;
                            left_key_var_now = Left_Key_A_Var;
                            xcb_clear_area(conn, 1, left_board, 0, 0, 0, 0);
                            Draw_Left_Window(conn,screen,left_board,left_now,LEFT_KEY_LEN,button_width,button_height);
                        break;
                        case 4:
                            left_now = Left_Key_Def;
                            left_key_var_now = Left_Key_Def_Var;
                            xcb_clear_area(conn, 1, left_board, 0, 0, 0, 0);
                            Draw_Left_Window(conn,screen,left_board,left_now,LEFT_KEY_LEN,button_width,button_height);
                        break;
                    }
                }else if(button_event->event == right_super){
                    uint32_t kry_num = y / button_height;
                    switch(kry_num){
                        case 0:
                            right_now = Right_Key_C;
                            right_key_var_now = Right_Key_C_Var;
                            xcb_clear_area(conn, 1, right_board, 0, 0, 0, 0);
                            Draw_Right_Window(conn,screen,right_board,right_now,RIGHT_KEY_LEN_A,RIGHT_KEY_LEN_B,button_width,button_height);
                        break;
                        case 1:
                            right_now = Right_Key_B;
                            right_key_var_now = Right_Key_B_Var;
                            xcb_clear_area(conn, 1, right_board, 0, 0, 0, 0);
                            Draw_Right_Window(conn,screen,right_board,right_now,RIGHT_KEY_LEN_A,RIGHT_KEY_LEN_B,button_width,button_height);
                        break;
                        case 2:
                            right_now = Right_Key_A;
                            right_key_var_now = Right_Key_A_Var;
                            xcb_clear_area(conn, 1, right_board, 0, 0, 0, 0);
                            Draw_Right_Window(conn,screen,right_board,right_now,RIGHT_KEY_LEN_A,RIGHT_KEY_LEN_B,button_width,button_height);
                        break;
                        case 3:
                            right_now = Right_Key_Def;
                            right_key_var_now = Right_Key_Def_Var;
                            xcb_clear_area(conn, 1, right_board, 0, 0, 0, 0);
                            Draw_Right_Window(conn,screen,right_board,right_now,RIGHT_KEY_LEN_A,RIGHT_KEY_LEN_B,button_width,button_height);
                        break;
                    }
                }else if(button_event->event == hide_win){
                    xcb_map_window(conn, left_board);
                    xcb_map_window(conn, right_board);
                    Draw_Left_Window(conn,screen,left_board,left_now,LEFT_KEY_LEN,button_width,button_height);
                    Draw_Right_Window(conn,screen,right_board,right_now,RIGHT_KEY_LEN_A,RIGHT_KEY_LEN_B,button_width,button_height);
                    xcb_unmap_window(conn,hide_win);
                    xcb_flush(conn);
                }
            default:
                break;
        }
    }
    // 清理
    board_out(0);
}


