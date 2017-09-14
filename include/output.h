/*
 * vim:ts=4:sw=4:expandtab
 *
 * i3 - an improved dynamic tiling window manager
 * © 2009 Michael Stapelberg and contributors (see also: LICENSE)
 *
 * output.c: Output (monitor) related functions.
 *
 */
#pragma once

#include <config.h>

/**
 * Returns the output container below the given output container.
 *
 */
Con *output_get_content(Con *output);

/**
 * Returns an 'output' corresponding to one of left/right/down/up or a specific
 * output name.
 *
 */
Output *get_output_from_string(Output *current_output, const char *output_str);

/**
 * Retrieves the primary name of an output.
 *
 */
char *output_primary_name(Output *output);

/**
 * Returns the output for the given con.
 *
 */
Output *get_output_for_con(Con *con);

/**
 * Iterates over all outputs and pushes sticky windows to the currently visible
 * workspace on that output.
 *
 */
void output_push_sticky_windows(Con *to_focus);
