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

#include "licq_gtk.h"

#include "licq_icqd.h"
#include "licq_user.h"

#include <gtk/gtk.h>
#include <stdlib.h>

struct search_user *su;
static gint num_found_users;

void search_user_window()
{
	/* Only one search window */
	if(su != 0)
	{
		gdk_window_raise(su->window->window);
		return;
	}

	GtkWidget *notebook;
	GtkWidget *table;
	GtkWidget *table_in_nb;
	GtkWidget *label;
	GtkWidget *statusbar;
	GtkWidget *clear;
	GtkWidget *search;
	GtkWidget *v_box;
	GtkWidget *done;
	GtkWidget *scroll;

	su = g_new0(struct search_user, 1);
	su->etag = g_new0(struct e_tag_data, 1);

	num_found_users = 0;

	/* Create the window */
	su->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(su->window), "Licq - Search User");

	gtk_window_set_default_size(GTK_WINDOW(su->window), 550, 350);

	/* destroy event for window */
	g_signal_connect(G_OBJECT(su->window), "destroy",
			   G_CALLBACK(search_close), su->window);
	
	/* Create the main table */
	table = gtk_table_new(4, 3, FALSE);
	gtk_container_add(GTK_CONTAINER(su->window), table);

	/* Create the notebook */
	notebook = gtk_notebook_new();

	/* The table inside the name tab */
	table_in_nb = gtk_table_new(4, 3, FALSE);

	label = gtk_label_new("Nick Name:");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(table_in_nb), label, 0, 1, 0, 1,
			 GTK_FILL, GTK_FILL, 3, 3);

	su->nick_name = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table_in_nb), su->nick_name, 1, 2, 0, 1,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	label = gtk_label_new("First Name:");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(table_in_nb), label, 0, 1, 1, 2,
			 GTK_FILL, GTK_FILL, 3, 3);

	su->first_name = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table_in_nb), su->first_name, 1, 2, 1, 2,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	label = gtk_label_new("Last Name:");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(table_in_nb), label, 0, 1, 2, 3,
			 GTK_FILL, GTK_FILL, 3, 3);

	su->last_name = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table_in_nb), su->last_name, 1, 2, 2, 3,
                         GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	label = gtk_label_new("E-Mail Address:");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(table_in_nb), label, 0, 1, 3, 4,
		GTK_FILL, GTK_FILL, 3, 3);

	su->email = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table_in_nb), su->email, 1, 2, 3, 4,
			 GTK_FILL,
			 GTK_FILL, 3, 3);

	label = gtk_label_new("Name");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table_in_nb, label);

	/* Another new tab (UIN Tab), a new table */
	table_in_nb = gtk_table_new(1, 2, FALSE);

	label = gtk_label_new("UIN:");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(table_in_nb), label, 0, 1, 0, 1,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 5, 25);

	su->uin = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(su->uin), MAX_LENGTH_UIN);
	gtk_table_attach(GTK_TABLE(table_in_nb), su->uin, 1, 2, 0, 1,
			 GTK_FILL, GTK_FILL, 5, 25);

	/* Only numbers allowed in the uin entry box */
	g_signal_connect(G_OBJECT(su->uin), "insert-text",
			   G_CALLBACK(verify_numbers), 0);
	
	/* Append to the notebook */
	label = gtk_label_new("UIN");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table_in_nb, label);

	/* Add the notebook to the main table */
	gtk_table_attach(GTK_TABLE(table), notebook, 0, 1, 0, 2,
			 GTK_FILL, GTK_FILL, 3, 3);

	/* The v_box for the clear and search buttons */
	v_box = gtk_vbox_new(FALSE, 5);

	/* The "Search" button */
	search = gtk_button_new_with_mnemonic("_Search");
	gtk_box_pack_start(GTK_BOX(v_box), search, TRUE, TRUE, 0);

	/* The search signal */
	g_signal_connect(G_OBJECT(search), "clicked",
			   G_CALLBACK(search_callback), 0);
	
	/* The "Clear List" button */
	clear = gtk_button_new_with_mnemonic("_Clear List");
	gtk_box_pack_start(GTK_BOX(v_box), clear, TRUE, TRUE, 0);

	/* The clear signal */
	g_signal_connect(G_OBJECT(clear), "clicked", G_CALLBACK(clear_callback), 0);
	
	/* The "Done" button */
	done = gtk_button_new_with_mnemonic("_Done");
	gtk_box_pack_start(GTK_BOX(v_box), done, TRUE, TRUE, 0);

	/* The done signal */
	g_signal_connect(G_OBJECT(done), "clicked",
			   G_CALLBACK(search_close), 0);
	
	gtk_table_attach(GTK_TABLE(table), v_box, 2, 3, 1, 2,
                         GTK_EXPAND, GTK_EXPAND, 3, 3);

	/* Found users list with a scroll bar */
	scroll = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);

	GtkListStore *store = gtk_list_store_new(6,	G_TYPE_STRING, G_TYPE_STRING, 
			G_TYPE_STRING, G_TYPE_STRING,	G_TYPE_STRING,	G_TYPE_STRING);
	su->list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(G_OBJECT(store));
	GtkCellRenderer *r_text = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
			"Nick", r_text, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(su->list), col);
	col = gtk_tree_view_column_new_with_attributes(
			"UIN", r_text, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(su->list), col);
	col = gtk_tree_view_column_new_with_attributes(
			"Name", r_text, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(su->list), col);
	col = gtk_tree_view_column_new_with_attributes(
			"E-mail", r_text, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(su->list), col);
	col = gtk_tree_view_column_new_with_attributes(
			"Status", r_text, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(su->list), col);
	col = gtk_tree_view_column_new_with_attributes(
			"Sex & Age", r_text, "text", 5, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(su->list), col);
	
	/* Double click on a user */
	g_signal_connect(G_OBJECT(su->list), "button_press_event",
			   G_CALLBACK(search_list_double_click), 0);
	
	gtk_widget_set_size_request(GTK_WIDGET(su->list), 230, 300);
	gtk_container_add(GTK_CONTAINER(scroll), su->list);

	gtk_table_attach(GTK_TABLE(table), scroll, 0, 3, 2, 3,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	/* The statusbar */
	statusbar = gtk_statusbar_new();
	gtk_table_attach(GTK_TABLE(table), statusbar, 0, 3, 3, 4,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);
	
	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "sta");
	gtk_statusbar_pop(GTK_STATUSBAR(statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), id, "Enter Search Parameters");

	/* The etag has to know about the statusbar */
	su->etag->statusbar = statusbar;

	gtk_widget_set_size_request(GTK_WIDGET(su->window), 435, 465); 
	gtk_widget_show_all(su->window);
}

void clear_callback(GtkWidget *widget, gpointer data)
{
	num_found_users = 0;
	GtkListStore *store = 
			GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	gtk_list_store_clear(store);
}

void search_callback(GtkWidget *widget, gpointer data)
{
	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(su->etag->statusbar),
						"sta");
	gtk_statusbar_pop(GTK_STATUSBAR(su->etag->statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(su->etag->statusbar),
			   id, "Searching, this may take awhile.");

	gulong uin = 0;
	gchar *nick_name;
	gchar *first_name;
	gchar *last_name;
	gchar *email;

	uin = (gulong)(atol(gtk_editable_get_chars(GTK_EDITABLE(su->uin),
						   0, -1)));
	nick_name = gtk_editable_get_chars(GTK_EDITABLE(su->nick_name),
					   0, -1);
	first_name = gtk_editable_get_chars(GTK_EDITABLE(su->first_name),
					    0, -1);
	last_name = gtk_editable_get_chars(GTK_EDITABLE(su->last_name),
					   0, -1);
	email = gtk_editable_get_chars(GTK_EDITABLE(su->email),
				       0, -1);

	if(uin >= 10000)
		su->etag->e_tag = icq_daemon->icqSearchByUin(uin);
/* XXX
	else
                su->etag->e_tag = icq_daemon->icqSerachWhitePages(first_name,
						last_name,
						nick_name,
						email,
						0,
						0,
						0,
						0,
						0,
						0,
						0,
						0,
						0,
						0,
						0,
						false);
*/
}

void search_list_double_click(GtkWidget *widget,
			      GdkEventButton *eb,
			      gpointer data)
{
	gulong uin;
	ICQUser *user;

	/* Get which cell was clicked in */
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(su->list));
	GtkTreeIter iter;
	GtkTreeModel *model;
	
	if (!gtk_tree_selection_get_selected(sel, &model, &iter))
		return;
	if(!(eb->type == GDK_2BUTTON_PRESS && eb->button == 1))
		return;

	gchar *c_uin;
	gtk_tree_model_get(model, &iter, 1, &c_uin, -1);
	uin = atol(c_uin);
	g_free(c_uin);

	if((uin == 0) || (user = gUserManager.FetchUser(uin, LOCK_N)))
		return;

	icq_daemon->AddUserToList(uin);

	char *for_statusbar = g_strdup_printf("User (%ld) added", uin);

	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(su->etag->statusbar),
						"sta");
	gtk_statusbar_pop(GTK_STATUSBAR(su->etag->statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(su->etag->statusbar), id, for_statusbar);
}

void search_result(ICQEvent *event)
{	
	/* Make sure it's the right event */
	if(!event->Equals(su->etag->e_tag))
		return;

	CSearchAck *search_ack = event->SearchAck();

	if(event->Result() == EVENT_SUCCESS)
		search_done(search_ack->More());
	else if(event->Result() == EVENT_ACKED)
		search_found(search_ack);
	else
		search_failed();
}

void search_done(bool more)
{
	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(su->etag->statusbar),
						 "sta");
	
	if(more)
	{
		gtk_statusbar_pop(GTK_STATUSBAR(su->etag->statusbar), id);
		gtk_statusbar_push(GTK_STATUSBAR(su->etag->statusbar), id,
			"More users found, narrow your search and try again.");
	}
	
	else
	{
		gtk_statusbar_pop(GTK_STATUSBAR(su->etag->statusbar), id);
		gtk_statusbar_push(GTK_STATUSBAR(su->etag->statusbar), id,
			"Search complete, double click user to add him/her.");
	}
}

void search_found(CSearchAck *ack)
{
	gchar *name = g_strdup_printf("%s %s", ack->FirstName(), ack->LastName());	

	char *txt = g_strdup_printf("%ld", ack->Uin());

	GtkListStore *store = 
			GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(su->list)));
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 
			1, txt,
			2, (gchar *)ack->Alias(),
			3, name,
			4, (gchar *)ack->Alias(),
			-1);
	
	num_found_users++;
	g_free(txt);
}

void search_failed()
{
	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(su->etag->statusbar),
						"sta");
	gtk_statusbar_pop(GTK_STATUSBAR(su->etag->statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(su->etag->statusbar), id, "Search failed.");
}

void search_close(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(su->window);
	su = 0;
}
