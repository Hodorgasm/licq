/*
 * Licq GTK GUI Plugin
 *
 * Copyright (C) 2000, Jon Keating <jon@licq.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <gtk/gtk.h>

#include "licq_icqd.h"
#include "licq_gtk.h"

void menu_system_auth_user(GtkWidget *widget, const unsigned long uin)
{
	GtkWidget *label;
	GtkWidget *scroll;
	GtkWidget *ok;
	GtkWidget *cancel;
	GtkWidget *refuse;
	GtkWidget *h_box;
	GtkWidget *v_box;
	struct auth_user *au = g_new0(struct auth_user, 1);

	/* Create the window */
	au->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(au->window), "Licq - Authorize User");
	
	/* Make the boxes */
	h_box = gtk_hbox_new(FALSE, 5);
	v_box = gtk_vbox_new(FALSE, 5);

	/* Make the label and entry and pack them */
	label = gtk_label_new("Authorize UIN:");
  au->entry = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(au->entry), MAX_LENGTH_UIN);
	gtk_box_pack_start(GTK_BOX(h_box), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), au->entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 5);

	/* Accept only numbers in au->entry */
	g_signal_connect(G_OBJECT(au->entry), "insert-text",
			   G_CALLBACK(verify_numbers), 0);

	/* Make the scrolled text box */
	au->text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(au->text), TRUE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(au->text), GTK_WRAP_WORD);

	scroll = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll), au->text);
	gtk_box_pack_start(GTK_BOX(v_box), scroll, FALSE, FALSE, 5);

	/* Make the buttons and pack them */
	h_box = gtk_hbox_new(FALSE, 5);
	ok = gtk_button_new_from_stock(GTK_STOCK_OK);
	refuse = gtk_button_new_with_mnemonic("_Refuse");
	cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_box_pack_start(GTK_BOX(h_box), ok, TRUE, TRUE, 15);
	gtk_box_pack_start(GTK_BOX(h_box), refuse, TRUE, TRUE, 15);
	gtk_box_pack_start(GTK_BOX(h_box), cancel, TRUE, TRUE, 15);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 5);

	/* Connect the signals */
	g_signal_connect(G_OBJECT(cancel), "clicked",
			   G_CALLBACK(dialog_close), au->window);
	g_signal_connect(G_OBJECT(au->window), "destroy",
			   G_CALLBACK(dialog_close), au->window);
	g_signal_connect(G_OBJECT(ok), "clicked",
			   G_CALLBACK(auth_user_grant), au);
	g_signal_connect(G_OBJECT(refuse), "clicked",
			   G_CALLBACK(auth_user_refuse), au);

	/* Show the window */
	gtk_container_add(GTK_CONTAINER(au->window), v_box);
	gtk_widget_show_all(au->window);
	gtk_window_set_focus(GTK_WINDOW(au->window), au->entry);

	if(uin != 0)
	{
		const gchar *s_uin = g_strdup_printf("%ld", uin);
		gtk_entry_set_text(GTK_ENTRY(au->entry), s_uin);
	}
}

void auth_user_grant(GtkWidget *widget, struct auth_user *au)
{
	gulong uin = atol((const char *)gtk_editable_get_chars(GTK_EDITABLE(
					au->entry), 0, -1));
	gchar *reason = gtk_editable_get_chars(GTK_EDITABLE(au->text), 0, -1);

	icq_daemon->icqAuthorizeGrant(uin, reason);

	dialog_close(au->window, au->window);
}

void auth_user_refuse(GtkWidget *widget, struct auth_user *au)
{
	gulong uin = atol((const char *)gtk_editable_get_chars(GTK_EDITABLE(
					au->entry), 0, -1));
	gchar *reason = gtk_editable_get_chars(GTK_EDITABLE(au->text), 0, -1);

	/* Refuse it */
	icq_daemon->icqAuthorizeRefuse(uin, reason);

	dialog_close(au->window, au->window);
}
