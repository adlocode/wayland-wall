/*
 * Copyright © 2016 Quentin "Sardem FF7" Glidic
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "helpers.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#include <sys/mman.h>

#include <wayland-cursor.h>
#ifdef ENABLE_IMAGES
#include <gdk-pixbuf/gdk-pixbuf.h>
#if ( ( GDK_PIXBUF_MAJOR < 2 ) || ( ( GDK_PIXBUF_MAJOR == 2 ) && ( GDK_PIXBUF_MINOR < 32 ) ) )
static inline const guchar *gdk_pixbuf_read_pixels(GdkPixbuf *pixbuf) { return gdk_pixbuf_get_pixels(pixbuf); }
#endif /* gdk-pixbux < 2.32 */
#endif /* ENABLE_IMAGES */
#include "unstable/background/background-unstable-v1-client-protocol.h"

/* Supported interface versions */
#define WL_COMPOSITOR_INTERFACE_VERSION 3
#define WW_BACKGROUND_INTERFACE_VERSION 1
#define WL_SHM_INTERFACE_VERSION 1
#define WL_SEAT_INTERFACE_VERSION 5
#define WL_OUTPUT_INTERFACE_VERSION 2

typedef enum {
    WW_BACKGROUND_GLOBAL_COMPOSITOR,
    WW_BACKGROUND_GLOBAL_BACKGROUND,
    WW_BACKGROUND_GLOBAL_SHM,
    _WW_BACKGROUND_GLOBAL_SIZE,
} WwBackgroundGlobalName;

typedef struct {
    bool released;
    struct wl_buffer *buffer;
    void *data;
    size_t size;
} WwBackgroundBuffer;

typedef struct {
    char runtime_dir[PATH_MAX];
    struct wl_display *display;
    struct wl_registry *registry;
    uint32_t global_names[_WW_BACKGROUND_GLOBAL_SIZE];
    struct wl_compositor *compositor;
    struct zww_background_v1 *background;
    struct wl_shm *shm;
    struct {
        char *theme_name;
        char **name;
        struct wl_cursor_theme *theme;
        struct wl_cursor *cursor;
        struct wl_cursor_image *image;
        struct wl_surface *surface;
        struct wl_callback *frame_cb;
    } cursor;
    struct wl_list seats;
    struct wl_list outputs;
#ifdef ENABLE_IMAGES
    char *image;
    bool image_scalable;
    GdkPixbuf *pixbuf;
#endif /* ENABLE_IMAGES */
    int32_t width;
    int32_t height;
    WwColour colour;
    enum zww_background_v1_fit_method fit_method;
    WwBackgroundBuffer *buffer;
} WwBackgroundContext;

typedef struct _WwBackgroundOutput WwBackgroundOutput;
typedef struct  {
    WwBackgroundOutput *output;
    int32_t width;
    int32_t height;
    struct wl_surface *surface;
} WwBackgroundSurface;

typedef struct {
    WwBackgroundContext *context;
    struct wl_list link;
    uint32_t global_name;
    struct wl_seat *seat;
    struct wl_pointer *pointer;
} WwBackgroundSeat;

struct _WwBackgroundOutput {
    WwBackgroundContext *context;
    struct wl_list link;
    uint32_t global_name;
    struct wl_output *output;
    int32_t width;
    int32_t height;
    int32_t scale;
    WwBackgroundSurface *surface;
};

static void
_ww_background_buffer_release(void *data, struct wl_buffer *buf)
{
    WwBackgroundBuffer *self = data;

    if ( self->released )
    {
        wl_buffer_destroy(self->buffer);
        free(self);
    }
    else
        self->released = true;
}

static const struct wl_buffer_listener _ww_background_buffer_listener = {
    _ww_background_buffer_release
};

static void
_ww_background_surface_update(WwBackgroundSurface *self, WwBackgroundBuffer *buffer)
{
    struct wl_region *region;

    wl_surface_damage(self->surface, 0, 0, self->width, self->height);
    wl_surface_attach(self->surface, buffer->buffer, 0, 0);
    if ( wl_surface_get_version(self->surface) >= WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION )
        wl_surface_set_buffer_scale(self->surface, self->output->scale);
    region = wl_compositor_create_region(self->output->context->compositor);
    wl_region_add(region, 0, 0, self->width, self->height);
    wl_surface_set_opaque_region(self->surface, region);
    wl_surface_commit(self->surface);
    wl_region_destroy(region);

    zww_background_v1_set_background(self->output->context->background, self->surface, self->output->output, self->output->context->fit_method);
}

#if BYTE_ORDER == BIG_ENDIAN
#define RED_BYTE 1
#define GREEN_BYTE 2
#define BLUE_BYTE 3
#define ALPHA_BYTE 0
#else
#define RED_BYTE 2
#define GREEN_BYTE 1
#define BLUE_BYTE 0
#define ALPHA_BYTE 3
#endif

static bool
_ww_background_create_buffer(WwBackgroundContext *self, int32_t width, int32_t height)
{
    struct wl_shm_pool *pool;
    struct wl_buffer *buffer;
    int fd;
    uint8_t *data;
    int32_t stride;
    size_t size;

#ifdef ENABLE_IMAGES
    const uint8_t *pdata = NULL;
    int32_t cstride, bytes;
    if ( self->pixbuf != NULL )
    {
        cstride = gdk_pixbuf_get_rowstride(self->pixbuf);
        bytes = gdk_pixbuf_get_has_alpha(self->pixbuf) ? 4 : 3;
        pdata = gdk_pixbuf_read_pixels(self->pixbuf);
        self->fit_method = ZWW_BACKGROUND_V1_FIT_METHOD_DEFAULT;
    }
    else
#endif /* ENABLE_IMAGES */
        self->fit_method = ZWW_BACKGROUND_V1_FIT_METHOD_CROP;

    stride = 4 * width;
    size = stride * height;

    char filename[PATH_MAX];
    snprintf(filename, PATH_MAX, "%s/%s", self->runtime_dir, "wayland-surface");
    fd = open(filename, O_CREAT | O_RDWR | O_CLOEXEC, 0);
    unlink(filename);
    if ( fd < 0 )
    {
        ww_warning("creating a buffer file for %zu B failed: %s\n", size, strerror(errno));
        return false;
    }
    if ( ftruncate(fd, size) < 0 )
    {
        close(fd);
        return false;
    }

    data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if ( data == MAP_FAILED )
    {
        ww_warning("mmap failed: %s\n", strerror(errno));
        close(fd);
        return false;
    }

    for ( int32_t y = 0 ; y < height ; ++y )
    {
        uint8_t *line = data + y * stride;
        for ( int32_t x = 0 ; x < width ; ++x )
        {
            uint8_t *pixel = line + x * 4;
#ifdef ENABLE_IMAGES
            if ( pdata != NULL )
            {
                const uint8_t *ppixel = pdata + y * cstride + x * bytes;

                pixel[ALPHA_BYTE] = 0xff;
                pixel[RED_BYTE]   = ppixel[0];
                pixel[GREEN_BYTE] = ppixel[1];
                pixel[BLUE_BYTE]  = ppixel[2];
            }
            else
#endif /* ENABLE_IMAGES */
            {
                pixel[ALPHA_BYTE] = 0xff;
                pixel[RED_BYTE]   = self->colour.r * 0xff;
                pixel[GREEN_BYTE] = self->colour.g * 0xff;
                pixel[BLUE_BYTE]  = self->colour.b * 0xff;
            }
        }
    }

    munmap(data, size);

    pool = wl_shm_create_pool(self->shm, fd, size);
    buffer = wl_shm_pool_create_buffer(pool, 0, width, height, width * 4, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    if ( self->buffer != NULL )
        _ww_background_buffer_release(self->buffer, self->buffer->buffer);

    self->buffer = ww_new0(WwBackgroundBuffer, 1);
    self->buffer->buffer = buffer;
    self->buffer->data = data;
    self->buffer->size = size;

    wl_buffer_add_listener(buffer, &_ww_background_buffer_listener, self->buffer);

    WwBackgroundOutput *output;
    wl_list_for_each(output, &self->outputs, link)
    {
        if ( output->surface != NULL )
            _ww_background_surface_update(output->surface, self->buffer);
    }

    return true;
}

static void
_ww_background_check_buffer(WwBackgroundContext *self, WwBackgroundSurface *surface)
{
    int32_t width, height;

    width = surface->width;
    height = surface->height;

#ifdef ENABLE_IMAGES
    if ( self->pixbuf != NULL )
    {
        int pw, ph;
        pw = gdk_pixbuf_get_width(self->pixbuf);
        ph = gdk_pixbuf_get_height(self->pixbuf);
        if ( self->image_scalable && ( ( pw < width ) || ( ph < height ) ) )
        {
            GdkPixbuf *pixbuf;
            GError *error = NULL;

            /*
             * If the image is scalable, we already loaded it at the biggest size we need
             * so we use MAX() to get the biggest size again
             */
            pixbuf = gdk_pixbuf_new_from_file_at_size(self->image, MAX(pw, width), MAX(ph, height), &error);
            if ( pixbuf != NULL )
            {
                GdkPixbuf *tmp = self->pixbuf;
                self->pixbuf = pixbuf;
                width = gdk_pixbuf_get_width(self->pixbuf);
                height = gdk_pixbuf_get_height(self->pixbuf);
                if ( _ww_background_create_buffer(self, width, height) )
                {
                    g_object_unref(tmp);
                    return;
                }
                else
                {
                    ww_warning("Couldn’t create a new buffer from the pixbuf");
                    g_object_unref(pixbuf);
                    self->pixbuf = tmp;
                }
            }
            else
            {
                ww_warning("Couldn’t reload the pixbuf: %s", error->message);
                g_error_free(error);
            }
        }
    }
    else
#endif /* ENABLE_IMAGES */
    {
        if ( ( self->width < width ) || ( self->height < height ) )
        {
            int32_t new_width = MAX(self->width, width), new_height = MAX(self->height, height);

            if ( _ww_background_create_buffer(self, width, height) )
            {
                self->width = new_width;
                self->height = new_height;
                return;
            }
        }
    }
    _ww_background_surface_update(surface, self->buffer);
}

static WwBackgroundSurface *
_ww_background_surface_new(WwBackgroundOutput *output)
{
    WwBackgroundSurface *self;

    self = ww_new0(WwBackgroundSurface, 1);
    self->output = output;
    self->width = self->output->width * self->output->scale;
    self->height = self->output->height * self->output->scale;

    self->surface = wl_compositor_create_surface(self->output->context->compositor);
    wl_surface_set_user_data(self->surface, self);

    return self;
}

static void
_ww_background_surface_free(WwBackgroundSurface *self)
{
    wl_surface_destroy(self->surface);

    free(self);
}

static void
_ww_background_cursor_set_image(WwBackgroundContext *self, int i)
{
    struct wl_buffer *buffer;
    struct wl_cursor_image *image;
    image = self->cursor.cursor->images[i];

    self->cursor.image = image;
    buffer = wl_cursor_image_get_buffer(self->cursor.image);
    wl_surface_attach(self->cursor.surface, buffer, 0, 0);
    wl_surface_damage(self->cursor.surface, 0, 0, self->cursor.image->width, self->cursor.image->height);
    wl_surface_commit(self->cursor.surface);
}

static void _ww_background_cursor_frame_callback(void *data, struct wl_callback *callback, uint32_t time);

static const struct wl_callback_listener _ww_background_cursor_frame_wl_callback_listener = {
    .done = _ww_background_cursor_frame_callback,
};

static void
_ww_background_cursor_frame_callback(void *data, struct wl_callback *callback, uint32_t time)
{
    WwBackgroundContext *self = data;
    int i;

    if ( self->cursor.frame_cb != NULL )
        wl_callback_destroy(self->cursor.frame_cb);
    self->cursor.frame_cb = wl_surface_frame(self->cursor.surface);
    wl_callback_add_listener(self->cursor.frame_cb, &_ww_background_cursor_frame_wl_callback_listener, self);

    i = wl_cursor_frame(self->cursor.cursor, time);
    _ww_background_cursor_set_image(self, i);
}

static void
_ww_background_pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y)
{
    WwBackgroundSeat *self = data;
    WwBackgroundContext *context = self->context;

    if ( context->cursor.surface == NULL )
        return;

    if ( context->cursor.cursor->image_count < 2 )
        _ww_background_cursor_set_image(context, 0);
    else
        _ww_background_cursor_frame_callback(context, context->cursor.frame_cb, 0);

    wl_pointer_set_cursor(self->pointer, serial, context->cursor.surface, context->cursor.image->hotspot_x, context->cursor.image->hotspot_y);
}

static void
_ww_background_pointer_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface)
{
    WwBackgroundSeat *self = data;
    WwBackgroundContext *context = self->context;

    if ( context->cursor.frame_cb != NULL )
        wl_callback_destroy(context->cursor.frame_cb);
}

static void
_ww_background_pointer_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
}

static void
_ww_background_pointer_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, enum wl_pointer_button_state state)
{
}

static void
_ww_background_pointer_axis(void *data, struct wl_pointer *pointer, uint32_t time, enum wl_pointer_axis axis, wl_fixed_t value)
{
}

static void
_ww_background_pointer_frame(void *data, struct wl_pointer *pointer)
{
}

static void
_ww_background_pointer_axis_source(void *data, struct wl_pointer *pointer, enum wl_pointer_axis_source axis_source)
{
}

static void
_ww_background_pointer_axis_stop(void *data, struct wl_pointer *pointer, uint32_t time, enum wl_pointer_axis axis)
{
}

static void
_ww_background_pointer_axis_discrete(void *data, struct wl_pointer *pointer, enum wl_pointer_axis axis, int32_t discrete)
{
}

static const struct wl_pointer_listener _ww_background_pointer_listener = {
    .enter = _ww_background_pointer_enter,
    .leave = _ww_background_pointer_leave,
    .motion = _ww_background_pointer_motion,
    .button = _ww_background_pointer_button,
    .axis = _ww_background_pointer_axis,
    .frame = _ww_background_pointer_frame,
    .axis_source = _ww_background_pointer_axis_source,
    .axis_stop = _ww_background_pointer_axis_stop,
    .axis_discrete = _ww_background_pointer_axis_discrete,
};

static void
_ww_background_pointer_release(WwBackgroundSeat *self)
{
    if ( self->pointer == NULL )
        return;

    if ( wl_pointer_get_version(self->pointer) >= WL_POINTER_RELEASE_SINCE_VERSION )
        wl_pointer_release(self->pointer);
    else
        wl_pointer_destroy(self->pointer);

    self->pointer = NULL;
}

static void
_ww_background_seat_release(WwBackgroundSeat *self)
{
    _ww_background_pointer_release(self);

    if ( wl_seat_get_version(self->seat) >= WL_SEAT_RELEASE_SINCE_VERSION )
        wl_seat_release(self->seat);
    else
        wl_seat_destroy(self->seat);

    wl_list_remove(&self->link);

    free(self);
}

static void
_ww_background_seat_capabilities(void *data, struct wl_seat *seat, uint32_t capabilities)
{
    WwBackgroundSeat *self = data;
    if ( ( capabilities & WL_SEAT_CAPABILITY_POINTER ) && ( self->pointer == NULL ) )
    {
        self->pointer = wl_seat_get_pointer(self->seat);
        wl_pointer_add_listener(self->pointer, &_ww_background_pointer_listener, self);
    }
    else if ( ( ! ( capabilities & WL_SEAT_CAPABILITY_POINTER ) ) && ( self->pointer != NULL ) )
        _ww_background_pointer_release(self);
}

static void
_ww_background_seat_name(void *data, struct wl_seat *seat, const char *name)
{
}

static const struct wl_seat_listener _ww_background_seat_listener = {
    .capabilities = _ww_background_seat_capabilities,
    .name = _ww_background_seat_name,
};

static void
_ww_background_output_release(WwBackgroundOutput *self)
{
    _ww_background_surface_free(self->surface);

    if ( wl_output_get_version(self->output) >= WL_OUTPUT_RELEASE_SINCE_VERSION )
        wl_output_release(self->output);
    else
        wl_output_destroy(self->output);

    wl_list_remove(&self->link);

    free(self);
}

static void
_ww_background_output_done(void *data, struct wl_output *output)
{
    WwBackgroundOutput *self = data;

    if ( self->surface == NULL )
        self->surface = _ww_background_surface_new(self);
    _ww_background_check_buffer(self->context, self->surface);
}

static void
_ww_background_output_geometry(void *data, struct wl_output *output, int32_t x, int32_t y, int32_t width, int32_t height, int32_t subpixel, const char *make, const char *model, int32_t transform)
{
}

static void
_ww_background_output_mode(void *data, struct wl_output *output, enum wl_output_mode flags, int32_t width, int32_t height, int32_t refresh)
{
    WwBackgroundOutput *self = data;

    if ( ! ( flags & WL_OUTPUT_MODE_CURRENT ) )
        return;

    self->width = width;
    self->height = height;

    if ( wl_output_get_version(self->output) < WL_OUTPUT_DONE_SINCE_VERSION )
        _ww_background_output_done(self, self->output);
}

static void
_ww_background_output_scale(void *data, struct wl_output *output, int32_t scale)
{
    WwBackgroundOutput *self = data;

    self->scale = scale;
}

static const struct wl_output_listener _ww_background_output_listener = {
    .geometry = _ww_background_output_geometry,
    .mode = _ww_background_output_mode,
    .scale = _ww_background_output_scale,
    .done = _ww_background_output_done,
};

static const char * const _ww_background_cursor_names[] = {
    "left_ptr",
    "default",
    "top_left_arrow",
    "left-arrow",
    NULL
};

static void
_ww_background_registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    WwBackgroundContext *self = data;

    if ( strcmp0(interface, "wl_compositor") == 0 )
    {
        self->global_names[WW_BACKGROUND_GLOBAL_COMPOSITOR] = name;
        self->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, MIN(version, WL_COMPOSITOR_INTERFACE_VERSION));
    }
    else if ( strcmp0(interface, "zww_background_v1") == 0 )
    {
        self->global_names[WW_BACKGROUND_GLOBAL_BACKGROUND] = name;
        self->background = wl_registry_bind(registry, name, &zww_background_v1_interface, WW_BACKGROUND_INTERFACE_VERSION);
    }
    else if ( strcmp0(interface, "wl_shm") == 0 )
    {
        self->global_names[WW_BACKGROUND_GLOBAL_SHM] = name;
        self->shm = wl_registry_bind(registry, name, &wl_shm_interface, MIN(version, WL_SHM_INTERFACE_VERSION));
    }
    else if ( strcmp0(interface, "wl_seat") == 0 )
    {
        WwBackgroundSeat *seat = ww_new0(WwBackgroundSeat, 1);
        seat->context = self;
        seat->global_name = name;
        seat->seat = wl_registry_bind(registry, name, &wl_seat_interface, MIN(version, WL_SEAT_INTERFACE_VERSION));

        wl_list_insert(&self->seats, &seat->link);

        wl_seat_add_listener(seat->seat, &_ww_background_seat_listener, seat);
    }
    else if ( strcmp0(interface, "wl_output") == 0 )
    {
        WwBackgroundOutput *output = ww_new0(WwBackgroundOutput, 1);
        output->context = self;
        output->global_name = name;
        output->output = wl_registry_bind(registry, name, &wl_output_interface, MIN(version, WL_OUTPUT_INTERFACE_VERSION));
        output->scale = 1;

        wl_list_insert(&self->outputs, &output->link);

        wl_output_add_listener(output->output, &_ww_background_output_listener, output);
    }

    if ( ( self->cursor.theme == NULL ) && ( self->compositor != NULL ) && ( self->shm != NULL ) )
    {
        self->cursor.theme = wl_cursor_theme_load(self->cursor.theme_name, 32, self->shm);
        if ( self->cursor.theme != NULL )
        {
            const char * const *cname = (const char * const *) self->cursor.name;
            for ( cname = ( cname != NULL ) ? cname : _ww_background_cursor_names ; ( self->cursor.cursor == NULL ) && ( *cname != NULL ) ; ++cname )
                self->cursor.cursor = wl_cursor_theme_get_cursor(self->cursor.theme, *cname);
            if ( self->cursor.cursor == NULL )
            {
                wl_cursor_theme_destroy(self->cursor.theme);
                self->cursor.theme = NULL;
            }
            else
                self->cursor.surface = wl_compositor_create_surface(self->compositor);
        }
    }
}

static void
_ww_background_registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
    WwBackgroundContext *self = data;

    WwBackgroundGlobalName i;
    for ( i = 0 ; i < _WW_BACKGROUND_GLOBAL_SIZE ; ++i )
    {
        if ( self->global_names[i] != name )
            continue;
        self->global_names[i] = 0;

        switch ( i )
        {
        case WW_BACKGROUND_GLOBAL_COMPOSITOR:
            wl_compositor_destroy(self->compositor);
            self->compositor = NULL;
        break;
        case WW_BACKGROUND_GLOBAL_BACKGROUND:
            zww_background_v1_destroy(self->background);
            self->background = NULL;
        break;
        case WW_BACKGROUND_GLOBAL_SHM:
            wl_shm_destroy(self->shm);
            self->shm = NULL;
        break;
        case _WW_BACKGROUND_GLOBAL_SIZE:
            assert_not_reached();
        }
        return;
    }
    if ( ( self->cursor.theme != NULL ) && ( ( self->compositor == NULL ) || ( self->shm == NULL ) ) )
    {
        if ( self->cursor.frame_cb != NULL )
            wl_callback_destroy(self->cursor.frame_cb);
        self->cursor.frame_cb = NULL;

        wl_surface_destroy(self->cursor.surface);
        wl_cursor_theme_destroy(self->cursor.theme);
        self->cursor.surface = NULL;
        self->cursor.image = NULL;
        self->cursor.cursor = NULL;
        self->cursor.theme = NULL;
    }

    WwBackgroundSeat *seat, *tmp_seat;
    wl_list_for_each_safe(seat, tmp_seat, &self->seats, link)
    {
        if ( seat->global_name != name )
            continue;

        _ww_background_seat_release(seat);
        return;
    }

    WwBackgroundOutput *output, *tmp_output;
    wl_list_for_each_safe(output, tmp_output, &self->outputs, link)
    {
        if ( output->global_name != name )
            continue;

        _ww_background_output_release(output);
        return;
    }
}

static const struct wl_registry_listener _ww_background_registry_listener = {
    .global = _ww_background_registry_handle_global,
    .global_remove = _ww_background_registry_handle_global_remove,
};


static void
_ww_background_disconnect(WwBackgroundContext *self)
{
    WwBackgroundSeat *seat, *tmp;
    wl_list_for_each_safe(seat, tmp, &self->seats, link)
        _ww_background_seat_release(seat);

    WwBackgroundGlobalName i;
    for ( i = 0 ; i < _WW_BACKGROUND_GLOBAL_SIZE ; ++i )
    {
        if ( self->global_names[i] != 0 )
            _ww_background_registry_handle_global_remove(self, self->registry, self->global_names[i]);
    }

    wl_registry_destroy(self->registry);
    self->registry = NULL;

    wl_display_disconnect(self->display);
    self->display = NULL;
}

int
main(int argc, char *argv[])
{
    static WwBackgroundContext self_;
    WwBackgroundContext *self = &self_;

    self->width = 1920;
    self->height = 1080;

    setlocale(LC_ALL, "");

    const char *runtime_dir;
    runtime_dir = getenv("XDG_RUNTIME_DIR");
    if ( runtime_dir == NULL )
        return 1;
    snprintf(self->runtime_dir, PATH_MAX, "%s/" PACKAGE_NAME, runtime_dir);

    if ( mkdir(self->runtime_dir, 0755) < 0 )
    {
        if ( errno == EEXIST )
        {
            struct stat buf;
            if ( stat(self->runtime_dir, &buf) < 0 )
                return 1;
            if ( ! S_ISDIR(buf.st_mode) )
                return 1;
        }
        else
            return 1;
    }

    self->display = wl_display_connect(NULL);
    if ( self->display == NULL )
        return 2;

    wl_list_init(&self->seats);
    wl_list_init(&self->outputs);

    if ( argc > 1 )
    {
        if ( _ww_parse_colour(argv[1], &self->colour) )
        {
            if ( argc > 3 )
            {
                self->width = strtol(argv[2], NULL, 0);
                self->height = strtol(argv[3], NULL, 0);
            }
        }
#ifdef ENABLE_IMAGES
        else
        {
            GError *error = NULL;
            GdkPixbufFormat *format;
            format = gdk_pixbuf_get_file_info(argv[1], NULL, NULL);
            if ( format != NULL )
            {
                self->image = argv[1];
                self->image_scalable = gdk_pixbuf_format_is_scalable(format);
                self->pixbuf = gdk_pixbuf_new_from_file(self->image, &error);
                if ( self->pixbuf == NULL )
                {
                    ww_warning("Couldn’t load image: %s", error->message);
                    g_error_free(error);
                }
                else
                {
                    self->width = gdk_pixbuf_get_width(self->pixbuf);
                    self->height = gdk_pixbuf_get_height(self->pixbuf);
                }
            }
        }
#endif /* ENABLE_IMAGES */
    }

    self->registry = wl_display_get_registry(self->display);
    wl_registry_add_listener(self->registry, &_ww_background_registry_listener, self);
    wl_display_roundtrip(self->display);

    if ( self->shm == NULL )
    {
        _ww_background_disconnect(self);
        ww_warning("No wl_shm interface provided by the compositor");
        return 3;
    }
    if ( self->background == NULL )
    {
        _ww_background_disconnect(self);
        ww_warning("No ww_background interface provided by the compositor");
        return 3;
    }

    if ( ! _ww_background_create_buffer(self, self->width, self->height) )
        return 4;

    int ret;
    do
        ret = wl_display_dispatch(self->display);
    while ( ret > 0 );
    if ( ret < 0 )
        ww_warning("Couldn’t dispatch events: %s", strerror(errno));

    return 0;
}
