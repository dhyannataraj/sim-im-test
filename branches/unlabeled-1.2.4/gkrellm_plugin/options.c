/***************************************************************************
 *                           options.c  -  description
 *                           -------------------
 *                           begin                : Sun Mar 10 2002
 *                           copyright            : (C) 2002 by Vladimir Shutoff
 *                           email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


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

