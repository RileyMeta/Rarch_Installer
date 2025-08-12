// File   : main.c
// Project: Rarch Installer - Riley's (customized) Arch Installer
//
// Description:
// A custom GTK4 GUI Rarch Linux installer written in C with minimal libraries.
//
// Author: Riley Ava
// Date  : 2025-08-11
//
// License: Mozilla Public Licenese 2.0 - MPL-2
// Toolkit: GTK4 (Lesser General Public License - LGPL License)
//
// Copyright (c) 2025 Riley Ava
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to use,
// copy, modify, and distribute the Software under the terms of the Mozilla Public
// License Version 2.0 (the "License"). You may not use this file except in compliance
// with the License. You may obtain a copy of the License at:
//
//     https://www.mozilla.org/en/MPL/2.0/
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glib.h>

typedef enum {
    MODE_NORMAL,
    MODE_OEM,
    MODE_RECOVERY
} InstallerMode;

typedef struct {
    InstallerMode mode;
    gboolean fullscreen;
    gboolean debug;
    gboolean show_version;
    gboolean show_help;
} InstallerConfig;

static InstallerConfig config = {
    .mode = MODE_NORMAL,
    .fullscreen = FALSE,
    .debug = FALSE,
    .show_version = FALSE,
    .show_help = FALSE
};

static void print_version(void) {
    g_print("Rarch Linux Installer v1.0.0\n");
    g_print("Built with GTK %d.%d.%d\n",
            gtk_get_major_version(),
            gtk_get_minor_version(),
            gtk_get_micro_version());
}

static void debug_log(const char *format, ...) {
    if (!config.debug)
        return;

    va_list args;
    va_start(args, format);
    g_print("[DEBUG] ");
    g_vprintf(format, args);
    g_print("\n");
    va_end(args);
}

static gint handle_local_options(GApplication *app,
                                 GVariantDict *options,
                                 gpointer user_data) {
    // Handle --version
    if (g_variant_dict_lookup(options, "version", "b", &config.show_version)) {
        if (config.show_version) {
            print_version();
            return 0; // Exit successfully
        }
    }

    // Handle --oem
    if (g_variant_dict_lookup(options, "oem", "b", NULL)) {
        config.mode = MODE_OEM;
        debug_log("OEM mode enabled");
    }

    // Handle --recovery
    if (g_variant_dict_lookup(options, "recovery", "b", NULL)) {
        config.mode = MODE_RECOVERY;
        debug_log("Recovery mode enabled");
    }

    // Handle --fullscreen
    g_variant_dict_lookup(options, "fullscreen", "b", &config.fullscreen);
    if (config.fullscreen) {
        debug_log("Fullscreen mode enabled");
    }

    // Handle --debug
    g_variant_dict_lookup(options, "debug", "b", &config.debug);
    if (config.debug) {
        g_print("[DEBUG] Debug mode enabled\n");
    }

    return -1; // Continue with normal startup
}

static void setup_window_for_mode(GtkWindow *window) {
    const char *title;
    const char *css_class;

    switch (config.mode) {
        case MODE_OEM:
            title = "Rarch Installer - OEM Mode";
            css_class = "oem-mode";
            debug_log("Setting up OEM mode interface");
            break;
        case MODE_RECOVERY:
            title = "Rarch Installer - Recovery Mode";
            css_class = "recovery-mode";
            debug_log("Setting up recovery mode interface");
            break;
        default:
            title = "Rarch Installer";
            css_class = "normal-mode";
            debug_log("Setting up normal mode interface");
            break;
    }

    gtk_window_set_title(window, title);
    gtk_widget_add_css_class(GTK_WIDGET(window), css_class);

    if (config.fullscreen) {
        gtk_window_fullscreen(window);
        debug_log("Window set to fullscreen");
    } else {
        gtk_window_set_default_size(window, 800, 600);
    }
}

// Page management structure
typedef struct {
    GtkStack *stack;
    GtkButton *back_button;
    GtkButton *next_button;
    GtkWindow *window;
    int current_page;
    int total_pages;
} PageManager;

static PageManager page_manager;

// Page IDs
static const char* PAGE_WELCOME = "welcome";
static const char* PAGE_DISK_SELECTION = "disk_selection";
static const char* PAGE_PARTITIONING = "partitioning";
static const char* PAGE_USER_SETUP = "user_setup";
static const char* PAGE_INSTALLATION = "installation";
static const char* PAGE_COMPLETE = "complete";

static void update_navigation_buttons(void);
static void on_next_clicked(GtkButton *button, gpointer user_data);
static void on_back_clicked(GtkButton *button, gpointer user_data);

static GtkWidget* create_welcome_page(void) {
    GtkWidget *box, *label;

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);

    switch (config.mode) {
        case MODE_OEM:
            label = gtk_label_new("OEM Installation Mode\n\n"
                                "This will prepare the system for OEM deployment.\n"
                                "The system will be configured for first-boot setup.");
            break;
        case MODE_RECOVERY:
            label = gtk_label_new("Recovery Mode\n\n"
                                "System recovery and repair options.");
            break;
        default:
            label = gtk_label_new("Welcome to the Rarch Linux Installer\n\n"
                                "This wizard will guide you through the installation process.");
            break;
    }

    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), label);

    debug_log("Welcome page created");
    return box;
}

static GtkWidget* create_disk_selection_page(void) {
    GtkWidget *box, *label, *listbox;

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);

    label = gtk_label_new("Select Installation Disk\n\n"
                         "Choose the disk where you want to install the system.");
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), label);

    // Create a simple listbox for disk selection
    listbox = gtk_list_box_new();
    gtk_widget_set_size_request(listbox, -1, 200);

    // Add some example disks (in real implementation, scan for actual disks)
    GtkWidget *row1 = gtk_list_box_row_new();
    GtkWidget *disk1_label = gtk_label_new("/dev/sda - 500 GB SSD");
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row1), disk1_label);
    gtk_list_box_append(GTK_LIST_BOX(listbox), row1);

    GtkWidget *row2 = gtk_list_box_row_new();
    GtkWidget *disk2_label = gtk_label_new("/dev/sdb - 1 TB HDD");
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row2), disk2_label);
    gtk_list_box_append(GTK_LIST_BOX(listbox), row2);

    gtk_box_append(GTK_BOX(box), listbox);

    debug_log("Disk selection page created");
    return box;
}

static GtkWidget* create_partitioning_page(void) {
    GtkWidget *box, *label, *radio1, *radio2, *radio3;

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);

    label = gtk_label_new("Partitioning Options\n\n"
                         "How would you like to partition the disk?");
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), label);

    radio1 = gtk_check_button_new_with_label("Erase disk and install");
    radio2 = gtk_check_button_new_with_label("Install alongside existing OS");
    radio3 = gtk_check_button_new_with_label("Manual partitioning");

    gtk_check_button_set_group(GTK_CHECK_BUTTON(radio2), GTK_CHECK_BUTTON(radio1));
    gtk_check_button_set_group(GTK_CHECK_BUTTON(radio3), GTK_CHECK_BUTTON(radio1));

    gtk_check_button_set_active(GTK_CHECK_BUTTON(radio1), TRUE);

    gtk_box_append(GTK_BOX(box), radio1);
    gtk_box_append(GTK_BOX(box), radio2);
    gtk_box_append(GTK_BOX(box), radio3);

    debug_log("Partitioning page created");
    return box;
}

static GtkWidget* create_user_setup_page(void) {
    GtkWidget *box, *label, *grid, *name_entry, *username_entry, *password_entry;

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);

    label = gtk_label_new("Create User Account\n\n"
                         "Set up your user account for the new system.");
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), label);

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);

    // Full name
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Full Name:"), 0, 0, 1, 1);
    name_entry = gtk_entry_new();
    gtk_widget_set_size_request(name_entry, 250, -1);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 1, 1);

    // Username
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Username:"), 0, 1, 1, 1);
    username_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), username_entry, 1, 1, 1, 1);

    // Password
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Password:"), 0, 2, 1, 1);
    password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_grid_attach(GTK_GRID(grid), password_entry, 1, 2, 1, 1);

    gtk_box_append(GTK_BOX(box), grid);

    debug_log("User setup page created");
    return box;
}

static GtkWidget* create_installation_page(void) {
    GtkWidget *box, *label, *progress;

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    label = gtk_label_new("Installing System\n\n"
                         "Please wait while the system is being installed...");
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), label);

    progress = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), 0.45);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), "Installing packages...");
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), TRUE);
    gtk_widget_set_size_request(progress, 400, -1);
    gtk_widget_set_halign(progress, GTK_ALIGN_CENTER);

    gtk_box_append(GTK_BOX(box), progress);

    debug_log("Installation page created");
    return box;
}

static GtkWidget* create_complete_page(void) {
    GtkWidget *box, *label;

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    label = gtk_label_new("Installation Complete!\n\n"
                         "The system has been successfully installed.\n"
                         "Please restart your computer to boot into the new system.");
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), label);

    debug_log("Complete page created");
    return box;
}

static void on_next_clicked(GtkButton *button, gpointer user_data) {
    const char *current_visible = gtk_stack_get_visible_child_name(page_manager.stack);

    debug_log("Next clicked from page: %s", current_visible);

    if (g_strcmp0(current_visible, PAGE_WELCOME) == 0) {
        gtk_stack_set_visible_child_name(page_manager.stack, PAGE_DISK_SELECTION);
        page_manager.current_page = 1;
    }
    else if (g_strcmp0(current_visible, PAGE_DISK_SELECTION) == 0) {
        gtk_stack_set_visible_child_name(page_manager.stack, PAGE_PARTITIONING);
        page_manager.current_page = 2;
    }
    else if (g_strcmp0(current_visible, PAGE_PARTITIONING) == 0) {
        gtk_stack_set_visible_child_name(page_manager.stack, PAGE_USER_SETUP);
        page_manager.current_page = 3;
    }
    else if (g_strcmp0(current_visible, PAGE_USER_SETUP) == 0) {
        gtk_stack_set_visible_child_name(page_manager.stack, PAGE_INSTALLATION);
        page_manager.current_page = 4;
    }
    else if (g_strcmp0(current_visible, PAGE_INSTALLATION) == 0) {
        gtk_stack_set_visible_child_name(page_manager.stack, PAGE_COMPLETE);
        page_manager.current_page = 5;
    }

    update_navigation_buttons();
}

static void on_back_clicked(GtkButton *button, gpointer user_data) {
    const char *current_visible = gtk_stack_get_visible_child_name(page_manager.stack);

    debug_log("Back clicked from page: %s", current_visible);

    if (g_strcmp0(current_visible, PAGE_DISK_SELECTION) == 0) {
        gtk_stack_set_visible_child_name(page_manager.stack, PAGE_WELCOME);
        page_manager.current_page = 0;
    }
    else if (g_strcmp0(current_visible, PAGE_PARTITIONING) == 0) {
        gtk_stack_set_visible_child_name(page_manager.stack, PAGE_DISK_SELECTION);
        page_manager.current_page = 1;
    }
    else if (g_strcmp0(current_visible, PAGE_USER_SETUP) == 0) {
        gtk_stack_set_visible_child_name(page_manager.stack, PAGE_PARTITIONING);
        page_manager.current_page = 2;
    }
    else if (g_strcmp0(current_visible, PAGE_INSTALLATION) == 0) {
        gtk_stack_set_visible_child_name(page_manager.stack, PAGE_USER_SETUP);
        page_manager.current_page = 3;
    }
    else if (g_strcmp0(current_visible, PAGE_COMPLETE) == 0) {
        gtk_stack_set_visible_child_name(page_manager.stack, PAGE_INSTALLATION);
        page_manager.current_page = 4;
    }

    update_navigation_buttons();
}

static void update_navigation_buttons(void) {
    const char *current_visible = gtk_stack_get_visible_child_name(page_manager.stack);

    // Update back button
    if (g_strcmp0(current_visible, PAGE_WELCOME) == 0) {
        gtk_button_set_label(page_manager.back_button, "Exit");
        g_signal_handlers_disconnect_by_func(page_manager.back_button,
                                           G_CALLBACK(on_back_clicked), NULL);
        g_signal_connect_swapped(page_manager.back_button, "clicked",
                               G_CALLBACK(gtk_window_close), page_manager.window);
    } else {
        gtk_button_set_label(page_manager.back_button, "Back");
        g_signal_handlers_disconnect_by_func(page_manager.back_button,
                                           G_CALLBACK(gtk_window_close), NULL);
        g_signal_connect(page_manager.back_button, "clicked",
                        G_CALLBACK(on_back_clicked), NULL);
    }

    // Update next button
    if (g_strcmp0(current_visible, PAGE_COMPLETE) == 0) {
        gtk_button_set_label(page_manager.next_button, "Restart");
        gtk_widget_remove_css_class(GTK_WIDGET(page_manager.next_button), "suggested-action");
        gtk_widget_add_css_class(GTK_WIDGET(page_manager.next_button), "destructive-action");
    } else if (g_strcmp0(current_visible, PAGE_INSTALLATION) == 0) {
        gtk_widget_set_sensitive(GTK_WIDGET(page_manager.next_button), FALSE);
        gtk_button_set_label(page_manager.next_button, "Installing...");
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(page_manager.next_button), TRUE);
        gtk_button_set_label(page_manager.next_button, "Next");
        gtk_widget_remove_css_class(GTK_WIDGET(page_manager.next_button), "destructive-action");
        gtk_widget_add_css_class(GTK_WIDGET(page_manager.next_button), "suggested-action");
    }

    debug_log("Navigation buttons updated for page: %s", current_visible);
}

static void create_installer_ui(GtkWindow *window) {
    GtkWidget *main_box, *stack, *button_box, *back_button, *next_button;

    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Create the stack for pages
    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 300);

    // Add all pages to the stack
    gtk_stack_add_named(GTK_STACK(stack), create_welcome_page(), PAGE_WELCOME);
    gtk_stack_add_named(GTK_STACK(stack), create_disk_selection_page(), PAGE_DISK_SELECTION);
    gtk_stack_add_named(GTK_STACK(stack), create_partitioning_page(), PAGE_PARTITIONING);
    gtk_stack_add_named(GTK_STACK(stack), create_user_setup_page(), PAGE_USER_SETUP);
    gtk_stack_add_named(GTK_STACK(stack), create_installation_page(), PAGE_INSTALLATION);
    gtk_stack_add_named(GTK_STACK(stack), create_complete_page(), PAGE_COMPLETE);

    // Set margins for the stack
    gtk_widget_set_margin_top(stack, 24);
    gtk_widget_set_margin_bottom(stack, 24);
    gtk_widget_set_margin_start(stack, 24);
    gtk_widget_set_margin_end(stack, 24);

    gtk_box_append(GTK_BOX(main_box), stack);

    // Create button box
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    gtk_widget_set_margin_top(button_box, 0);
    gtk_widget_set_margin_bottom(button_box, 24);
    gtk_widget_set_margin_start(button_box, 24);
    gtk_widget_set_margin_end(button_box, 24);

    back_button = gtk_button_new_with_label("Exit");
    next_button = gtk_button_new_with_label("Next");
    gtk_widget_add_css_class(next_button, "suggested-action");

    gtk_box_append(GTK_BOX(button_box), back_button);
    gtk_box_append(GTK_BOX(button_box), next_button);

    gtk_box_append(GTK_BOX(main_box), button_box);

    // Initialize page manager
    page_manager.stack = GTK_STACK(stack);
    page_manager.back_button = GTK_BUTTON(back_button);
    page_manager.next_button = GTK_BUTTON(next_button);
    page_manager.window = window;
    page_manager.current_page = 0;
    page_manager.total_pages = 6;

    // Connect signals
    g_signal_connect(next_button, "clicked", G_CALLBACK(on_next_clicked), NULL);
    g_signal_connect_swapped(back_button, "clicked",
                           G_CALLBACK(gtk_window_close), window);

    // Set initial page
    gtk_stack_set_visible_child_name(GTK_STACK(stack), PAGE_WELCOME);
    update_navigation_buttons();

    gtk_window_set_child(window, main_box);

    debug_log("UI created for mode: %d with stack navigation", config.mode);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;

    debug_log("Application activating");

    window = gtk_application_window_new(app);
    setup_window_for_mode(GTK_WINDOW(window));
    create_installer_ui(GTK_WINDOW(window));

    gtk_window_present(GTK_WINDOW(window));

    debug_log("Main window presented");
}

int main(int argc, char *argv[]) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.github.RileyMeta.Rarch_Installer", G_APPLICATION_DEFAULT_FLAGS);

    // Add command line options (--help is automatic)
    g_application_add_main_option(G_APPLICATION(app),
                                  "version", 'V', G_OPTION_FLAG_NONE,
                                  G_OPTION_ARG_NONE,
                                  "Show version information", NULL);

    g_application_add_main_option(G_APPLICATION(app),
                                  "oem", 'O', G_OPTION_FLAG_NONE,
                                  G_OPTION_ARG_NONE,
                                  "Run in OEM mode", NULL);

    g_application_add_main_option(G_APPLICATION(app),
                                "recovery", 'R', G_OPTION_FLAG_NONE,
                                G_OPTION_ARG_NONE,
                                "Show help information", NULL);

    g_application_add_main_option(G_APPLICATION(app),
                                  "fullscreen", 'f', G_OPTION_FLAG_NONE,
                                  G_OPTION_ARG_NONE,
                                  "Run in fullscreen mode", NULL);

    g_application_add_main_option(G_APPLICATION(app),
                                  "debug", 'd', G_OPTION_FLAG_NONE,
                                  G_OPTION_ARG_NONE,
                                  "Enable debug output", NULL);

    // Connect signals
    g_signal_connect(app, "handle-local-options",
                     G_CALLBACK(handle_local_options), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
