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

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gprintf.h>

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

static void
print_version(void)
{
    g_print("Custom Linux Installer v1.0.0\n");
    g_print("Built with GTK %d.%d.%d\n",
            gtk_get_major_version(),
            gtk_get_minor_version(),
            gtk_get_micro_version());
}

static void
print_help(const char *program_name)
{
    g_print("Usage: %s [OPTIONS]\n\n", program_name);
    g_print("Options:\n");
    g_print("  -h, --help        Show this help message\n");
    g_print("  -v, --version     Show version information\n");
    g_print("  -o, --oem         Run in OEM mode\n");
    g_print("  -f, --fullscreen  Run in fullscreen mode\n");
    g_print("  -d, --debug       Enable debug output\n");
    g_print("\nModes:\n");
    g_print("  Normal mode:      Default installation mode\n");
    g_print("  OEM mode:         Prepare system for OEM deployment\n");
}

static void
debug_log(const char *format, ...)
{
    if (!config.debug)
        return;

    va_list args;
    va_start(args, format);
    g_print("[DEBUG] ");
    g_vprintf(format, args);
    g_print("\n");
    va_end(args);
}

static gint
handle_local_options(GApplication *app,
                     GVariantDict *options,
                     gpointer user_data)
{
    // Handle --version
    if (g_variant_dict_lookup(options, "version", "b", &config.show_version)) {
        if (config.show_version) {
            print_version();
            return 0; // Exit successfully
        }
    }

    // Handle --help
    if (g_variant_dict_lookup(options, "help", "b", &config.show_help)) {
        if (config.show_help) {
            print_help(g_get_prgname());
            return 0; // Exit successfully
        }
    }

    // Handle --oem
    if (g_variant_dict_lookup(options, "oem", "b", NULL)) {
        config.mode = MODE_OEM;
        debug_log("OEM mode enabled");
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

static void
setup_window_for_mode(GtkWindow *window)
{
    const char *title;
    const char *css_class;

    switch (config.mode) {
        case MODE_OEM:
            title = "Linux Installer - OEM Mode";
            css_class = "oem-mode";
            debug_log("Setting up OEM mode interface");
            break;
        case MODE_RECOVERY:
            title = "Linux Installer - Recovery Mode";
            css_class = "recovery-mode";
            debug_log("Setting up recovery mode interface");
            break;
        default:
            title = "Linux Installer";
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

static void
create_installer_ui(GtkWindow *window)
{
    GtkWidget *box, *label, *button_box, *next_button, *cancel_button;

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(box, 24);
    gtk_widget_set_margin_bottom(box, 24);
    gtk_widget_set_margin_start(box, 24);
    gtk_widget_set_margin_end(box, 24);

    // Create mode-specific content
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
            label = gtk_label_new("Welcome to the Linux Installer\n\n"
                                "This wizard will guide you through the installation process.");
            break;
    }

    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), label);

    // Create button box
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);

    cancel_button = gtk_button_new_with_label("Cancel");
    next_button = gtk_button_new_with_label("Next");
    gtk_widget_add_css_class(next_button, "suggested-action");

    gtk_box_append(GTK_BOX(button_box), cancel_button);
    gtk_box_append(GTK_BOX(button_box), next_button);

    // Add some spacing and append button box
    gtk_widget_set_margin_top(button_box, 24);
    gtk_box_append(GTK_BOX(box), button_box);

    gtk_window_set_child(window, box);

    debug_log("UI created for mode: %d", config.mode);
}

static void
activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window;

    debug_log("Application activating");

    window = gtk_application_window_new(app);
    setup_window_for_mode(GTK_WINDOW(window));
    create_installer_ui(GTK_WINDOW(window));

    gtk_window_present(GTK_WINDOW(window));

    debug_log("Main window presented");
}

int
main(int argc, char *argv[])
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.installer", G_APPLICATION_DEFAULT_FLAGS);

    // Add command line options
    g_application_add_main_option(G_APPLICATION(app),
                                  "version", 'v', G_OPTION_FLAG_NONE,
                                  G_OPTION_ARG_NONE,
                                  "Show version information", NULL);

    g_application_add_main_option(G_APPLICATION(app),
                                  "help", 'h', G_OPTION_FLAG_NONE,
                                  G_OPTION_ARG_NONE,
                                  "Show help information", NULL);

    g_application_add_main_option(G_APPLICATION(app),
                                  "oem", 'o', G_OPTION_FLAG_NONE,
                                  G_OPTION_ARG_NONE,
                                  "Run in OEM mode", NULL);

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
