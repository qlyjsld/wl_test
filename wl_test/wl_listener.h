#pragma once
#include "wl_util.h"
#include "wl_keyboard_listener.h"

static void xdg_wm_base_handle_ping(void *data, struct xdg_wm_base *xdg_wm_base,
				    uint32_t serial)
{
	xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
	.ping = xdg_wm_base_handle_ping,
};

static void seat_handle_capabilities(void *data, struct wl_seat *wl_seat,
				     uint32_t capabilities)
{
	struct wl_state *wl_state = (struct wl_state *)data;
	if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
		wl_state->keyboard = wl_seat_get_keyboard(wl_state->seat);
		wl_keyboard_add_listener(wl_state->keyboard, &keyboard_listener,
					 wl_state);
	}
}

static void seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name)
{
	//
}

static const struct wl_seat_listener seat_listener = {
	.capabilities = seat_handle_capabilities,
	.name = seat_handle_name,
};

static void registry_handle_global(void *data, struct wl_registry *wl_registry,
				   uint32_t name, const char *interface, uint32_t version)
{
	struct wl_state *wl_state = (struct wl_state *)data;

	if (!strcmp(interface, wl_compositor_interface.name))
		wl_state->compositor = (struct wl_compositor *)wl_registry_bind(
			wl_registry, name, &wl_compositor_interface, version);

	if (!strcmp(interface, wl_shm_interface.name))
		wl_state->shm = (struct wl_shm *)wl_registry_bind(
			wl_registry, name, &wl_shm_interface, version);

	if (!strcmp(interface, xdg_wm_base_interface.name)) {
		wl_state->xdg_wm_base = (struct xdg_wm_base *)wl_registry_bind(
			wl_registry, name, &xdg_wm_base_interface, version);
		xdg_wm_base_add_listener(wl_state->xdg_wm_base, &xdg_wm_base_listener,
					 wl_state);
	}

	if (!strcmp(interface, wl_seat_interface.name)) {
		wl_state->seat = (struct wl_seat *)wl_registry_bind(
			wl_registry, name, &wl_seat_interface, version);
		wl_seat_add_listener(wl_state->seat, &seat_listener, wl_state);
	}

	// printf("name: %d interface: %s version: %d\n", name, interface,
	//        version);
}

static void registry_handle_global_remove(void *data, struct wl_registry *wl_registry,
					  uint32_t name)
{
	//
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

static void buffer_handle_release(void *data, struct wl_buffer *wl_buffer)
{
	struct wl_state *wl_state = (struct wl_state *)data;
	struct frame *frame = &wl_state->frame;

	wl_buffer_destroy(wl_buffer);
	munmap(frame->data, wl_state->frame_size);
}

static const struct wl_buffer_listener buffer_listener = {
	.release = buffer_handle_release,
};
