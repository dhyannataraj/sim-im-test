#include <gkrellm2/gkrellm.h>
#include "config.h"

#include <stdlib.h>
#include <stdio.h>

#define CONFIG_NAME	"SIMKrell"
#define CONFIG_KEYWORD	"simkrell"
#define STYLE_NAME	"simkrell"

GkrellmMonitor *gkrellm_init_plugin(void);

void create_simkrell_plugin(GtkWidget *vbox, gint first_create);
void update_simkrell_plugin(void);

void create_simkrell_config(GtkWidget *tab);
void simkrell_save_config(FILE *f);
void simkrell_load_config(gchar *arg);
void simkrell_apply_config(void);

