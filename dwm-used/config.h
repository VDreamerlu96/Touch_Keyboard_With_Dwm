/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "monospace:size=10" };
static const char dmenufont[]       = "monospace:size=10";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ NULL,       NULL,       NULL,       0,            False,       -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "qterminal", NULL };

static const Key keys[] = {
    /* modifier                     key        function        argument */
    { MODKEY,                       XK_Return, spawn,          {.v = termcmd } }, // mod + Enter 打开终端模拟器
    { MODKEY|ShiftMask,             XK_Return, zoom,           {0} }, // mod + Shift + Enter 使焦点窗口占满屏幕
    { MODKEY,                       XK_BackSpace, killclient,  {0} }, // mod + BackSpzce 关闭终端模拟器
    { MODKEY,                       XK_m,      spawn,          {.v = dmenucmd } }, // mod + m 打开应用菜单
    { MODKEY,                       XK_j,      focusstack,     {.i = +1 } },// mod + l 移动焦点到下一个窗口
    { MODKEY,                       XK_h,      focusstack,     {.i = -1 } },
    { MODKEY,                       XK_d,      setmfact,       {.f = +0.05} }, // mod + a 增加主窗口大小
    { MODKEY,                       XK_a,      setmfact,       {.f = -0.05} }, // mod + d 减少主窗口大小
    TAGKEYS(                        XK_1,                      0) // mod + 1 切换到标签 1
    TAGKEYS(                        XK_2,                      1)
    TAGKEYS(                        XK_3,                      2)
    TAGKEYS(                        XK_4,                      3)
    TAGKEYS(                        XK_5,                      4)
    TAGKEYS(                        XK_6,                      5)
    TAGKEYS(                        XK_7,                      6)
    TAGKEYS(                        XK_8,                      7)
    TAGKEYS(                        XK_9,                      8)
    { MODKEY|ShiftMask,             XK_q,      quit,           {0} }, // mod + shift + q 退出 dwm
    { MODKEY|ShiftMask,             XK_comma,  focusmon,       {.i = -1 } }, // mod + Shift + <, 切换到左侧显示器
    { MODKEY|ShiftMask,             XK_period, focusmon,       {.i = +1 } }, // mod + Shift + > 切换到右侧显示器
    { MODKEY,                       XK_comma,  tagmon,         {.i = -1 } }, // mod + <, 将标签切换到左侧显示器
    { MODKEY,                       XK_period, tagmon,         {.i = +1 } }, // mod + > 将标签切换到右侧显示器
    { MODKEY,                       XK_c,      incnmaster,     {.i = +1 } }, // mod + c 增加主窗口数量
    { MODKEY,                       XK_z,      incnmaster,     {.i = -1 } }, // mod + z 减少主窗口数量
    { MODKEY,                       XK_t,      togglebar,      {0} },        // mod + t切换状态栏的显示与隐藏
    { MODKEY|ShiftMask,               XK_1,      tag,            {.ui = 1 << 0} },
    { MODKEY|ShiftMask,               XK_2,      tag,            {.ui = 1 << 1} },
    { MODKEY|ShiftMask,               XK_3,      tag,            {.ui = 1 << 2} },
    { MODKEY|ShiftMask,               XK_4,      tag,            {.ui = 1 << 3} },
    { MODKEY|ShiftMask,               XK_5,      tag,            {.ui = 1 << 4} },
    { MODKEY|ShiftMask,               XK_6,      tag,            {.ui = 1 << 5} },
    { MODKEY|ShiftMask,               XK_7,      tag,            {.ui = 1 << 6} },
    { MODKEY|ShiftMask,               XK_8,      tag,            {.ui = 1 << 7} },
    { MODKEY|ShiftMask,               XK_9,      tag,            {.ui = 1 << 8} },
    { MODKEY, 			      XK_p, togglelayout, {.v = &layouts[0]} }, 	/* 当前窗口 -> 平铺否则全局平铺 */
    { MODKEY, 			      XK_o, togglelayout, {.v = &layouts[2]} }, 	/* 当前窗口 -> 全屏 */
    { MODKEY|ShiftMask,             XK_f,  togglefloating, {0} },                    // 切换浮动窗口
    { MODKEY,                       XK_0,      view,           {.ui = ~0 } },        // 选择所有标签
    { MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },        // 移动窗口到所有标签
    { MODKEY,            XK_g,      Change_key_or_full,   {0} },                     //
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

