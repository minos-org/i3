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
 * Iterates over all outputs and pushes sticky windows to the currently visible
 * workspace on that output.
 *
 */
void output_push_sticky_windows(Con *to_focus);
