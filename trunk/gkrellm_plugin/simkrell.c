#include "simkrell.h"

#include "offline.xpm"

static GkrellmPanel     	*panel;
static GkrellmDecal     	*decal_msg, *decal_status;
static GkrellmDecal		*decal_icon;

static gint     style_id;

static GkrellmMonitor   *simkrell_plugin;

static gchar    *scroll_text = (char*)"Scrolling text";

static gint
panel_expose_event(GtkWidget *widget, GdkEventExpose *ev)
{
    gdk_draw_pixmap(widget->window,
                    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                    panel->pixmap, ev->area.x, ev->area.y, ev->area.x, ev->area.y,
                    ev->area.width, ev->area.height);
    return FALSE;
}


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

    decal_msg = gkrellm_create_decal_text(panel, (char*)"XXXXX", ts, style, 20, 2, -1);
    y = decal_msg->y + decal_msg->h + 2;
    decal_status = gkrellm_create_decal_text(panel, (char*)"SIM down", ts_alt, style, 20, y, 0);
    y = (decal_status->y + decal_status->h - 14) / 2;
    if (y < 0) y = 0;
    im = gkrellm_piximage_new_from_xpm_data((gchar**)offline);
    decal_icon = gkrellm_make_scaled_decal_pixmap(panel, im, style, 0, 2, y, 16, 16); 
    
    gkrellm_panel_configure(panel, NULL, style);
    gkrellm_panel_create(vbox, simkrell_plugin, panel);
    gkrellm_draw_decal_text(panel, decal_status, (char*)"SIM down", 1);
    gkrellm_draw_decal_pixmap(panel, decal_icon, 0);
    if (first_create)
        gtk_signal_connect(GTK_OBJECT (panel->drawing_area), "expose_event",
                           (GtkSignalFunc) panel_expose_event, NULL);
}

void update_simkrell_plugin(void)
{
    static gint     x_scroll, w;

    if (w == 0)
        w = gkrellm_chart_width();
    x_scroll = (x_scroll + 1) % (2 * w);
    decal_msg->x_off = w - x_scroll;
    gkrellm_draw_decal_text(panel, decal_msg, scroll_text, w - x_scroll);
    gkrellm_draw_decal_pixmap(panel, decal_icon, 0);
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
    simkrell_plugin = &plugin_monitor;
    style_id = gkrellm_add_meter_style(&plugin_monitor, (char*)STYLE_NAME);
    return &plugin_monitor;
}


