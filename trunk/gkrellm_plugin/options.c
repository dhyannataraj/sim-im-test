#include "simkrell.h"

static char simkrell_help_text[] = 
"SIMkrell\n";

void create_simkrell_config(GtkWidget *tab)
{
    GtkWidget *notebook, *vbox, *help;

    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    gtk_box_pack_start(GTK_BOX(tab), notebook, TRUE, TRUE, 0);

    /* help tab */
    vbox = gkrellm_gtk_framed_notebook_page(notebook, (char*)"Help");

    help = gkrellm_gtk_scrolled_text_view(vbox, NULL, 
		    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gkrellm_gtk_text_view_append(help, (char*)simkrell_help_text);
}

void simkrell_apply_config(void)
{
}

void simkrell_save_config(FILE *f)
{
}

void simkrell_load_config(gchar *arg)
{
}

