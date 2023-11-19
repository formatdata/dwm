/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int gappx     = 5;        /* gaps between windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const int focusonwheel       = 0;
static const char *fonts[]          = { "Iosevka Nerd Font:style=Bold:size=10" };
static const char dmenufont[]       = "Iosevka Nerd Font:style=Bold:size=10";
static const char col_gray1[]       = "#161616";
static const char col_gray2[]       = "#393939";
static const char col_gray3[]       = "#f2f4f8";
static const char col_gray4[]       = "#161616";
static const char col_cyan[]        = "#78a9ff";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_cyan, col_gray2,  col_cyan  },
	[SchemeTitle]  = { col_gray3, col_gray1,  col_gray2  },
};
static const XPoint stickyicon[]    = { {0,0}, {4,0}, {4,8}, {2,6}, {0,8}, {0,0} }; /* represents the icon as an array of vertices */
static const XPoint stickyiconbb    = {4,8};	/* defines the bottom right corner of the polygon's bounding box (speeds up scaling) */

typedef struct {
	const char *name;
	const void *cmd;
} Sp;
const char *spcmd1[] = {"st", "-n", "spterm", "-g", "120x34", NULL };
const char *spcmd2[] = {"st", "-n", "spfm", "-g", "120x34", "-e", "lfub", NULL };
const char *spcmd3[] = {"st", "-n", "spncm", "-g", "90x20", "-e", "ncmpcpp", NULL };
static Sp scratchpads[] = {
	/* name          cmd  */
	{"spterm",      spcmd1},
	{"splf",    spcmd2},
	{"spncm",   spcmd3},
};

/* tagging */
static const char *tags[] = { "一", "二", "三", "四", "五", "六", "七", "八", "九" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title            tags mask  isfloating  isterminal  noswallow  monitor */
	{ "Gimp",     NULL,       NULL,            0,         1,          0,           0,        -1 },
	{ "St",       NULL,       NULL,            0,         0,          1,           0,        -1 },
	{ NULL,       NULL,       "Event Tester",  0,         0,          0,           1,        -1 }, /* xev */
	{ NULL,		    "spterm",		NULL,		         SPTAG(0),	1,          0,           0, 			 -1 },
	{ NULL,		    "spfm", 		NULL,		         SPTAG(1),	1,          0,           0, 			 -1 },
	{ NULL,		    "spncm",    NULL,		         SPTAG(2),	1,          0,           0, 			 -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "󰖲",      tile },    /* first entry is default */
	{ "󰖰",      NULL },    /* no layout function means floating behavior */
	{ "󰖯",      monocle },
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
static const char *dmenucmd[] = { "dmenu-dwm", NULL };
static const char *termcmd[]  = { "st", NULL };
static const char *clipmenu[] = { "dmenu-clipmenu", NULL };
static const char *upvol[]    = { "amixer", "set", "Master", "5%+",     NULL };
static const char *downvol[]  = { "amixer", "set", "Master", "5%-",     NULL };
static const char *mutevol[]  = { "amixer", "set", "Master", "toggle",     NULL };
static const char *mutemic[]  = { "amixer", "set", "Capture", "toggle",     NULL };
static const char *brightup[] = { "brightnessctl", "s", "+2", NULL };
static const char *brightdown[] = { "brightnessctl", "s", "2-", NULL };
static const char *iwd[] = { "dmenu-iwd", NULL };
static const char *bookmarks[] = { "dmenu-book", "select", NULL };
static const char *copybookmarks[] = { "dmenu-book", "copy", NULL };
static const char *xrandr[] = { "dmenu-xrandr", NULL };
static const char *keepmenu[] = { "keepmenu", NULL };
static const char *scrot[] = { "screenshot", NULL };
static const char *winscrot[] = { "screenshot", "window", NULL };
static const char *selscrot[] = { "screenshot", "select", NULL };

#include "movestack.c"
static const Key keys[] = {
	/* modifier                     key        function        argument */
	{ 0,                            XF86XK_AudioLowerVolume,     spawn,          {.v = downvol } },
  { 0,                            XF86XK_AudioMute,            spawn,          {.v = mutevol } },
  { 0,                            XF86XK_AudioRaiseVolume,     spawn,          {.v = upvol } },
  { 0,                            XF86XK_AudioMicMute,         spawn,          {.v = mutemic } },
  { 0,                            XF86XK_MonBrightnessDown,    spawn,          {.v = brightdown } },
  { 0,                            XF86XK_MonBrightnessUp,      spawn,          {.v = brightup } },
	{ 0,		                        XK_Print,	 spawn,	         {.v = scrot } },
	{ MODKEY,		                    XK_Print,	 spawn,	         {.v = winscrot } },
	{ MODKEY|ShiftMask,		          XK_Print,	 spawn,	         {.v = selscrot } },
	{ MODKEY|ShiftMask,             XK_w,	     spawn,	         {.v = iwd } },
	{ MODKEY|ShiftMask,		          XK_p,	     spawn,	         {.v = clipmenu } },
	{ MODKEY|ShiftMask,             XK_Delete, spawn,	         SHCMD("clipdel -d '.'") },
	{ MODKEY|ControlMask,           XK_b,      spawn,          {.v = copybookmarks } },
	{ MODKEY|ShiftMask,             XK_b,      spawn,          {.v = bookmarks } },
	{ MODKEY,                       XK_x,      spawn,          {.v = xrandr } },
	{ MODKEY|ShiftMask,             XK_x,      spawn,          {.v = keepmenu } },
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_j,      movestack,      {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_k,      movestack,      {.i = -1 } },
	{ MODKEY,                       XK_r,      reorganizetags, {0} },
	{ MODKEY|ShiftMask,             XK_r,      distributetags, {0} },
	{ MODKEY|ShiftMask,             XK_h,      setcfact,       {.f = +0.25} },
	{ MODKEY|ShiftMask,             XK_l,      setcfact,       {.f = -0.25} },
	{ MODKEY|ShiftMask,             XK_o,      setcfact,       {.f =  0.00} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_s,      togglesticky,   {0} },
	{ MODKEY,                       XK_Down,   moveresize,     {.v = "0x 25y 0w 0h" } },
	{ MODKEY,                       XK_Up,     moveresize,     {.v = "0x -25y 0w 0h" } },
	{ MODKEY,                       XK_Right,  moveresize,     {.v = "25x 0y 0w 0h" } },
	{ MODKEY,                       XK_Left,   moveresize,     {.v = "-25x 0y 0w 0h" } },
	{ MODKEY|ShiftMask,             XK_Down,   moveresize,     {.v = "0x 0y 0w 25h" } },
	{ MODKEY|ShiftMask,             XK_Up,     moveresize,     {.v = "0x 0y 0w -25h" } },
	{ MODKEY|ShiftMask,             XK_Right,  moveresize,     {.v = "0x 0y 25w 0h" } },
	{ MODKEY|ShiftMask,             XK_Left,   moveresize,     {.v = "0x 0y -25w 0h" } },
	{ MODKEY|ControlMask,           XK_Up,     moveresizeedge, {.v = "t"} },
	{ MODKEY|ControlMask,           XK_Down,   moveresizeedge, {.v = "b"} },
	{ MODKEY|ControlMask,           XK_Left,   moveresizeedge, {.v = "l"} },
	{ MODKEY|ControlMask,           XK_Right,  moveresizeedge, {.v = "r"} },
	{ MODKEY|ControlMask|ShiftMask, XK_Up,     moveresizeedge, {.v = "T"} },
	{ MODKEY|ControlMask|ShiftMask, XK_Down,   moveresizeedge, {.v = "B"} },
	{ MODKEY|ControlMask|ShiftMask, XK_Left,   moveresizeedge, {.v = "L"} },
	{ MODKEY|ControlMask|ShiftMask, XK_Right,  moveresizeedge, {.v = "R"} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	//{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	{ MODKEY,            			      XK_y,  	   togglescratch,  {.ui = 0 } },
	{ MODKEY,            			      XK_u, 	   togglescratch,  {.ui = 1 } },
	{ MODKEY|ShiftMask,             XK_u, 	   togglescratch,  {.ui = 2 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	/* placemouse options, choose which feels more natural:
	 *    0 - tiled position is relative to mouse cursor
	 *    1 - tiled postiion is relative to window center
	 *    2 - mouse pointer warps to window center
	 *
	 * The moveorplace uses movemouse or placemouse depending on the floating state
	 * of the selected client. Set up individual keybindings for the two if you want
	 * to control these separately (i.e. to retain the feature to move a tiled window
	 * into a floating position).
	 */
	{ ClkClientWin,         MODKEY,         Button1,        moveorplace,    {.i = 1} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizeorfacts,  {0} },
//	{ ClkClientWin,       MODKEY|ShiftMask, Button3,        resizemouse,    {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

