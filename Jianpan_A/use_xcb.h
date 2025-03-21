#pragma once
#include <xcb/xcb.h>

extern void Start_Xconnect(xcb_connection_t **xc, int *screen_nbr, const char *display_name);
extern void Get_Screen_Info(xcb_connection_t *conn, xcb_screen_t **screen_info, int screen_nbr);
extern void Stop_Xconnect(xcb_connection_t *xc);
