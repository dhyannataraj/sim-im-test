/***************************************************************************
                        simkrell.c  -  description
                        -------------------
                        begin                : Sun Sep 10 2002
                        copyright            : (C) 2002 by Vladimir Shutoff
                        email                : vovan@shutoff.ru
**************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#else
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <pwd.h>

#include "simkrell.h"

#include "offline.xpm"

/* #define SIM_KRELL_DEBUG	1	*/

static GkrellmPanel     	*panel;
static GkrellmDecal     	*decal_msg, *decal_status;
static GkrellmDecal		*decal_icon;

static gint     style_id;

static GkrellmMonitor   *simkrell_plugin;

int sim_sock = -1;

static char sock_name[256];
static char local_name_buf[256];

static char line[8192];
static int  line_pos = 0;

static char msg_str[512];
static char status_str[256];

static char SIM_DOWN[] = "SIM down";

static gint x_scroll = 0;

static char STATUS[] = "Status:";
static char MSG[] = "Msg:";
static char ICONS[] = "Icons:";
static char ICON[] = "ICON ";

static GdkPixmap *offline_icon;
static GdkBitmap *offline_mask;

static GdkPixmap **icons = NULL;
static GdkBitmap **masks = NULL;
static char	 **names = NULL;

static int n_icons = 0;
static int alloc_icons = 0;

static int icon_free = 0;
static int req_sent = 0;

static char current_icons[256];
static char *current_icon = NULL;

static char *icon1 = NULL;
static char *icon2 = NULL;
static char *icon3 = NULL;

static int change_icon = 0;

static int cnt_status = 0;

static void
add_icon(const char *name, GkrellmPiximage *im)
{
    GdkPixmap *pict = NULL;
    GdkBitmap *mask = NULL;
    char *p;
#ifdef SIM_KRELL_DEBUG
    fprintf(stderr, "Add icon %s\n", name);
#endif
    gkrellm_scale_theme_background(im, &pict, &mask, 16, 16);
    if (n_icons >= alloc_icons){
        alloc_icons += 5;
        if (icons){
            icons = (GdkPixmap**)realloc(icons, sizeof(GdkPixmap*) * alloc_icons);
            masks = (GdkBitmap**)realloc(masks, sizeof(GdkBitmap*) * alloc_icons);
            names = (char**)realloc(names, sizeof(char*) * alloc_icons);
        }else{
            icons = (GdkPixmap**)malloc(sizeof(GdkPixmap*) * alloc_icons);
            masks = (GdkBitmap**)malloc(sizeof(GdkBitmap*) * alloc_icons);
            names = (char**)malloc(sizeof(char*) * alloc_icons);
        }
    }
    icons[n_icons] = pict;
    masks[n_icons] = mask;
    names[n_icons] = strdup(name);
    for (p = names[n_icons]; *p; p++)
        if (*p == ' '){
            *p = 0;
            break;
        }
    n_icons++;
#ifdef SIM_KRELL_DEBUG
    fprintf(stderr, ">> %u %u\n", n_icons, alloc_icons);
#endif
}

static void
clear_icons()
{
    int i;
    for (i = 0; i < n_icons; i++){
        if (icons[i] == decal_icon->pixmap){
            icon_free = 1;
        }else{
            gkrellm_free_pixmap(&icons[i]);
            gkrellm_free_bitmap(&masks[i]);
        }
        free(names[i]);
    }
    free(icons);
    free(masks);
    free(names);
    icons = NULL;
    masks = NULL;
    names = NULL;
    alloc_icons = 0;
    n_icons = 0;
}

static void
process_line()
{
    if (strlen(line) == 0) return;
#ifdef SIM_KRELL_DEBUG
    fprintf(stderr, "Line [%s]\n", line);
#endif
    if (!strcmp(line, "SIM ready")){
        char NOTIFY[] = "NOTIFY on\n";
        write(sim_sock, NOTIFY, strlen(NOTIFY));
        return;
    }
    if ((strlen(line) > strlen(STATUS)) && !memcmp(line, STATUS, strlen(STATUS))){
        strncpy(status_str, line + strlen(STATUS), sizeof(status_str));
        cnt_status++;
        return;
    }
    if ((strlen(line) > strlen(MSG)) && !memcmp(line, MSG, strlen(MSG))){
        strncpy(msg_str, line + strlen(MSG), sizeof(msg_str));
        x_scroll = gkrellm_chart_width();
        return;
    }
    if ((strlen(line) > strlen(ICONS)) && !memcmp(line, ICONS, strlen(ICONS))){
        char *p;
        strncpy(current_icons, line + strlen(ICONS), sizeof(current_icons));
        icon2 = icon3 = NULL;
        icon1 = current_icons;
        for (p = current_icons; *p; p++)
            if (*p == ' ') break;
        if (*p){
            *p = 0;
            p++;
        }
        for (; *p; p++)
            if (*p != ' ') break;
        if (*p) icon2 = p;
        for (; *p; p++)
            if (*p == ' ') break;
        if (*p){
            *p = 0;
            p++;
        }
        for (; *p; p++)
            if (*p != ' ') break;
        if (*p) icon3 = p;
        current_icon = icon1;
        change_icon = 1;
        return;
    }
    if ((strlen(line) > strlen(ICON)) && !memcmp(line, ICON, strlen(ICON))){
        int n_line = 0;
        char **xpm;
        char *data, *p;
        char *name = line + strlen(ICON);
        GkrellmPiximage *im;
        for (; *name; name++)
            if (*name != ' ') break;
        for (p = name; *p; p++)
            if (*p == ' ') break;
        for (; *p; p++)
            if (*p == '\"') break;
        data = p;
        for (; *p; p++)
            if (*p == '\"') n_line++;
        if (n_line == 0) return;
        xpm = (char**)malloc(n_line * sizeof(char*));
        n_line = 0;
        for (p = data; *p ; p++){
            if (*p != '\"') continue;
            *p = 0;
            xpm[n_line++] = (p + 1);
        }
        im = gkrellm_piximage_new_from_xpm_data((gchar**)xpm);
        free(xpm);
        add_icon(name, im);
        req_sent = 0;
        return;
    }
    if (!strcmp(line, "Clear icons")){
        clear_icons();
        change_icon = 1;
        return;
    }
}

static void
close_sock()
{
    close(sim_sock);
    sim_sock = -1;
    msg_str[0] = 0;
    strcpy(status_str, SIM_DOWN);
    x_scroll = 0;
    clear_icons();
    icon1 = icon2 = icon3 = current_icon = NULL;
    change_icon = 1;
    line_pos = 0;
    cnt_status++;
}

static void
check_connection()
{
    char *local_name;
    struct sockaddr_un sun_local;
    struct sockaddr_un sun_remote;

    if (sim_sock < 0){
        if (access(sock_name, W_OK) < 0)
            return;

        sim_sock = socket(PF_UNIX, SOCK_STREAM, 0);
        if (sim_sock < 0){
#ifdef SIM_KRELL_DEBUG
            fprintf(stderr, "Can't create socket: %s\n", strerror(errno));
#endif
            return;
        }
        strcpy(local_name_buf, "/tmp/sim.XXXXX");
        local_name = mktemp(local_name_buf);

        sun_local.sun_family = AF_UNIX;
        strcpy(sun_local.sun_path, local_name);
        if (bind(sim_sock, (struct sockaddr*)&sun_local, sizeof(sun_local)) < 0){
            fprintf(stderr, "Can't bind %s: %s\n", local_name, strerror(errno));
            return;
        }
        sun_remote.sun_family = AF_UNIX;
        strcpy(sun_remote.sun_path, sock_name);
        if (connect(sim_sock, (struct sockaddr*)&sun_remote, sizeof(sun_remote)) < 0){
#ifdef SIM_KRELL_DEBUG
            fprintf(stderr, "Can't connect to %s: %s\n", sock_name, strerror(errno));
#endif
            unlink(local_name);
            return;
        }
        unlink(local_name);
#ifdef SIM_KRELL_DEBUG
        fprintf(stderr, "Connect OK\n");
#endif
    }
    for (;;){
        int r;
        char c;
        fd_set fs;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&fs);
        FD_SET(sim_sock, &fs);
        r = select(sim_sock + 1, &fs, NULL, NULL, &tv);
        if (r < 0){
            fprintf(stderr, "Select error: %s\n", strerror(errno));
            close_sock();
            return;
        }
        if (r == 0) return;
        r = read(sim_sock, &c, 1);
        if (r < 0){
#ifdef SIM_KRELL_DEBUG
            fprintf(stderr, "Read error: %s\n", strerror(errno));
#endif
            close_sock();
            return;
        }
        if (r == 0){
#ifdef SIM_KRELL_DEBUG
            fprintf(stderr, "Connection closed\n");
#endif
            close_sock();
            return;
        }
        if (c == '\n'){
            line[line_pos] = 0;
            process_line();
            line_pos = 0;
            continue;
        }
        if (c != '\r'){
            if (line_pos < (int)(sizeof(line)))
                line[line_pos++] = c;
        }
    }
}

static gint
panel_expose_event(GtkWidget *widget, GdkEventExpose *ev)
{
    gdk_draw_pixmap(widget->window,
                    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                    panel->pixmap, ev->area.x, ev->area.y, ev->area.x, ev->area.y,
                    ev->area.width, ev->area.height);
    return FALSE;
}

static gint
cb_panel_press(GtkWidget *widget, GdkEventButton *ev)
{
    if (sim_sock != -1){
        if (ev->button == 1){
            char MAINWINDOW_TOGGLE[] = "MAINWINDOW TOGGLE\n";
            write(sim_sock, MAINWINDOW_TOGGLE, strlen(MAINWINDOW_TOGGLE));
        }else if (ev->button == 3){
            char buf[256];
            snprintf(buf, sizeof(buf), "POPUP %i %i\n", (int)(ev->x_root), (int)(ev->y_root));
            write(sim_sock, buf, strlen(buf));
        }else if (ev->button == 2){
            char OPEN[] = "OPEN\n";
            write(sim_sock, OPEN, strlen(OPEN));
        }
    }
    return FALSE;
}

static char *sim_argv[] =
    {
        (char*)(PREFIX "/bin/sim"),
        NULL
    };

void create_simkrell_plugin(GtkWidget *vbox, gint first_create)
{
    GkrellmStyle            *style;
    GkrellmTextstyle        *ts, *ts_alt;
    gint                    y;
    GkrellmPiximage	    *im;

    if (first_create)
        panel = gkrellm_panel_new0();

    style = gkrellm_meter_style(style_id);
    ts = gkrellm_meter_textstyle(style_id);
    ts_alt = gkrellm_meter_alt_textstyle(style_id);

    strcpy(status_str, SIM_DOWN);
    msg_str[0] = 0;

    decal_msg = gkrellm_create_decal_text(panel, (char*)"XXXXXX" , ts, style, 20, 2, -1);
    y = decal_msg->y + decal_msg->h + 2;
    decal_status = gkrellm_create_decal_text(panel, (char*)status_str, ts_alt, style, 20, y, -1);
    y = (decal_status->y + decal_status->h - 14) / 2;
    if (y < 0) y = 0;
    im = gkrellm_piximage_new_from_xpm_data((gchar**)offline);
    gkrellm_scale_theme_background(im, &offline_icon, &offline_mask, 16, 16);
    gkrellm_destroy_piximage(im);
    decal_icon = gkrellm_create_decal_pixmap(panel, offline_icon, offline_mask, 0, style, 2, y);

    gkrellm_panel_configure(panel, NULL, style);
    gkrellm_panel_create(vbox, simkrell_plugin, panel);
    gkrellm_draw_decal_pixmap(panel, decal_icon, 0);
    if (first_create){
        gtk_signal_connect(GTK_OBJECT (panel->drawing_area), "expose_event",
                           (GtkSignalFunc) panel_expose_event, NULL);
        gtk_signal_connect(GTK_OBJECT(panel->drawing_area), "button_press_event",
                           (GtkSignalFunc) cb_panel_press, NULL);
    }
    check_connection();
    if (sim_sock == -1){
#ifdef SIM_KRELL_DEBUG
        fprintf(stderr, "Run sim\n");
#endif
        if (fork()) return;
        if (execv(sim_argv[0], sim_argv) < 0){
            fprintf(stderr, "Can't run sim: %s\n", strerror(errno));
            return;
        }
        return;
    }
}

void update_simkrell_plugin(void)
{
    int i;
    static gint w;
    char *new_icon = icon1;
    int n = GK.timer_ticks >> 3;
    check_connection();
    if (w == 0)
        w = gkrellm_chart_width();
    x_scroll = (x_scroll + 1) % (2 * w);
    decal_msg->x_off = w - x_scroll;
    decal_status->x_off = 0;
    gkrellm_draw_decal_text(panel, decal_msg, msg_str, *msg_str ? (w - x_scroll) : 2 * w);
    gkrellm_draw_decal_text(panel, decal_status, status_str, cnt_status);
    if ((GK.timer_ticks & 7) == 0){
        if (icon3){
            if (n & 1){
                new_icon = icon3;
            }else if ((n & 3) == 2){
                new_icon = icon2;
            }
        }else if (icon2){
            if (n & 1)
                new_icon = icon2;
        }
        if (new_icon != current_icon){
            current_icon = new_icon;
            change_icon = 1;
        }}

    if (change_icon){
#ifdef SIM_KRELL
        fprintf(stderr, "Icon changed [%s]\n", current_icon);
#endif
        if (current_icon){
            for (i = 0; i < n_icons; i++){
                if (!strcmp(names[i], current_icon))
                    break;
            }
            if (i < n_icons){
                if (icon_free){
                    gkrellm_free_pixmap(&decal_icon->pixmap);
                    gkrellm_free_bitmap(&decal_icon->mask);
                    icon_free = 0;
                }
                decal_icon->pixmap = icons[i];
                decal_icon->mask = masks[i];
                decal_icon->value = 1;
                gkrellm_draw_decal_pixmap(panel, decal_icon, 0);
                change_icon = 0;
            }else if (!req_sent){
                write(sim_sock, ICON, strlen(ICON));
                write(sim_sock, current_icon, strlen(current_icon));
                write(sim_sock, "\n", 1);
                req_sent = 1;
            }
        }else{
            if (icon_free){
                gkrellm_free_pixmap(&decal_icon->pixmap);
                gkrellm_free_bitmap(&decal_icon->mask);
                icon_free = 0;
            }
            decal_icon->pixmap = offline_icon;
            decal_icon->mask = offline_mask;
            decal_icon->value = 1;
            gkrellm_draw_decal_pixmap(panel, decal_icon, 0);
            change_icon = 0;
        }
    }
    gkrellm_draw_panel_layers(panel);
}

static GkrellmMonitor plugin_monitor =
    {
        (char*)CONFIG_NAME,             /* Name, for config tab.             */
        0,                              /* Id,  0 if a plugin                */
        create_simkrell_plugin,         /* The create function               */
        update_simkrell_plugin,         /* The update function               */
        create_simkrell_config,  	/* The config tab create function    */
        simkrell_apply_config,          /* Apply the config function         */
        simkrell_save_config,           /* Save user config                  */
        simkrell_load_config,           /* Load user config                  */
        (char*)CONFIG_KEYWORD,          /* config keyword                    */
        NULL,                           /* Undefined 2                       */
        NULL,                           /* Undefined 1                       */
        NULL,                           /* private                           */
        MON_MAIL,                       /* Insert plugin before this monitor */
        NULL,                           /* Handle if a plugin, filled in by GkrellM */
        NULL                            /* path if a plugin, filled in by GKrellM   */
    };

GkrellmMonitor *gkrellm_init_plugin(void)
{
    uid_t uid = getuid();
    struct passwd *pwd = getpwuid(uid);
    simkrell_plugin = &plugin_monitor;
    style_id = gkrellm_add_meter_style(&plugin_monitor, (char*)STYLE_NAME);

    if (pwd){
        snprintf(sock_name, sizeof(sock_name), "/tmp/sim.%s", pwd->pw_name);
    }else{
        snprintf(sock_name, sizeof(sock_name), "/tmp/sim.%u", uid);
    }
    return &plugin_monitor;
}


