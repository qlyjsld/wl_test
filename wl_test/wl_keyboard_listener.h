#pragma once
#include "wl_util.h"
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

static void keyboard_handle_keymap(void *data, struct wl_keyboard *wl_keyboard,
				   uint32_t format, int32_t fd, uint32_t size)
{
	struct wl_state *wl_state = (struct wl_state *)data;
	assert(format == XKB_KEYMAP_FORMAT_TEXT_V1);

	char *keymap = (char *)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	assert(keymap != MAP_FAILED);

	struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(
		wl_state->xkb_context, keymap, XKB_KEYMAP_FORMAT_TEXT_V1,
		XKB_KEYMAP_COMPILE_NO_FLAGS);

	munmap(keymap, size);
	close(fd);

	struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
	xkb_keymap_unref(wl_state->xkb_keymap);
	xkb_state_unref(wl_state->xkb_state);
	wl_state->xkb_keymap = xkb_keymap;
	wl_state->xkb_state = xkb_state;
}

static void keyboard_handle_enter(void *data, struct wl_keyboard *wl_keyboard,
				  uint32_t serial, struct wl_surface *surface,
				  struct wl_array *keys)
{
	// char buf[128];
	// struct wl_state *wl_state = (struct wl_state *)data;
	// fprintf(stderr, "keyboard enter; keys pressed:\n");

	// uint32_t *key;
	// wl_array_for_each_uint32_t(key, keys) {
	// 	xkb_keysym_t sym =
	// 		xkb_state_key_get_one_sym(wl_state->xkb_state, *key + 8);
	// 	xkb_keysym_get_name(sym, buf, 128);
	// 	fprintf(stderr, "sym: %s ", buf);
	// 	xkb_keysym_to_utf8(sym, buf, 128);
	// 	fprintf(stderr, "utf8: %s\n", buf);
	// }
}

static void keyboard_handle_leave(void *data, struct wl_keyboard *wl_keyboard,
				  uint32_t serial, struct wl_surface *surface)
{
	// fprintf(stderr, "keyboard leave\n");
}

static void keyboard_handle_key(void *data, struct wl_keyboard *wl_keyboard,
				uint32_t serial, uint32_t time, uint32_t key,
				uint32_t state)
{
	char buf[128];
	struct wl_state *wl_state = (struct wl_state *)data;
	wl_state->key_frame = time;
	xkb_keysym_t sym = xkb_state_key_get_one_sym(wl_state->xkb_state, key + 8);
	wl_state->sym = sym;

	const char *action = state == WL_KEYBOARD_KEY_STATE_PRESSED ? "pressed" :
								      "released";

	fprintf(stderr, "key %s:\n", action);
	xkb_keysym_get_name(sym, buf, 128);
	fprintf(stderr, "sym: %s ", buf);
	xkb_keysym_to_utf8(sym, buf, 128);
	fprintf(stderr, "utf8: %s\n", buf);
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard *wl_keyboard,
				      uint32_t serial, uint32_t mods_depressed,
				      uint32_t mods_latched, uint32_t mods_locked,
				      uint32_t group)
{
	struct wl_state *wl_state = (struct wl_state *)data;
	xkb_state_update_mask(wl_state->xkb_state, mods_depressed, mods_latched,
			      mods_locked, 0, 0, group);
}

static void keyboard_handle_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
					int32_t rate, int32_t delay)
{
	struct wl_state *wl_state = (struct wl_state *)data;
	// fprintf(stderr, "rate: %d, delay: %d\n", rate, delay);
	wl_state->rate = rate;
	wl_state->delay = delay;
}

static const struct wl_keyboard_listener keyboard_listener = {
	.keymap = keyboard_handle_keymap,
	.enter = keyboard_handle_enter,
	.leave = keyboard_handle_leave,
	.key = keyboard_handle_key,
	.modifiers = keyboard_handle_modifiers,
	.repeat_info = keyboard_handle_repeat_info,
};