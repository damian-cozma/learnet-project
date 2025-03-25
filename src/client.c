#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <gtk/gtk.h>
#include <stdbool.h>

// ---------------------------------- UI ----------------------------------
char current_user[128] = "";
int sd;
static guint refresh_timer_id = 0;

GtkWidget *window_main, *window_cautare, *entry_message, *text_view_response, *window_forum, *window_discutii, *window_lista_prieteni, *window_add_prieteni;
GtkWidget *window_login, *entry_username, *entry_password;

//LOGIN
bool validate_login(const char *username, const char *password) {
    FILE *file = fopen("login.config", "r");

    char line[256];
    char user[128], pass[128];

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%127[^:]:%127s", user, pass) == 2) {
            if (strcmp(username, user) == 0 && strcmp(password, pass) == 0) {
                fclose(file);
                return true;
            }
        }
    }

    fclose(file);
    return false;
}
void windowMeniu();
void on_login_clicked(GtkWidget *widget, gpointer data) {
    const char *username = gtk_entry_get_text(GTK_ENTRY(entry_username));
    const char *password = gtk_entry_get_text(GTK_ENTRY(entry_password));

    if (validate_login(username, password)) {
        gtk_widget_hide(window_login);
        strncpy(current_user, username, sizeof(current_user) - 1);
        current_user[sizeof(current_user) - 1] = '\0';
        windowMeniu();
    } else {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_login),
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_OK,
                                                   "Informatii gresite!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

//ALTELE
char *afisare_prieteni() {
    char request[256];
    snprintf(request, sizeof(request), "serverFRIENDS:%s", current_user);
    char buffer[4096];
    int bytes;

    if (write(sd, request, strlen(request)) <= 0) {
        perror("[client] Eroare la trimiterea cererii LIST_FRIENDS.\n");
        return strdup("Eroare la comunicarea cu serverul.");
    }

    bzero(buffer, sizeof(buffer));
    bytes = read(sd, buffer, sizeof(buffer) - 1);
    if (bytes < 0) {
        perror("[client] Eroare la citirea raspunsului de la server.\n");
        return strdup("Eroare la primirea datelor de la server.");
    }

    buffer[bytes] = '\0';
    return strdup(buffer);
}
void CSS() {
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_css_provider_load_from_path(provider, "style.css", NULL);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
}
void clickAcceptFriend(GtkWidget *widget, gpointer data);
void clickCautare(GtkWidget *widget, gpointer data);
void clickBackMain(GtkWidget *widget, gpointer data);
void clickSend(GtkWidget *widget, gpointer data);
void clickSendPrieteni(GtkWidget *widget, gpointer data);
void clickForum(GtkWidget *widget, gpointer data);
void clickBackForum(GtkWidget *widget, gpointer data);
char *create_topic(const char *topic);
char *list_topics();
char *add_message_to_topic(const char *topic, const char *message);
void on_create_topic_clicked(GtkWidget *widget, gpointer data);
void on_topic_double_click(GtkListBox *list_box, GtkListBoxRow *row, gpointer user_data);
char *get_topic_messages(const char *topic);
void on_add_message_to_topic(GtkWidget *widget, gpointer data);
void clickBackToDiscussions(GtkWidget *widget, gpointer data);
void clickBookmark(GtkWidget *widget, gpointer data);
char *get_user_bookmarks(const char *username);
void clickRemoveFriend(GtkWidget *widget, gpointer data);
gboolean refresh_messages(gpointer data) {
    GtkWidget *text_view = GTK_WIDGET(data);
    const char *topic = gtk_window_get_title(GTK_WINDOW(gtk_widget_get_toplevel(text_view)));

    char *messages = get_topic_messages(topic);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, messages, -1);
    free(messages);

    return TRUE;
}

//WINDOWS
void windowListaPrieteni() {
    GtkWidget *overlay, *image, *box, *button_back, *scrolled_window, *friends_list;

    gtk_widget_hide(window_forum);

    window_lista_prieteni = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_lista_prieteni), "Lista prieteni");
    gtk_window_set_default_size(GTK_WINDOW(window_lista_prieteni), 1000, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window_lista_prieteni), 10);
    gtk_widget_set_name(window_lista_prieteni, "window-lista-prieteni");

    gtk_window_set_position(GTK_WINDOW(window_lista_prieteni), GTK_WIN_POS_CENTER);

    overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(window_lista_prieteni), overlay);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(box, 50);
    gtk_widget_set_margin_bottom(box, 50);
    gtk_widget_set_margin_start(box, 50);
    gtk_widget_set_margin_end(box, 50);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), box);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_name(scrolled_window, "scrolled-window");
    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);

    friends_list = gtk_list_box_new();
    gtk_widget_set_name(friends_list, "friends-list");
    gtk_container_add(GTK_CONTAINER(scrolled_window), friends_list);

    char *prieteni = afisare_prieteni();
    char *line = strtok(prieteni, "\n");
    while (line) {
        char *clean_line = strdup(line);
        if (clean_line) {
            g_strstrip(clean_line);

            if (strlen(clean_line) > 0) {
                GtkWidget *row = gtk_list_box_row_new();
                GtkWidget *row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
                gtk_container_add(GTK_CONTAINER(row), row_box);

                GtkWidget *center_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
                gtk_widget_set_halign(center_box, GTK_ALIGN_CENTER);
                gtk_box_pack_start(GTK_BOX(row_box), center_box, TRUE, TRUE, 0);

                GtkWidget *label = gtk_label_new(clean_line);
                gtk_box_pack_start(GTK_BOX(center_box), label, FALSE, FALSE, 0);

                GtkWidget *button_remove = gtk_button_new_with_label("X");
                gtk_widget_set_name(button_remove, "button-remove-friend");
                gtk_widget_set_margin_start(button_remove, 30);
                g_signal_connect(button_remove, "clicked", G_CALLBACK(clickRemoveFriend), strdup(clean_line));
                gtk_box_pack_start(GTK_BOX(center_box), button_remove, FALSE, FALSE, 0);

                gtk_list_box_insert(GTK_LIST_BOX(friends_list), row, -1);
                gtk_widget_show_all(row);
            }
            free(clean_line);
        }
        line = strtok(NULL, "\n");
    }

    free(prieteni);

    button_back = gtk_button_new_with_label("Înapoi");
    gtk_widget_set_size_request(button_back, 400, 40);
    gtk_widget_set_margin_top(button_back, 20);
    gtk_widget_set_halign(button_back, GTK_ALIGN_CENTER);

    g_signal_connect(button_back, "clicked", G_CALLBACK(clickBackForum), window_lista_prieteni);
    gtk_box_pack_start(GTK_BOX(box), button_back, FALSE, FALSE, 0);

    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_widget_show_all(window_lista_prieteni);
}
void windowAddPrieteni() {
    GtkWidget *box, *label_instructiuni, *button_send, *button_back;
    GtkWidget *friend_request_box, *friend_request_label, *button_accept_request;

    if (GTK_IS_WIDGET(window_forum)) {
        gtk_widget_hide(window_forum);
    }

    window_add_prieteni = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_add_prieteni), "Adauga prieteni");
    gtk_window_set_default_size(GTK_WINDOW(window_add_prieteni), 1000, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window_add_prieteni), 10);
    gtk_window_set_position(GTK_WINDOW(window_add_prieteni), GTK_WIN_POS_CENTER);
    gtk_widget_set_name(window_add_prieteni, "window-add-prieteni");

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(window_add_prieteni), box);

    label_instructiuni = gtk_label_new("Introdu un nume de utilizator:");
    gtk_widget_set_margin_top(label_instructiuni, 20);
    gtk_box_pack_start(GTK_BOX(box), label_instructiuni, FALSE, FALSE, 0);

    entry_message = gtk_entry_new();
    gtk_widget_set_margin_start(entry_message, 50);
    gtk_widget_set_margin_end(entry_message, 50);
    gtk_widget_set_margin_top(entry_message, 10);
    gtk_box_pack_start(GTK_BOX(box), entry_message, FALSE, FALSE, 0);

    button_send = gtk_button_new_with_label("Send");
    gtk_widget_set_margin_start(button_send, 50);
    gtk_widget_set_margin_end(button_send, 50);
    gtk_widget_set_margin_top(button_send, 10);
    g_signal_connect(button_send, "clicked", G_CALLBACK(clickSendPrieteni), NULL);
    gtk_box_pack_start(GTK_BOX(box), button_send, FALSE, FALSE, 0);

    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 30);
    gtk_box_pack_start(GTK_BOX(box), spacer, FALSE, FALSE, 0);

    GtkWidget *label_cereri_prietenie = gtk_label_new("Cereri de prietenie:");
    gtk_box_pack_start(GTK_BOX(box), label_cereri_prietenie, FALSE, FALSE, 0);

    char request[256];
    snprintf(request, sizeof(request), "serverFRIEND_REQUESTS:%s", current_user);

    if (write(sd, request, strlen(request)) <= 0) {
        perror("[client] Eroare la trimiterea cererii FRIEND_REQUESTS.\n");
        return;
    }

    char buffer[4096];
    bzero(buffer, sizeof(buffer));
    int bytes = read(sd, buffer, sizeof(buffer) - 1);
    if (bytes < 0) {
        perror("[client] Eroare la citirea raspunsului de la server.\n");
        return;
    }

    buffer[bytes] = '\0';

    char *line = strtok(buffer, "\n");

    if (strncmp(line, "Cereri de prietenie pentru", 26) == 0) {
        line = strtok(NULL, "\n");
    }

    while (line) {
        friend_request_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

        gtk_widget_set_halign(friend_request_box, GTK_ALIGN_CENTER);

        friend_request_label = gtk_label_new(line);
        gtk_box_pack_start(GTK_BOX(friend_request_box), friend_request_label, FALSE, FALSE, 0);

        button_accept_request = gtk_button_new_with_label("Accepta");
        g_signal_connect(button_accept_request, "clicked", G_CALLBACK(clickAcceptFriend), strdup(line));

        gtk_widget_set_margin_start(button_accept_request, 20);
        gtk_box_pack_start(GTK_BOX(friend_request_box), button_accept_request, FALSE, FALSE, 0);

        gtk_box_pack_start(GTK_BOX(box), friend_request_box, FALSE, FALSE, 0);

        line = strtok(NULL, "\n");
    }

    button_back = gtk_button_new_with_label("Înapoi");
    gtk_widget_set_size_request(button_back, 400, 40);
    gtk_widget_set_margin_top(button_back, 50);
    gtk_widget_set_margin_bottom(button_back, 50);
    gtk_widget_set_halign(button_back, GTK_ALIGN_CENTER);

    g_signal_connect(button_back, "clicked", G_CALLBACK(clickBackForum), window_add_prieteni);
    gtk_box_pack_end(GTK_BOX(box), button_back, FALSE, FALSE, 0);

    gtk_widget_show_all(window_add_prieteni);
}
void windowDiscutii() {
    GtkWidget *notebook, *tab_discutii, *tab_bookmarks;
    GtkWidget *label_discutii, *label_bookmarks;
    GtkWidget *box_bookmarks, *scroll_area_bookmarks, *bookmarks_list;
    GtkWidget *button_back;
    GtkWidget *main_box;

    gtk_widget_hide(window_forum);

    window_discutii = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_discutii), "Discuții");
    gtk_window_set_default_size(GTK_WINDOW(window_discutii), 1000, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window_discutii), 10);
    gtk_window_set_position(GTK_WINDOW(window_discutii), GTK_WIN_POS_CENTER);
    gtk_widget_set_name(window_discutii, "window-discutii");

    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window_discutii), main_box);

    notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(main_box), notebook, TRUE, TRUE, 0);

    tab_discutii = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(tab_discutii, 20);
    gtk_widget_set_margin_start(tab_discutii, 20);
    gtk_widget_set_margin_end(tab_discutii, 20);
    gtk_widget_set_margin_bottom(tab_discutii, 20);

    label_discutii = gtk_label_new("Toate Discuțiile");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_discutii, label_discutii);

    GtkWidget *create_topic_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(tab_discutii), create_topic_box, FALSE, FALSE, 0);

    GtkWidget *label_instructiuni = gtk_label_new("Creează un nou topic:");
    gtk_box_pack_start(GTK_BOX(create_topic_box), label_instructiuni, FALSE, FALSE, 0);

    GtkWidget *entry_new_topic = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(create_topic_box), entry_new_topic, TRUE, TRUE, 0);

    GtkWidget *button_send_topic = gtk_button_new_with_label("Adaugă Topic");
    gtk_box_pack_start(GTK_BOX(create_topic_box), button_send_topic, FALSE, FALSE, 0);
    g_signal_connect(button_send_topic, "clicked", G_CALLBACK(on_create_topic_clicked), entry_new_topic);

    GtkWidget *scroll_area_topics = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_area_topics), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(tab_discutii), scroll_area_topics, TRUE, TRUE, 0);

    GtkWidget *topics_list = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(scroll_area_topics), topics_list);

    g_signal_connect(topics_list, "row-activated", G_CALLBACK(on_topic_double_click), NULL);

    char *topics = list_topics();
    char *line = strtok(topics, "\n");
    while (line) {
        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_container_add(GTK_CONTAINER(row), row_box);

        GtkWidget *label_topic = gtk_label_new(line);
        gtk_box_pack_start(GTK_BOX(row_box), label_topic, TRUE, TRUE, 0);

        GtkWidget *button_bookmark = gtk_button_new_with_label("Bookmark");
        g_signal_connect(button_bookmark, "clicked", G_CALLBACK(clickBookmark), strdup(line));
        gtk_box_pack_end(GTK_BOX(row_box), button_bookmark, FALSE, FALSE, 0);

        gtk_list_box_insert(GTK_LIST_BOX(topics_list), row, -1);
        gtk_widget_show_all(row);

        line = strtok(NULL, "\n");
    }
    free(topics);

    tab_bookmarks = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(tab_bookmarks, 20);
    gtk_widget_set_margin_start(tab_bookmarks, 20);
    gtk_widget_set_margin_end(tab_bookmarks, 20);
    gtk_widget_set_margin_bottom(tab_bookmarks, 20);

    label_bookmarks = gtk_label_new("Bookmark-uri");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_bookmarks, label_bookmarks);

    box_bookmarks = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(tab_bookmarks), box_bookmarks);

    scroll_area_bookmarks = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scroll_area_bookmarks, -1, 200);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_area_bookmarks), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(box_bookmarks), scroll_area_bookmarks, TRUE, TRUE, 0);

    bookmarks_list = gtk_list_box_new();
    g_signal_connect(bookmarks_list, "row-activated", G_CALLBACK(on_topic_double_click), NULL);
    gtk_container_add(GTK_CONTAINER(scroll_area_bookmarks), bookmarks_list);

    char *bookmarks = get_user_bookmarks(current_user);
    line = strtok(bookmarks, "\n");
    while (line) {
        GtkWidget *row = gtk_list_box_row_new();
        GtkWidget *label = gtk_label_new(line);
        gtk_container_add(GTK_CONTAINER(row), label);
        gtk_list_box_insert(GTK_LIST_BOX(bookmarks_list), row, -1);
        gtk_widget_show_all(row);

        line = strtok(NULL, "\n");
    }
    free(bookmarks);

    button_back = gtk_button_new_with_label("Înapoi");
    gtk_widget_set_size_request(button_back, 400, 40);
    gtk_widget_set_margin_top(button_back, 50);
    gtk_widget_set_margin_bottom(button_back, 50);
    gtk_widget_set_halign(button_back, GTK_ALIGN_CENTER);
    g_signal_connect(button_back, "clicked", G_CALLBACK(clickBackForum), window_discutii);
    gtk_box_pack_end(GTK_BOX(main_box), button_back, FALSE, FALSE, 0);

    gtk_widget_show_all(window_discutii);
}
void windowMeniu() {
    GtkWidget *box, *button_cautare, *button_forum;
    GtkWidget *overlay;
    GtkWidget *button_box;

    window_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_main), "Meniu Principal");
    gtk_window_set_default_size(GTK_WINDOW(window_main), 1000, 600);
    gtk_window_set_position(GTK_WINDOW(window_main), GTK_WIN_POS_CENTER);
    gtk_widget_set_name(window_main, "window-main");

    overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(window_main), overlay);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), box);

    button_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_box_pack_start(GTK_BOX(box), button_box, TRUE, FALSE, 0);

    button_cautare = gtk_button_new_with_label("Cautare informatii");
    gtk_widget_set_size_request(button_cautare, 200, 50);
    gtk_widget_set_margin_start(button_cautare, 300);
    gtk_widget_set_margin_end(button_cautare, 300);
    gtk_widget_set_valign(button_cautare, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(button_box), button_cautare, FALSE, FALSE, 10);
    g_signal_connect(button_cautare, "clicked", G_CALLBACK(clickCautare), NULL);

    button_forum = gtk_button_new_with_label("Forum");
    gtk_widget_set_size_request(button_forum, 200, 50);
    gtk_widget_set_valign(button_forum, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_start(button_forum, 300);
    gtk_widget_set_margin_end(button_forum, 300);
    gtk_box_pack_start(GTK_BOX(button_box), button_forum, FALSE, FALSE, 10);
    g_signal_connect(button_forum, "clicked", G_CALLBACK(clickForum), NULL);

    g_signal_connect(window_main, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window_main);
}
void windowCautare() {
    GtkWidget *box, *button_send, *button_back, *label_instructiuni, *scrolled_window;

    window_cautare = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_cautare), "Cautare informatii");
    gtk_window_set_default_size(GTK_WINDOW(window_cautare), 1000, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window_cautare), 10);
    gtk_widget_set_name(window_cautare, "window-cautare");

    gtk_window_set_position(GTK_WINDOW(window_cautare), GTK_WIN_POS_CENTER);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window_cautare), box);

    label_instructiuni = gtk_label_new("Introdu un mesaj pentru server:");
    gtk_box_pack_start(GTK_BOX(box), label_instructiuni, FALSE, FALSE, 0);

    entry_message = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(box), entry_message, FALSE, FALSE, 0);

    button_send = gtk_button_new_with_label("Send");
    g_signal_connect(button_send, "clicked", G_CALLBACK(clickSend), NULL);
    gtk_box_pack_start(GTK_BOX(box), button_send, FALSE, FALSE, 0);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);

    text_view_response = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view_response), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view_response), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view_response);

    button_back = gtk_button_new_with_label("Back");
    g_signal_connect(button_back, "clicked", G_CALLBACK(clickBackMain), window_cautare);
    gtk_box_pack_start(GTK_BOX(box), button_back, FALSE, FALSE, 0);

    gtk_widget_show_all(window_cautare);
}
void windowForum() {
    GtkWidget *box, *grid_box, *grid, *button_discutii, *button_vezi_prieteni, *button_adauga_prieteni, *button_back;

    window_forum = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_forum), "Forum");
    gtk_window_set_default_size(GTK_WINDOW(window_forum), 1000, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window_forum), 10);
    gtk_window_set_position(GTK_WINDOW(window_forum), GTK_WIN_POS_CENTER);
    gtk_widget_set_name(window_forum, "window-forum");

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(box, 50);
    gtk_widget_set_margin_bottom(box, 50);
    gtk_widget_set_margin_start(box, 50);
    gtk_widget_set_margin_end(box, 50);
    gtk_container_add(GTK_CONTAINER(window_forum), box);

    GtkWidget *label_title = gtk_label_new("Bine ai venit pe Forum!");
    gtk_widget_set_halign(label_title, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_bottom(label_title, 20);
    gtk_box_pack_start(GTK_BOX(box), label_title, FALSE, FALSE, 0);

    grid_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(grid_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(box), grid_box, TRUE, TRUE, 0);

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 30);
    gtk_box_pack_start(GTK_BOX(grid_box), grid, FALSE, FALSE, 0);

    button_discutii = gtk_button_new_with_label("Discuții");
    gtk_widget_set_size_request(button_discutii, 250, 150);
    g_signal_connect(button_discutii, "clicked", G_CALLBACK(windowDiscutii), NULL);
    gtk_grid_attach(GTK_GRID(grid), button_discutii, 0, 0, 1, 1);

    button_vezi_prieteni = gtk_button_new_with_label("Vezi prieteni");
    gtk_widget_set_size_request(button_vezi_prieteni, 250, 150);
    g_signal_connect(button_vezi_prieteni, "clicked", G_CALLBACK(windowListaPrieteni), NULL);
    gtk_grid_attach(GTK_GRID(grid), button_vezi_prieteni, 1, 0, 1, 1);

    button_adauga_prieteni = gtk_button_new_with_label("Adaugă prieteni");
    gtk_widget_set_size_request(button_adauga_prieteni, 250, 150);
    g_signal_connect(button_adauga_prieteni, "clicked", G_CALLBACK(windowAddPrieteni), NULL);
    gtk_grid_attach(GTK_GRID(grid), button_adauga_prieteni, 2, 0, 1, 1);

    button_back = gtk_button_new_with_label("Înapoi");
    gtk_widget_set_size_request(button_back, 400, 40);
    gtk_widget_set_margin_top(button_back, 20);
    gtk_widget_set_halign(button_back, GTK_ALIGN_CENTER);
    g_signal_connect(button_back, "clicked", G_CALLBACK(clickBackMain), window_forum);
    gtk_box_pack_start(GTK_BOX(box), button_back, FALSE, FALSE, 0);

    gtk_widget_show_all(window_forum);
}
void windowTopic(const char *topic) {
    GtkWidget *window_topic, *box, *text_view, *entry_message, *button_send, *button_back;
    GtkWidget *scrolled_window;
    GtkWidget *bottom_box;

    if (GTK_IS_WIDGET(window_discutii)) {
        gtk_widget_hide(window_discutii);
    }

    window_topic = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_topic), topic);
    gtk_window_set_default_size(GTK_WINDOW(window_topic), 1000, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window_topic), 10);
    gtk_window_set_position(GTK_WINDOW(window_topic), GTK_WIN_POS_CENTER);
    gtk_widget_set_name(window_topic, "window-topic");

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window_topic), box);

    gtk_widget_set_margin_start(box, 40);
    gtk_widget_set_margin_end(box, 40);
    gtk_widget_set_margin_top(box, 30);
    gtk_widget_set_margin_bottom(box, 50);

    text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);

    char *messages = get_topic_messages(topic);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, messages, -1);
    free(messages);

    refresh_timer_id = g_timeout_add(3000, refresh_messages, text_view);

    bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box), bottom_box, FALSE, FALSE, 0);

    entry_message = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(bottom_box), entry_message, TRUE, TRUE, 0);

    button_send = gtk_button_new_with_label("Trimite Mesaj");
    gtk_box_pack_start(GTK_BOX(bottom_box), button_send, FALSE, FALSE, 0);
    g_signal_connect(button_send, "clicked", G_CALLBACK(on_add_message_to_topic), entry_message);

    g_object_set_data(G_OBJECT(button_send), "text_view", text_view);

    button_back = gtk_button_new_with_label("Back");
    gtk_box_pack_start(GTK_BOX(bottom_box), button_back, FALSE, FALSE, 0);
    g_signal_connect(button_back, "clicked", G_CALLBACK(clickBackToDiscussions), window_topic);

    gtk_widget_show_all(window_topic);
}
void windowLogin() {
    GtkWidget *grid, *label_title, *label_username, *label_password, *button_login;

    window_login = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_login), "Login");
    gtk_window_set_default_size(GTK_WINDOW(window_login), 1000, 600);
    gtk_window_set_position(GTK_WINDOW(window_login), GTK_WIN_POS_CENTER);
    gtk_widget_set_name(window_login, "window-login");

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_widget_set_hexpand(grid, TRUE);
    gtk_widget_set_vexpand(grid, TRUE);
    gtk_container_add(GTK_CONTAINER(window_login), grid);
    GtkWidget *spacer = gtk_label_new(NULL);
    gtk_grid_attach(GTK_GRID(grid), spacer, 0, 0, 2, 1);

    label_username = gtk_label_new("Username:");
    gtk_widget_set_halign(label_username, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid), label_username, 0, 1, 1, 1);

    entry_username = gtk_entry_new();
    gtk_widget_set_hexpand(entry_username, TRUE);
    gtk_widget_set_size_request(entry_username, 300, -1);
    gtk_grid_attach(GTK_GRID(grid), entry_username, 1, 1, 1, 1);

    label_password = gtk_label_new("Password:");
    gtk_widget_set_halign(label_password, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid), label_password, 0, 2, 1, 1);

    entry_password = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE);
    gtk_widget_set_hexpand(entry_password, TRUE);
    gtk_widget_set_size_request(entry_password, 300, -1);
    gtk_grid_attach(GTK_GRID(grid), entry_password, 1, 2, 1, 1);

    button_login = gtk_button_new_with_label("Login");
    gtk_widget_set_margin_top(button_login, 20);
    gtk_grid_attach(GTK_GRID(grid), button_login, 0, 3, 2, 1);

    g_signal_connect(button_login, "clicked", G_CALLBACK(on_login_clicked), NULL);

    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);

    gtk_widget_show_all(window_login);
}

//BUTOANE
void clickAcceptFriend(GtkWidget *widget, gpointer data) {
    const char *friend_name = (const char *)data;

    if (strlen(friend_name) == 0) {
        fprintf(stderr, "[client] Eroare: Nume prieten invalid.\n");
        return;
    }
    if (strlen(current_user) == 0) {
        fprintf(stderr, "[client] Eroare: Utilizatorul logat nu este setat.\n");
        return;
    }

    char mesajPrefixat[256];
    snprintf(mesajPrefixat, sizeof(mesajPrefixat), "serverACCEPT:%s:%s", current_user, friend_name);

    if (write(sd, mesajPrefixat, strlen(mesajPrefixat)) <= 0) {
        perror("[client] Eroare la trimiterea comenzii ACCEPT.\n");
        return;
    }

    char raspuns[1024];
    bzero(raspuns, sizeof(raspuns));
    int bytes = read(sd, raspuns, sizeof(raspuns) - 1);
    if (bytes < 0) {
        perror("[client] Eroare la citirea raspunsului de la server.\n");
        return;
    }

    raspuns[bytes] = '\0';

    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_add_prieteni),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "Confirmare: %s",
                                               raspuns);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    gtk_widget_destroy(window_add_prieteni);
    windowAddPrieteni();
}
void clickCautare(GtkWidget *widget, gpointer data) {
    gtk_widget_hide(window_main);
    windowCautare();
}
void clickBackMain(GtkWidget *widget, gpointer data) {
    GtkWidget *current_window = GTK_WIDGET(data);
    gtk_widget_hide(current_window);
    gtk_widget_show_all(window_main);
}
void clickBackForum(GtkWidget *widget, gpointer data) {
    GtkWidget *current_window = GTK_WIDGET(data);
    gtk_widget_hide(current_window);
    gtk_widget_show_all(window_forum);
}
void clickSend(GtkWidget *widget, gpointer data) {
    const char *mesajEntry = gtk_entry_get_text(GTK_ENTRY(entry_message));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_response));
    char raspuns[1024];
    char raspuns_final[8192] = "";
    int bytes;

    if (strlen(mesajEntry) == 0) {
        gtk_text_buffer_set_text(buffer, "Mesajul este gol!", -1);
        return;
    }

    char mesajPrefixat[1024];
    snprintf(mesajPrefixat, sizeof(mesajPrefixat), "serverINFO:%s", mesajEntry);

    if (write(sd, mesajPrefixat, strlen(mesajPrefixat)) <= 0) {
        perror("[client] Eroare la write() catre server.\n");
        close(sd);
        exit(errno);
    }

    if (strcmp(mesajEntry, "exit") == 0) {
        gtk_text_buffer_set_text(buffer, "Conexiunea a fost inchisa.", -1);
        close(sd);
        gtk_main_quit();
        return;
    }

    do {
        bzero(raspuns, sizeof(raspuns));
        bytes = read(sd, raspuns, sizeof(raspuns) - 1);
        if (bytes < 0) {
            perror("[client] Eroare la read() de la server.\n");
            close(sd);
            exit(errno);
        }

        raspuns[bytes] = '\0';
        strcat(raspuns_final, raspuns);

    } while (bytes == sizeof(raspuns) - 1);

    gtk_text_buffer_set_text(buffer, raspuns_final, -1);
}
void clickSendPrieteni(GtkWidget *widget, gpointer data) {
    const char *mesajEntry = gtk_entry_get_text(GTK_ENTRY(entry_message));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_response));
    char raspuns[1024];
    char raspuns_final[8192] = "";
    int bytes;

    if (strlen(mesajEntry) == 0) {
        gtk_text_buffer_set_text(buffer, "Mesajul este gol!", -1);
        return;
    }

    char mesajPrefixat[1024];
    snprintf(mesajPrefixat, sizeof(mesajPrefixat), "serverADD:%s:%s", current_user, mesajEntry);

    if (write(sd, mesajPrefixat, strlen(mesajPrefixat)) <= 0) {
        perror("[client] Eroare la write() catre server.\n");
        close(sd);
        exit(errno);
    }

    if (strcmp(mesajEntry, "exit") == 0) {
        gtk_text_buffer_set_text(buffer, "Conexiunea a fost inchisa.", -1);
        close(sd);
        gtk_main_quit();
        return;
    }

    do {
        bzero(raspuns, sizeof(raspuns));
        bytes = read(sd, raspuns, sizeof(raspuns) - 1);
        if (bytes < 0) {
            perror("[client] Eroare la read() de la server.\n");
            close(sd);
            exit(errno);
        }

        raspuns[bytes] = '\0';
        strcat(raspuns_final, raspuns);

    } while (bytes == sizeof(raspuns) - 1);

    gtk_text_buffer_set_text(buffer, raspuns_final, -1);

    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_add_prieteni),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"Cererea de prietenie pentru '%s' a fost înregistrată!", mesajEntry);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
void clickForum(GtkWidget *widget, gpointer data) {
    gtk_widget_hide(window_main);
    windowForum();
}
void clickRemoveFriend(GtkWidget *widget, gpointer data) {
    const char *friend_name = (const char *)data;

    if (strlen(friend_name) == 0) {
        fprintf(stderr, "[client] Eroare: Nume prieten invalid.\n");
        return;
    }

    char request[256];
    snprintf(request, sizeof(request), "serverREMOVE_FRIEND:%s:%s", current_user, friend_name);

    if (write(sd, request, strlen(request)) <= 0) {
        perror("[client] Eroare la trimiterea cererii REMOVE_FRIEND către server.\n");
        return;
    }

    char response[1024];
    bzero(response, sizeof(response));
    int bytes = read(sd, response, sizeof(response) - 1);
    if (bytes < 0) {
        perror("[client] Eroare la citirea răspunsului de la server.\n");
        return;
    }

    response[bytes] = '\0';

    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_lista_prieteni),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "%s", response);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    gtk_widget_destroy(window_lista_prieteni);
    windowListaPrieteni();
}
void clickBackToDiscussions(GtkWidget *widget, gpointer data) {
    GtkWidget *current_window = GTK_WIDGET(data);

    if (refresh_timer_id > 0) {
        g_source_remove(refresh_timer_id);
        refresh_timer_id = 0;
    }

    gtk_widget_hide(current_window);
    gtk_widget_show_all(window_discutii);
}
void clickBookmark(GtkWidget *widget, gpointer data) {
    const char *topic = (const char *)data;

    char request[256];
    snprintf(request, sizeof(request), "serverBOOKMARK:%s:%s", current_user, topic);

    if (write(sd, request, strlen(request)) <= 0) {
        perror("[client] Eroare la trimiterea cererii BOOKMARK către server.\n");
        return;
    }

    char response[1024];
    bzero(response, sizeof(response));
    int bytes = read(sd, response, sizeof(response) - 1);
    if (bytes < 0) {
        perror("[client] Eroare la citirea răspunsului de la server.\n");
        return;
    }

    response[bytes] = '\0';

    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_discutii),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "%s", response);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

char *create_topic(const char *topic) {
    char request[256];
    snprintf(request, sizeof(request), "serverCREATE_DISCUSSION:%s", topic);
    char buffer[4096];
    int bytes;

    if (write(sd, request, strlen(request)) <= 0) {
        perror("[client] Eroare la trimiterea cererii CREATE_DISCUSSION.\n");
        return strdup("Eroare la comunicarea cu serverul.");
    }

    bzero(buffer, sizeof(buffer));
    bytes = read(sd, buffer, sizeof(buffer) - 1);
    if (bytes < 0) {
        perror("[client] Eroare la citirea răspunsului de la server.\n");
        return strdup("Eroare la primirea datelor de la server.");
    }

    buffer[bytes] = '\0';
    return strdup(buffer);
}
char *list_topics() {
    const char *request = "serverLIST_DISCUSSIONS:";
    char buffer[4096];
    int bytes;

    if (write(sd, request, strlen(request)) <= 0) {
        perror("[client] Eroare la trimiterea cererii LIST_DISCUSSIONS.\n");
        return strdup("Eroare la comunicarea cu serverul.");
    }

    bzero(buffer, sizeof(buffer));
    bytes = read(sd, buffer, sizeof(buffer) - 1);
    if (bytes < 0) {
        perror("[client] Eroare la citirea răspunsului de la server.\n");
        return strdup("Eroare la primirea datelor de la server.");
    }

    buffer[bytes] = '\0';
    return strdup(buffer);
}
char *add_message_to_topic(const char *topic, const char *message) {
    char request[2048];
    snprintf(request, sizeof(request), "serverADD_MESSAGE:%s:%s:%s", current_user, topic, message);

    char buffer[4096];
    int bytes;

    if (write(sd, request, strlen(request)) <= 0) {
        perror("[client] Eroare la trimiterea cererii ADD_MESSAGE.\n");
        return strdup("Eroare la comunicarea cu serverul.");
    }

    bzero(buffer, sizeof(buffer));
    bytes = read(sd, buffer, sizeof(buffer) - 1);
    if (bytes < 0) {
        perror("[client] Eroare la citirea răspunsului de la server.\n");
        return strdup("Eroare la primirea datelor de la server.");
    }

    buffer[bytes] = '\0';
    return strdup(buffer);
}

void on_create_topic_clicked(GtkWidget *widget, gpointer data) {
    const char *topic_name = gtk_entry_get_text(GTK_ENTRY(data));
    if (strlen(topic_name) == 0) {
        printf("Numele topicului este gol.\n");
        return;
    }
    char *response = create_topic(topic_name);
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_discutii),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "%s", response);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    free(response);
}
void on_add_message_to_topic(GtkWidget *widget, gpointer data) {
    const char *message = gtk_entry_get_text(GTK_ENTRY(data));
    if (strlen(message) == 0) {
        printf("Mesajul este gol.\n");
        return;
    }

    const char *topic = gtk_window_get_title(GTK_WINDOW(gtk_widget_get_toplevel(widget)));
    char *response = add_message_to_topic(topic, message);

    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "%s", response);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    free(response);

    char *updated_messages = get_topic_messages(topic);
    GtkWidget *text_view = g_object_get_data(G_OBJECT(widget), "text_view");
    if (text_view) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        gtk_text_buffer_set_text(buffer, updated_messages, -1);
        free(updated_messages);
    } else {
        g_warning("text_view nu a fost găsit în datele butonului 'Trimite Mesaj'.");
        free(updated_messages);
    }

    gtk_entry_set_text(GTK_ENTRY(data), "");
}
void on_topic_double_click(GtkListBox *list_box, GtkListBoxRow *row, gpointer user_data) {
    GtkWidget *row_box = gtk_bin_get_child(GTK_BIN(row));
    if (!row_box) {
        g_warning("Rândul nu are un widget copil.");
        return;
    }

    const char *topic = NULL;

    if (GTK_IS_BOX(row_box)) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(row_box));
        GtkWidget *label_topic = GTK_WIDGET(g_list_nth_data(children, 0));
        g_list_free(children);

        if (GTK_IS_LABEL(label_topic)) {
            topic = gtk_label_get_text(GTK_LABEL(label_topic));
        }
    } else if (GTK_IS_LABEL(row_box)) {
        topic = gtk_label_get_text(GTK_LABEL(row_box));
    }

    if (topic) {
        char *trimmed_topic = g_strdup(topic);
        g_strstrip(trimmed_topic);

        if (strlen(trimmed_topic) > 0) {
            windowTopic(trimmed_topic);
        } else {
            g_warning("Topic invalid după curățare.");
        }

        g_free(trimmed_topic);
    } else {
        g_warning("Eticheta nu are text sau structura rândului este invalidă.");
    }
}

char *get_topic_messages(const char *topic) {
    char request[256];
    snprintf(request, sizeof(request), "serverVIEW_TOPIC:%s", topic);
    char buffer[4096];
    int bytes;

    if (write(sd, request, strlen(request)) <= 0) {
        perror("[client] Eroare la trimiterea cererii VIEW_TOPIC.\n");
        return strdup("Eroare la comunicarea cu serverul.");
    }

    bzero(buffer, sizeof(buffer));
    bytes = read(sd, buffer, sizeof(buffer) - 1);
    if (bytes < 0) {
        perror("[client] Eroare la citirea răspunsului de la server.\n");
        return strdup("Eroare la primirea datelor de la server.");
    }

    buffer[bytes] = '\0';
    return strdup(buffer);
}
char *get_user_bookmarks(const char *username) {
    char request[256];
    snprintf(request, sizeof(request), "serverGET_BOOKMARKS:%s", username);
    char buffer[4096];
    int bytes;

    if (write(sd, request, strlen(request)) <= 0) {
        perror("[client] Eroare la trimiterea cererii GET_BOOKMARKS către server.\n");
        return strdup("Eroare la comunicarea cu serverul.");
    }

    bzero(buffer, sizeof(buffer));
    bytes = read(sd, buffer, sizeof(buffer) - 1);
    if (bytes < 0) {
        perror("[client] Eroare la citirea răspunsului de la server.\n");
        return strdup("Eroare la primirea datelor de la server.");
    }

    buffer[bytes] = '\0';
    return strdup(buffer);
}


// ---------------------------------- UI ----------------------------------

int main(int argc, char *argv[]) {
    struct sockaddr_in server;

    if (argc != 3) {
        printf("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    int port = atoi(argv[2]);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[client] Eroare la socket().\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        perror("[client] Eroare la connect().\n");
        return errno;
    }

    gtk_init(&argc, &argv);
    CSS();
    windowLogin();
    gtk_main();

    close(sd);
    return 0;
}
