#include "simkrell.h"

static GkrellmPanel     *panel;
static GtkWidget *gkrellm_vbox;

static gint     style_id;

static GkrellmMonitor   *simkrell_plugin;

void create_simkrell_plugin(GtkWidget *vbox, gint first_create)
{
    GkrellmStyle *style;
    GkrellmTextstyle *ts, *ts_alt;
    gint x, y;

#ifdef TEST_MODE
    fprintf(stderr, "start create_plugin\n");
#endif
    gkrellm_vbox = vbox;

    if (first_create)
    {
        panel = gkrellm_panel_new0();
    } else
    {
        gkrellm_destroy_krell_list(panel);
        gkrellm_destroy_decal_list(panel);
    }
    style = gkrellm_meter_style(style_id);

    ts = gkrellm_meter_textstyle(style_id);
    ts_alt = gkrellm_meter_alt_textstyle(style_id);

    panel->textstyle = ts;

    x = gkrellm_chart_width() - 20;
    y = 40;

    gkrellm_panel_configure(panel, NULL, style);
    gkrellm_panel_create(vbox, simkrell_plugin, panel);
}

void update_simkrell_plugin(void)
{
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


