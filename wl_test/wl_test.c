#include <stdio.h>
#include <wayland-client-protocol.h>
#include <xkbcommon/xkbcommon.h>
#include "xdg-shell-client-protocol.h"
#include "shm.h"
#include "wl_listener.h"

const int width = 800, height = 600;
const int stride = width * 4;

static const struct wl_callback_listener callback_listener;

static void create_frame(int width, int height, int stride, struct frame *frame,
			 void *data)
{
	struct wl_state *wl_state = data;
	size_t shm_pool_size = stride * height;

	wl_state->frame_size = shm_pool_size;
	int fd = allocate_shm_file(shm_pool_size);
	assert(fd != -1);

	frame->data =
		mmap(NULL, shm_pool_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	assert(frame->data != MAP_FAILED);
	wl_state->shm_pool = wl_shm_create_pool(wl_state->shm, fd, shm_pool_size);
	frame->buffer = wl_shm_pool_create_buffer(wl_state->shm_pool, 0, width, height,
						  stride, WL_SHM_FORMAT_XRGB8888);

	wl_buffer_add_listener(frame->buffer, &buffer_listener, NULL);

	wl_shm_pool_destroy(wl_state->shm_pool);
	close(fd);
}

static struct wl_buffer *draw_frame(void *data)
{
	struct wl_state *wl_state = data;
	struct frame *frame = &wl_state->frame;

	/* Draw checkerboxed background */
	int offset = (int)wl_state->offset % 8;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			if ((x + offset + (y + offset) / 8 * 8) % 16 < 8)
				frame->data[y * width + x] = 0xFF666666;
			else
				frame->data[y * width + x] = 0xFFEEEEEE;
		}
	}

	return frame->buffer;
}

static void xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface,
					 uint32_t serial)
{
	struct wl_state *wl_state = data;
	// printf("xdg_surface configure event with serial %d\n", serial);
	xdg_surface_ack_configure(xdg_surface, serial);
	struct wl_buffer *buffer = draw_frame(wl_state);
	wl_surface_attach(wl_state->surface, buffer, 0, 0);
	wl_surface_commit(wl_state->surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_handle_configure,
};

static void callback_handle_done(void *data, struct wl_callback *wl_callback,
				 uint32_t callback_data)
{
	char buf[128];
	struct wl_state *wl_state = (struct wl_state *)data;
	wl_callback_destroy(wl_callback);
	wl_callback = wl_surface_frame(wl_state->surface);
	wl_callback_add_listener(wl_callback, &callback_listener, wl_state);

	int elapsed = callback_data - wl_state->last_frame;
	wl_state->offset += elapsed / 1000.0 * 24.0;
	wl_state->last_frame = callback_data;

	// if (wl_state->pressed &&
	//     ((callback_data - wl_state->key_frame) > wl_state->delay)) {
	// 	fprintf(stderr, "key holding\n");
	// 	xkb_keysym_get_name(wl_state->sym, buf, 128);
	// 	fprintf(stderr, "sym: %s ", buf);
	// 	xkb_keysym_to_utf8(wl_state->sym, buf, 128);
	// 	fprintf(stderr, "utf8: %s\n", buf);
	// }

	struct wl_buffer *buffer = draw_frame(wl_state);
	wl_surface_attach(wl_state->surface, buffer, 0, 0);
	wl_surface_damage_buffer(wl_state->surface, 0, 0, INT32_MAX, INT32_MAX);
	wl_surface_commit(wl_state->surface);
	// printf("fps: %f\n", 1.0 / (float) elapsed * 1000);
}

static const struct wl_callback_listener callback_listener = {
	.done = callback_handle_done,
};

int main(int argv, char *argc[])
{
	struct wl_state wl_state = { 0 };
	wl_state.display = wl_display_connect(NULL);
	if (!wl_state.display) {
		fprintf(stderr, "failed to connect to wayland server\n");
		return 1;
	}
	printf("connection established\n");

	wl_state.registry = wl_display_get_registry(wl_state.display);
	wl_state.xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	wl_registry_add_listener(wl_state.registry, &registry_listener, &wl_state);
	wl_display_roundtrip(wl_state.display); // block until binding all globals

	wl_state.surface = wl_compositor_create_surface(wl_state.compositor);
	create_frame(width, height, stride, &wl_state.frame, &wl_state);
	wl_state.callback = wl_surface_frame(wl_state.surface);
	wl_callback_add_listener(wl_state.callback, &callback_listener, &wl_state);

	wl_state.xdg_surface =
		xdg_wm_base_get_xdg_surface(wl_state.xdg_wm_base, wl_state.surface);
	xdg_surface_add_listener(wl_state.xdg_surface, &xdg_surface_listener, &wl_state);
	wl_state.xdg_toplevel = xdg_surface_get_toplevel(wl_state.xdg_surface);
	xdg_toplevel_set_title(wl_state.xdg_toplevel, "");
	wl_surface_commit(wl_state.surface);

	while (wl_display_dispatch(wl_state.display)) {
		//
	}

	return 0;
}
