/*
 * zrunner theme engine
 *
 * zrunner.c
 *
 * Copyright (C) 2006 Quinn Storm <livinglatexkali@gmail.com> (original legacy theme engine)
 * Copyright (C) 2006 Varun <varunratnakar@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/* zrunner engine */
#include <emerald.h>
#include <engine.h>
#include <zrunner_icon.h>

#define SECT "zrunner_settings"

#define FLTS(flt) \
    load_float_setting(f,&((private_fs *)ws->fs_act->engine_fs)->flt,"active_" #flt ,SECT);\
    load_float_setting(f,&((private_fs *)ws->fs_inact->engine_fs)->flt,"inactive_" #flt ,SECT);

/*
 * settings structs
 */
typedef struct _private_fs
{
    alpha_color window_halo;
    alpha_color window_halo_1;
    alpha_color window_halo_2;
    alpha_color window_halo_3;
    alpha_color window_halo_4;
    alpha_color window_halo_5;
    alpha_color window_halo_6;
    alpha_color separator_line;
    alpha_color separator_line_1;
    alpha_color separator_line_2;
    alpha_color separator_line_3;
    alpha_color separator_line_4;
    alpha_color separator_line_5;
    alpha_color separator_line_6;
    alpha_color title_color;
    alpha_color title_color_1;
    alpha_color title_color_2;
    alpha_color title_color_3;
    alpha_color title_color_4;
    alpha_color title_color_5;
    alpha_color title_color_6;
} private_fs;

typedef struct _private_ws
{
    gboolean round_top_left;
    gboolean round_top_right;
    gboolean round_bottom_left;
    gboolean round_bottom_right;
    double	corner_radius;
    gchar       *extra_apps;
    gchar      **extra_apps_1;
    gchar      **extra_apps_2;
    gchar      **extra_apps_3;
    gchar      **extra_apps_4;
    gchar      **extra_apps_5;
    gchar      **extra_apps_6;
} private_ws;

void get_meta_info (EngineMetaInfo * emi)
{
    guint8 *pixbuf_data;

    emi->version = g_strdup("0.2");
    emi->description = g_strdup(_("Multiple gradients with somewhat glassy features too"));
    /* old themes are marked still compatible for now */
    emi->last_compat = g_strdup("0.0");

    pixbuf_data = g_memdup(ZRUNNER_ICON_PIXEL_DATA,
                           ZRUNNER_ICON_ROWSTRIDE * ZRUNNER_ICON_HEIGHT);
    emi->icon = gdk_pixbuf_new_from_data(pixbuf_data, GDK_COLORSPACE_RGB,
                                         (ZRUNNER_ICON_BYTES_PER_PIXEL != 3),
                                         8,
                                         ZRUNNER_ICON_WIDTH,
                                         ZRUNNER_ICON_HEIGHT,
                                         ZRUNNER_ICON_ROWSTRIDE,
                                         (GdkPixbufDestroyNotify) g_free,
                                         pixbuf_data);
}

void
draw_closed_curve (cairo_t *cr,
        double  x,
        double  y,
        double  w,
        double  h)
{
    cairo_move_to (cr, x, y);
    cairo_line_to (cr, x + w, y);
    cairo_line_to (cr, x + w, y + h);
    cairo_line_to (cr, x, y + h);
    cairo_line_to (cr, x, y);
}

void
draw_filled_closed_curve (cairo_t *cr,
        double        x,
        double        y,
        double        w,
        double        h,
        alpha_color * c0)
{
    draw_closed_curve (cr, x, y, w, h);
    cairo_set_source_alpha_color (cr, c0);
    cairo_fill (cr);
}

static gint get_real_pos(window_settings * ws, gint tobj, decor_t * d)
{
    gint width = d->width;
    gint base = ws->left_space;
    switch(d->tobj_item_state[tobj])
    {
        case 1:
            base = (width - ws->left_space - ws->right_space - d->tobj_size[0] - d->tobj_size[2]) / 2
                 - d->tobj_size[1]/2 + ws->left_space + d->tobj_size[0];
            break;
        case 2:
            base = width - ws->right_space - d->tobj_size[2];
            break;
        case 3:
            return -1;
        default:
            break;
    }
    return base + d->tobj_item_pos[tobj];
}

void engine_draw_frame(decor_t * d, cairo_t * cr)
{
    double x1, y1, x2, y2,
           top_title_height;
    int top, title_width = 0, title_height = 0, title_pos;
    gboolean is_extra_app_1 = FALSE;
    gboolean is_extra_app_2 = FALSE;
    gboolean is_extra_app_3 = FALSE;
    gboolean is_extra_app_4 = FALSE;
    gboolean is_extra_app_5 = FALSE;
    gboolean is_extra_app_6 = FALSE;
    guint i;
    alpha_color title_color;
    alpha_color separator_line;
    alpha_color window_halo;

    frame_settings * fs = d->fs;
    private_fs * pfs = fs->engine_fs;
    window_settings * ws = fs->ws;
    private_ws * pws = ws->engine_ws;
    top = ws->win_extents.top + ws->titlebar_height;

    x1 = ws->left_space - ws->win_extents.left;
    y1 = ws->top_space - ws->win_extents.top;
    x2 = d->width - ws->right_space + ws->win_extents.right;
    y2 = d->height - ws->bottom_space + ws->win_extents.bottom;

    int corners =
        ((pws->round_top_left)     ? CORNER_TOPLEFT     : 0) |
        ((pws->round_top_right)    ? CORNER_TOPRIGHT    : 0) |
        ((pws->round_bottom_left)  ? CORNER_BOTTOMLEFT  : 0) |
        ((pws->round_bottom_right) ? CORNER_BOTTOMRIGHT : 0);

	/* maximize work-a-round */
	if (d->state & (WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY |
                WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY))
        corners = 0;

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_line_width (cr, 0.0);

    top_title_height    = top;

    /* main top titlebar */
    rounded_rectangle (cr,
            x1 + 1,
            y1 + 1,
            x2 - x1 - 2,
            top,
            (CORNER_TOPLEFT | CORNER_TOPRIGHT) & corners, ws,
            pws->corner_radius);
    cairo_clip(cr);

    if (pws->extra_apps_1 != NULL) {
        for (i = 0; i < g_strv_length(pws->extra_apps_1); ++i) {
            if (!g_strcmp0(d->window_class, pws->extra_apps_1[i])) {
                    is_extra_app_1 = TRUE;
            }
        }
    }
    if (pws->extra_apps_2 != NULL) {
        for (i = 0; i < g_strv_length(pws->extra_apps_2); ++i) {
            if (!g_strcmp0(d->window_class, pws->extra_apps_2[i])) {
                    is_extra_app_2 = TRUE;
            }
        }
    }
    if (pws->extra_apps_3 != NULL) {
        for (i = 0; i < g_strv_length(pws->extra_apps_3); ++i) {
            if (!g_strcmp0(d->window_class, pws->extra_apps_3[i])) {
                    is_extra_app_3 = TRUE;
            }
        }
    }
    if (pws->extra_apps_4 != NULL) {
        for (i = 0; i < g_strv_length(pws->extra_apps_4); ++i) {
            if (!g_strcmp0(d->window_class, pws->extra_apps_4[i])) {
                    is_extra_app_4 = TRUE;
            }
        }
    }
    if (pws->extra_apps_5 != NULL) {
        for (i = 0; i < g_strv_length(pws->extra_apps_5); ++i) {
            if (!g_strcmp0(d->window_class, pws->extra_apps_5[i])) {
                    is_extra_app_5 = TRUE;
            }
        }
    }
    if (pws->extra_apps_6 != NULL) {
        for (i = 0; i < g_strv_length(pws->extra_apps_6); ++i) {
            if (!g_strcmp0(d->window_class, pws->extra_apps_6[i])) {
                    is_extra_app_6 = TRUE;
            }
        }
    }

    title_color = pfs->title_color;
    separator_line = pfs->separator_line;
    window_halo = pfs->window_halo;

    if (is_extra_app_1) {
        title_color = pfs->title_color_1;
        separator_line = pfs->separator_line_1;
        window_halo = pfs->window_halo_1;
    } else if (is_extra_app_2) {
        title_color = pfs->title_color_2;
        separator_line = pfs->separator_line_2;
        window_halo = pfs->window_halo_2;
    } else if (is_extra_app_3) {
        title_color = pfs->title_color_3;
        separator_line = pfs->separator_line_3;
        window_halo = pfs->window_halo_3;
    } else if (is_extra_app_4) {
        title_color = pfs->title_color_4;
        separator_line = pfs->separator_line_4;
        window_halo = pfs->window_halo_4;
    } else if (is_extra_app_5) {
        title_color = pfs->title_color_5;
        separator_line = pfs->separator_line_5;
        window_halo = pfs->window_halo_5;
    } else if (is_extra_app_6) {
        title_color = pfs->title_color_6;
        separator_line = pfs->separator_line_6;
        window_halo = pfs->window_halo_6;
    }
    draw_filled_closed_curve(cr,
        x1 + 1,
        y1 + 1,
        x2 - x1 - 2,
        top_title_height,
        &title_color);

    cairo_reset_clip(cr);

    /* ======= NEW LAYER ====== */
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_line_width (cr, 1.0);

    /* titlebar separator */
    if(pfs->separator_line.alpha > 0) {
       cairo_set_source_alpha_color(cr, &separator_line);
       cairo_move_to (cr, x1 + 1, y1 + top - 0.5);
       cairo_rel_line_to (cr, x2 - x1 - 2, 0.0);
       cairo_stroke (cr);
    }

    /* halo */
    rounded_rectangle (cr,
            x1 + 0.5, y1 + 0.5,
            x2 - x1 - 1.0, y2 - y1 - 1.0,
            (CORNER_TOPLEFT | CORNER_TOPRIGHT | CORNER_BOTTOMLEFT |
             CORNER_BOTTOMRIGHT) & corners, ws,
            pws->corner_radius);

    cairo_set_source_alpha_color (cr, &window_halo);
    cairo_stroke (cr);
}

void parse_extra_apps(private_ws * pws, gchar * extra_apps)
{
    guint i;
    gchar **extra_groups = g_strsplit(extra_apps, ",", 0);
    for (i = 0; i < g_strv_length(extra_groups); ++i) {
        if (i == 0) {
            pws->extra_apps_1 = g_strsplit(extra_groups[i], "|", 0);
        }
        if (i == 1) {
            pws->extra_apps_2 = g_strsplit(extra_groups[i], "|", 0);
        }
        if (i == 2) {
            pws->extra_apps_3 = g_strsplit(extra_groups[i], "|", 0);
        }
        if (i == 3) {
            pws->extra_apps_4 = g_strsplit(extra_groups[i], "|", 0);
        }
        if (i == 4) {
            pws->extra_apps_5 = g_strsplit(extra_groups[i], "|", 0);
        }
        if (i == 5) {
            pws->extra_apps_6 = g_strsplit(extra_groups[i], "|", 0);
        }
    }
}

void load_engine_settings(GKeyFile * f, window_settings * ws)
{
    private_ws * pws = ws->engine_ws;

    /* color settings */
    PFACS(window_halo);
    PFACS(window_halo_1);
    PFACS(window_halo_2);
    PFACS(window_halo_3);
    PFACS(window_halo_4);
    PFACS(window_halo_5);
    PFACS(window_halo_6);
    PFACS(separator_line);
    PFACS(separator_line_1);
    PFACS(separator_line_2);
    PFACS(separator_line_3);
    PFACS(separator_line_4);
    PFACS(separator_line_5);
    PFACS(separator_line_6);
    PFACS(title_color);
    PFACS(title_color_1);
    PFACS(title_color_2);
    PFACS(title_color_3);
    PFACS(title_color_4);
    PFACS(title_color_5);
    PFACS(title_color_6);

    private_fs * pfs = ws->fs_act->engine_fs;

    pfs = ws->fs_inact->engine_fs;

    load_bool_setting(f, &pws->round_top_left, "round_top_left", SECT);
    load_bool_setting(f, &pws->round_top_right, "round_top_right", SECT);
    load_bool_setting(f, &pws->round_bottom_left, "round_bottom_left", SECT);
    load_bool_setting(f, &pws->round_bottom_right, "round_bottom_right", SECT);
    load_float_setting(f, &pws->corner_radius, "radius", SECT);
    load_string_setting(f, &pws->extra_apps, "extra_apps", SECT);
    parse_extra_apps(pws, pws->extra_apps);
}

void init_engine(window_settings * ws)
{
    private_fs * pfs;
    private_ws * pws;

    pws = g_malloc0(sizeof(private_ws));
    ws->engine_ws = pws;
    pws->round_top_left = TRUE;
    pws->round_top_right = TRUE;
    pws->round_bottom_left = TRUE;
    pws->round_bottom_right = TRUE;
    pws->corner_radius = 5.0;
    pws->extra_apps = g_strdup("Atom|Gnome-terminal,discord|something");
    parse_extra_apps(pws, pws->extra_apps);

    pfs = g_malloc0(sizeof(private_fs));
    ws->fs_act->engine_fs = pfs;

    ACOLOR(window_halo, 0.8, 0.8, 0.8, 0.8);
    ACOLOR(window_halo_1, 0.8, 0.8, 0.8, 0.8);
    ACOLOR(window_halo_2, 0.8, 0.8, 0.8, 0.8);
    ACOLOR(window_halo_3, 0.8, 0.8, 0.8, 0.8);
    ACOLOR(window_halo_4, 0.8, 0.8, 0.8, 0.8);
    ACOLOR(window_halo_5, 0.8, 0.8, 0.8, 0.8);
    ACOLOR(window_halo_6, 0.8, 0.8, 0.8, 0.8);

    ACOLOR(separator_line, 0.0, 0.0, 0.0, 0.0);
    ACOLOR(separator_line_1, 0.0, 0.0, 0.0, 0.0);
    ACOLOR(separator_line_2, 0.0, 0.0, 0.0, 0.0);
    ACOLOR(separator_line_3, 0.0, 0.0, 0.0, 0.0);
    ACOLOR(separator_line_4, 0.0, 0.0, 0.0, 0.0);
    ACOLOR(separator_line_5, 0.0, 0.0, 0.0, 0.0);
    ACOLOR(separator_line_6, 0.0, 0.0, 0.0, 0.0);

    ACOLOR(title_color, 0.2, 0.9, 0.3, 0.9);
    ACOLOR(title_color_1, 0.2, 0.9, 0.3, 0.9);
    ACOLOR(title_color_2, 0.2, 0.9, 0.3, 0.9);
    ACOLOR(title_color_3, 0.2, 0.9, 0.3, 0.9);
    ACOLOR(title_color_4, 0.2, 0.9, 0.3, 0.9);
    ACOLOR(title_color_5, 0.2, 0.9, 0.3, 0.9);
    ACOLOR(title_color_6, 0.2, 0.9, 0.3, 0.9);

    pfs = g_malloc0(sizeof(private_fs));
    ws->fs_inact->engine_fs = pfs;

    ACOLOR(window_halo, 0.8, 0.8, 0.8, 0.8);
    ACOLOR(window_halo_1, 0.8, 0.8, 0.8, 0.8);
    ACOLOR(window_halo_2, 0.8, 0.8, 0.8, 0.8);
    ACOLOR(window_halo_3, 0.8, 0.8, 0.8, 0.8);
    ACOLOR(window_halo_4, 0.8, 0.8, 0.8, 0.8);
    ACOLOR(window_halo_5, 0.8, 0.8, 0.8, 0.8);
    ACOLOR(window_halo_6, 0.8, 0.8, 0.8, 0.8);

    ACOLOR(separator_line, 0.0, 0.0, 0.0, 0.0);
    ACOLOR(separator_line_1, 0.0, 0.0, 0.0, 0.0);
    ACOLOR(separator_line_2, 0.0, 0.0, 0.0, 0.0);
    ACOLOR(separator_line_3, 0.0, 0.0, 0.0, 0.0);
    ACOLOR(separator_line_4, 0.0, 0.0, 0.0, 0.0);
    ACOLOR(separator_line_5, 0.0, 0.0, 0.0, 0.0);
    ACOLOR(separator_line_6, 0.0, 0.0, 0.0, 0.0);

    ACOLOR(title_color, 0.2, 0.9, 0.3, 0.9);
    ACOLOR(title_color_1, 0.2, 0.9, 0.3, 0.9);
    ACOLOR(title_color_2, 0.2, 0.9, 0.3, 0.9);
    ACOLOR(title_color_3, 0.2, 0.9, 0.3, 0.9);
    ACOLOR(title_color_4, 0.2, 0.9, 0.3, 0.9);
    ACOLOR(title_color_5, 0.2, 0.9, 0.3, 0.9);
    ACOLOR(title_color_6, 0.2, 0.9, 0.3, 0.9);

}

void fini_engine(window_settings * ws)
{
    g_free(ws->fs_act->engine_fs);
    g_free(ws->fs_inact->engine_fs);
}

void layout_corners_frame(GtkWidget * vbox)
{
    GtkWidget * hbox;
    GtkWidget * junk;

    junk = gtk_check_button_new_with_label(_("Round Top Left Corner"));
    gtk_box_pack_startC(vbox, junk, FALSE, FALSE, 0);
    register_setting(junk, ST_BOOL, SECT, "round_top_left");

    junk = gtk_check_button_new_with_label(_("Round Top Right Corner"));
    gtk_box_pack_startC(vbox, junk, FALSE, FALSE, 0);
    register_setting(junk, ST_BOOL, SECT, "round_top_right");

    junk = gtk_check_button_new_with_label(_("Round Bottom Left Corner"));
    gtk_box_pack_startC(vbox, junk, FALSE, FALSE, 0);
    register_setting(junk, ST_BOOL, SECT, "round_bottom_left");

    junk = gtk_check_button_new_with_label(_("Round Bottom Right Corner"));
    gtk_box_pack_startC(vbox, junk, FALSE, FALSE, 0);
    register_setting(junk, ST_BOOL, SECT, "round_bottom_right");

#if GTK_CHECK_VERSION(3, 0, 0)
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
#else
    hbox = gtk_hbox_new(FALSE, 2);
#endif
    gtk_box_pack_startC(vbox, hbox, FALSE, FALSE, 0);

    gtk_box_pack_startC(hbox, gtk_label_new(_("Rounding Radius")), FALSE, FALSE, 0);

    junk = scaler_new(0, 20, 0.5);
    gtk_box_pack_startC(hbox, junk, TRUE, TRUE, 0);
    register_setting(junk, ST_FLOAT, SECT, "radius");
}

void layout_apps(GtkWidget * vbox)
{
    GtkWidget * hbox;
    GtkWidget * junk;

    junk = gtk_label_new("Extra Apps");
    gtk_box_pack_startC(vbox, junk, FALSE, FALSE, 0);

    junk = gtk_entry_new();
    gtk_box_pack_startC(vbox, junk, FALSE, FALSE, 0);
    register_setting(junk, ST_ACTIVE_STRING, SECT, "extra_apps");
}

void my_engine_settings(GtkWidget * hbox,  gboolean active)
{
    GtkWidget * vbox;
    GtkWidget * junk;
    GtkWidget * scroller;

#if GTK_CHECK_VERSION(3, 0, 0)
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
#else
    vbox = gtk_vbox_new(FALSE, 2);
#endif
    gtk_box_pack_startC(hbox, vbox, TRUE, TRUE, 0);
    gtk_box_pack_startC(vbox, gtk_label_new(active ? "Active Window" : "Inactive Window"), FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(3, 2, 0)
    gtk_box_pack_startC(vbox, gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);
#else
    gtk_box_pack_startC(vbox, gtk_hseparator_new(), FALSE, FALSE, 0);
#endif
    scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller),
            GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_startC(vbox, scroller, TRUE, TRUE, 0);

    table_new(3, FALSE, FALSE);

#if GTK_CHECK_VERSION(3, 8, 0)
    gtk_container_add(GTK_CONTAINER(scroller), GTK_WIDGET(get_current_table()));
#else
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroller), GTK_WIDGET(get_current_table()));
#endif

    make_labels(_("Colors"));
    table_append_separator();
    ACAV("Title Color", "title_color", SECT);
    ACAV("Title Color 1", "title_color_1", SECT);
    ACAV("Title Color 2", "title_color_2", SECT);
    ACAV("Title Color 3", "title_color_3", SECT);
    ACAV("Title Color 4", "title_color_4", SECT);
    ACAV("Title Color 5", "title_color_5", SECT);
    ACAV("Title Color 6", "title_color_6", SECT);

    table_append_separator();
    ACAV("Separator", "separator_line", SECT);
    ACAV("Separator 1", "separator_line_1", SECT);
    ACAV("Separator 2", "separator_line_2", SECT);
    ACAV("Separator 3", "separator_line_3", SECT);
    ACAV("Separator 4", "separator_line_4", SECT);
    ACAV("Separator 5", "separator_line_5", SECT);
    ACAV("Separator 6", "separator_line_6", SECT);

    table_append_separator();
    ACAV("Outline", "window_halo", SECT);
    ACAV("Outline 1", "window_halo_1", SECT);
    ACAV("Outline 2", "window_halo_2", SECT);
    ACAV("Outline 3", "window_halo_3", SECT);
    ACAV("Outline 4", "window_halo_4", SECT);
    ACAV("Outline 5", "window_halo_5", SECT);
    ACAV("Outline 6", "window_halo_6", SECT);

}

void layout_engine_colors(GtkWidget * vbox)
{
    GtkWidget * hbox;

#if GTK_CHECK_VERSION(3, 0, 0)
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
#else
    hbox = gtk_hbox_new(FALSE, 2);
#endif
    gtk_box_pack_startC(vbox, hbox, TRUE, TRUE, 0);
    my_engine_settings(hbox, TRUE);
#if GTK_CHECK_VERSION(3, 2, 0)
    gtk_box_pack_startC(hbox, gtk_separator_new (GTK_ORIENTATION_VERTICAL), FALSE, FALSE, 0);
#else
    gtk_box_pack_startC(hbox, gtk_vseparator_new(), FALSE, FALSE, 0);
#endif
    my_engine_settings(hbox, FALSE);
}

void layout_engine_settings(GtkWidget * vbox)
{
    GtkWidget * note;

    note = gtk_notebook_new();
    gtk_box_pack_startC(vbox, note, TRUE, TRUE, 0);
    layout_engine_colors(build_notebook_page(_("Colors"), note));
    layout_corners_frame(build_notebook_page(_("Frame"), note));
    layout_apps(build_notebook_page("Apps", note));
}
