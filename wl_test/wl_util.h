#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include <xkbcommon/xkbcommon.h>

#define wl_array_for_each_uint32_t(pos, array) \
	for (pos = (uint32_t *)(array)->data;  \
	     (const char *)pos < ((const char *)(array)->data + (array)->size); (pos)++)

struct frame {
	struct wl_buffer *buffer;
	uint32_t *data;
};

struct wl_state {
	/* wayland state */
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_surface *surface;
	struct frame frame;
	ssize_t frame_size;
	struct wl_callback *callback;
	struct wl_keyboard *keyboard;
	struct wl_shm *shm;
	struct wl_shm_pool *shm_pool;
	struct wl_seat *seat;
	struct xdg_wm_base *xdg_wm_base;
	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *xdg_toplevel;

	/* xkb keyboard state*/
	struct xkb_context *xkb_context;
	struct xkb_keymap *xkb_keymap;
	struct xkb_state *xkb_state;
	int32_t rate;
	int32_t delay;
	uint32_t key_frame;
	xkb_keysym_t sym;

	/* frame pace */
	float offset;
	uint32_t last_frame;
};