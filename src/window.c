#undef I3__FILE__
#define I3__FILE__ "window.c"
/*
 * vim:ts=4:sw=4:expandtab
 *
 * i3 - an improved dynamic tiling window manager
 * © 2009 Michael Stapelberg and contributors (see also: LICENSE)
 *
 * window.c: Updates window attributes (X11 hints/properties).
 *
 */
#include "all.h"

/*
 * Updates the WM_CLASS (consisting of the class and instance) for the
 * given window.
 *
 */
void window_update_class(i3Window *win, xcb_get_property_reply_t *prop, bool before_mgmt) {
    if (prop == NULL || xcb_get_property_value_length(prop) == 0) {
        DLOG("WM_CLASS not set.\n");
        FREE(prop);
        return;
    }

    /* We cannot use asprintf here since this property contains two
     * null-terminated strings (for compatibility reasons). Instead, we
     * use strdup() on both strings */
    const size_t prop_length = xcb_get_property_value_length(prop);
    char *new_class = xcb_get_property_value(prop);
    const size_t class_class_index = strnlen(new_class, prop_length) + 1;

    FREE(win->class_instance);
    FREE(win->class_class);

    win->class_instance = sstrndup(new_class, prop_length);
    if (class_class_index < prop_length)
        win->class_class = sstrndup(new_class + class_class_index, prop_length - class_class_index);
    else
        win->class_class = NULL;
    LOG("WM_CLASS changed to %s (instance), %s (class)\n",
        win->class_instance, win->class_class);

    if (before_mgmt) {
        free(prop);
        return;
    }

    run_assignments(win);

    free(prop);
}

/*
 * Updates the name by using _NET_WM_NAME (encoded in UTF-8) for the given
 * window. Further updates using window_update_name_legacy will be ignored.
 *
 */
void window_update_name(i3Window *win, xcb_get_property_reply_t *prop, bool before_mgmt) {
    if (prop == NULL || xcb_get_property_value_length(prop) == 0) {
        DLOG("_NET_WM_NAME not specified, not changing\n");
        FREE(prop);
        return;
    }

    i3string_free(win->name);
    win->name = i3string_from_utf8_with_length(xcb_get_property_value(prop),
                                               xcb_get_property_value_length(prop));
    if (win->title_format != NULL)
        ewmh_update_visible_name(win->id, i3string_as_utf8(window_parse_title_format(win)));
    win->name_x_changed = true;
    LOG("_NET_WM_NAME changed to \"%s\"\n", i3string_as_utf8(win->name));

    win->uses_net_wm_name = true;

    if (before_mgmt) {
        free(prop);
        return;
    }

    run_assignments(win);

    free(prop);
}

/*
 * Updates the name by using WM_NAME (encoded in COMPOUND_TEXT). We do not
 * touch what the client sends us but pass it to xcb_image_text_8. To get
 * proper unicode rendering, the application has to use _NET_WM_NAME (see
 * window_update_name()).
 *
 */
void window_update_name_legacy(i3Window *win, xcb_get_property_reply_t *prop, bool before_mgmt) {
    if (prop == NULL || xcb_get_property_value_length(prop) == 0) {
        DLOG("WM_NAME not set (_NET_WM_NAME is what you want anyways).\n");
        FREE(prop);
        return;
    }

    /* ignore update when the window is known to already have a UTF-8 name */
    if (win->uses_net_wm_name) {
        free(prop);
        return;
    }

    i3string_free(win->name);
    win->name = i3string_from_utf8_with_length(xcb_get_property_value(prop),
                                               xcb_get_property_value_length(prop));
    if (win->title_format != NULL)
        ewmh_update_visible_name(win->id, i3string_as_utf8(window_parse_title_format(win)));

    LOG("WM_NAME changed to \"%s\"\n", i3string_as_utf8(win->name));
    LOG("Using legacy window title. Note that in order to get Unicode window "
        "titles in i3, the application has to set _NET_WM_NAME (UTF-8)\n");

    win->name_x_changed = true;

    if (before_mgmt) {
        free(prop);
        return;
    }

    run_assignments(win);

    free(prop);
}

/*
 * Updates the CLIENT_LEADER (logical parent window).
 *
 */
void window_update_leader(i3Window *win, xcb_get_property_reply_t *prop) {
    if (prop == NULL || xcb_get_property_value_length(prop) == 0) {
        DLOG("CLIENT_LEADER not set on window 0x%08x.\n", win->id);
        win->leader = XCB_NONE;
        FREE(prop);
        return;
    }

    xcb_window_t *leader = xcb_get_property_value(prop);
    if (leader == NULL) {
        free(prop);
        return;
    }

    DLOG("Client leader changed to %08x\n", *leader);

    win->leader = *leader;

    free(prop);
}

/*
 * Updates the TRANSIENT_FOR (logical parent window).
 *
 */
void window_update_transient_for(i3Window *win, xcb_get_property_reply_t *prop) {
    if (prop == NULL || xcb_get_property_value_length(prop) == 0) {
        DLOG("TRANSIENT_FOR not set on window 0x%08x.\n", win->id);
        win->transient_for = XCB_NONE;
        FREE(prop);
        return;
    }

    xcb_window_t transient_for;
    if (!xcb_icccm_get_wm_transient_for_from_reply(&transient_for, prop)) {
        free(prop);
        return;
    }

    DLOG("Transient for changed to 0x%08x (window 0x%08x)\n", transient_for, win->id);

    win->transient_for = transient_for;

    free(prop);
}

/*
 * Updates the _NET_WM_STRUT_PARTIAL (reserved pixels at the screen edges)
 *
 */
void window_update_strut_partial(i3Window *win, xcb_get_property_reply_t *prop) {
    if (prop == NULL || xcb_get_property_value_length(prop) == 0) {
        DLOG("_NET_WM_STRUT_PARTIAL not set.\n");
        FREE(prop);
        return;
    }

    uint32_t *strut;
    if (!(strut = xcb_get_property_value(prop))) {
        free(prop);
        return;
    }

    DLOG("Reserved pixels changed to: left = %d, right = %d, top = %d, bottom = %d\n",
         strut[0], strut[1], strut[2], strut[3]);

    win->reserved = (struct reservedpx){strut[0], strut[1], strut[2], strut[3]};

    free(prop);
}

/*
 * Updates the WM_WINDOW_ROLE
 *
 */
void window_update_role(i3Window *win, xcb_get_property_reply_t *prop, bool before_mgmt) {
    if (prop == NULL || xcb_get_property_value_length(prop) == 0) {
        DLOG("WM_WINDOW_ROLE not set.\n");
        FREE(prop);
        return;
    }

    char *new_role;
    sasprintf(&new_role, "%.*s", xcb_get_property_value_length(prop),
              (char *)xcb_get_property_value(prop));
    FREE(win->role);
    win->role = new_role;
    LOG("WM_WINDOW_ROLE changed to \"%s\"\n", win->role);

    if (before_mgmt) {
        free(prop);
        return;
    }

    run_assignments(win);

    free(prop);
}

/*
 * Updates the _NET_WM_WINDOW_TYPE property.
 *
 */
void window_update_type(i3Window *window, xcb_get_property_reply_t *reply) {
    xcb_atom_t new_type = xcb_get_preferred_window_type(reply);
    if (new_type == XCB_NONE) {
        DLOG("cannot read _NET_WM_WINDOW_TYPE from window.\n");
        return;
    }

    window->window_type = new_type;
    LOG("_NET_WM_WINDOW_TYPE changed to %i.\n", window->window_type);

    run_assignments(window);
}

/*
 * Updates the WM_HINTS (we only care about the input focus handling part).
 *
 */
void window_update_hints(i3Window *win, xcb_get_property_reply_t *prop, bool *urgency_hint) {
    if (urgency_hint != NULL)
        *urgency_hint = false;

    if (prop == NULL || xcb_get_property_value_length(prop) == 0) {
        DLOG("WM_HINTS not set.\n");
        FREE(prop);
        return;
    }

    xcb_icccm_wm_hints_t hints;

    if (!xcb_icccm_get_wm_hints_from_reply(&hints, prop)) {
        DLOG("Could not get WM_HINTS\n");
        free(prop);
        return;
    }

    if (hints.flags & XCB_ICCCM_WM_HINT_INPUT) {
        win->doesnt_accept_focus = !hints.input;
        LOG("WM_HINTS.input changed to \"%d\"\n", hints.input);
    }

    if (urgency_hint != NULL)
        *urgency_hint = (xcb_icccm_wm_hints_get_urgency(&hints) != 0);

    free(prop);
}

/*
 * Updates the MOTIF_WM_HINTS. The container's border style should be set to
 * `motif_border_style' if border style is not BS_NORMAL.
 *
 * i3 only uses this hint when it specifies a window should have no
 * title bar, or no decorations at all, which is how most window managers
 * handle it.
 *
 * The EWMH spec intended to replace Motif hints with _NET_WM_WINDOW_TYPE, but
 * it is still in use by popular widget toolkits such as GTK+ and Java AWT.
 *
 */
void window_update_motif_hints(i3Window *win, xcb_get_property_reply_t *prop, border_style_t *motif_border_style) {
/* This implementation simply mirrors Gnome's Metacity. Official
     * documentation of this hint is nowhere to be found.
     * For more information see:
     * https://people.gnome.org/~tthurman/docs/metacity/xprops_8h-source.html
     * http://stackoverflow.com/questions/13787553/detect-if-a-x11-window-has-decorations
     */
#define MWM_HINTS_DECORATIONS (1 << 1)
#define MWM_DECOR_ALL (1 << 0)
#define MWM_DECOR_BORDER (1 << 1)
#define MWM_DECOR_TITLE (1 << 3)

    if (motif_border_style != NULL)
        *motif_border_style = BS_NORMAL;

    if (prop == NULL || xcb_get_property_value_length(prop) == 0) {
        FREE(prop);
        return;
    }

    /* The property consists of an array of 5 uint64_t's. The first value is a bit
     * mask of what properties the hint will specify. We are only interested in
     * MWM_HINTS_DECORATIONS because it indicates that the second value of the
     * array tells us which decorations the window should have, each flag being
     * a particular decoration. */
    uint64_t *motif_hints = (uint64_t *)xcb_get_property_value(prop);

    if (motif_border_style != NULL && motif_hints[0] & MWM_HINTS_DECORATIONS) {
        if (motif_hints[1] & MWM_DECOR_ALL || motif_hints[1] & MWM_DECOR_TITLE)
            *motif_border_style = BS_NORMAL;
        else if (motif_hints[1] & MWM_DECOR_BORDER)
            *motif_border_style = BS_PIXEL;
        else
            *motif_border_style = BS_NONE;
    }

    FREE(prop);

#undef MWM_HINTS_DECORATIONS
#undef MWM_DECOR_ALL
#undef MWM_DECOR_BORDER
#undef MWM_DECOR_TITLE
}

/*
 * Returns the window title considering the current title format.
 * If no format is set, this will simply return the window's name.
 *
 */
i3String *window_parse_title_format(i3Window *win) {
    /* We need to ensure that we only escape the window title if pango
     * is used by the current font. */
    const bool is_markup = font_is_pango();

    char *format = win->title_format;
    if (format == NULL)
        return i3string_copy(win->name);

    /* We initialize these lazily so we only escape them if really necessary. */
    const char *escaped_title = NULL;
    const char *escaped_class = NULL;
    const char *escaped_instance = NULL;

    /* We have to first iterate over the string to see how much buffer space
     * we need to allocate. */
    int buffer_len = strlen(format) + 1;
    for (char *walk = format; *walk != '\0'; walk++) {
        if (STARTS_WITH(walk, "%title")) {
            if (escaped_title == NULL)
                escaped_title = win->name == NULL ? "" : i3string_as_utf8(is_markup ? i3string_escape_markup(win->name) : win->name);

            buffer_len = buffer_len - strlen("%title") + strlen(escaped_title);
            walk += strlen("%title") - 1;
        } else if (STARTS_WITH(walk, "%class")) {
            if (escaped_class == NULL)
                escaped_class = is_markup ? g_markup_escape_text(win->class_class, -1) : win->class_class;

            buffer_len = buffer_len - strlen("%class") + strlen(escaped_class);
            walk += strlen("%class") - 1;
        } else if (STARTS_WITH(walk, "%instance")) {
            if (escaped_instance == NULL)
                escaped_instance = is_markup ? g_markup_escape_text(win->class_instance, -1) : win->class_instance;

            buffer_len = buffer_len - strlen("%instance") + strlen(escaped_instance);
            walk += strlen("%instance") - 1;
        }
    }

    /* Now we can parse the format string. */
    char buffer[buffer_len];
    char *outwalk = buffer;
    for (char *walk = format; *walk != '\0'; walk++) {
        if (*walk != '%') {
            *(outwalk++) = *walk;
            continue;
        }

        if (STARTS_WITH(walk + 1, "title")) {
            outwalk += sprintf(outwalk, "%s", escaped_title);
            walk += strlen("title");
        } else if (STARTS_WITH(walk + 1, "class")) {
            outwalk += sprintf(outwalk, "%s", escaped_class);
            walk += strlen("class");
        } else if (STARTS_WITH(walk + 1, "instance")) {
            outwalk += sprintf(outwalk, "%s", escaped_instance);
            walk += strlen("instance");
        } else {
            *(outwalk++) = *walk;
        }
    }
    *outwalk = '\0';

    i3String *formatted = i3string_from_utf8(buffer);
    i3string_set_markup(formatted, is_markup);
    return formatted;
}
