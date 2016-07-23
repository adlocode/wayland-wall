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

#ifndef __WW_HELPERS_H__
#define __WW_HELPERS_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif /* HAVE_LOCALE_H */
#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif /* HAVE_STDBOOL_H */
#ifdef HAVE_STDINT_H
#include <stdbool.h>
#endif /* HAVE_STDBOOL_H */
#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif /* HAVE_ASSERT_H */
#include <assert.h>

#include <wayland-client.h>

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(x, min, max) (MAX((min), MIN((max), (x))))
#endif

#define ww_log(type, format, ...) fprintf(stderr, "[%s:%d] %s() " type " " format "\n", __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#define ww_debug(format, ...) ww_log("DEBUG", format, ## __VA_ARGS__)
#define ww_warning(format, ...) ww_log("WARNING", format, ## __VA_ARGS__)
#define ww_error(format, ...) do { ww_log("ERROR", format, ## __VA_ARGS__); abort(); } while (0)

#define ww_new0(type, n) ((type *)calloc(sizeof(type), n))

#define strcmp0(a, b) (((a) == (b)) ? 0 : (((a) == NULL) ? -1 : (((b) == NULL) ? 1 : strcmp(a, b))))

#define assert_not_reached() do { assert(0 && "Should never be reached"); return; } while (0)



typedef struct {
    double r;
    double g;
    double b;
    double a;
} WwColour;

static inline bool
_ww_parse_colour(const char *spec, WwColour *colour)
{
    if ( spec[0] != '#' )
        return false;
    ++spec;

    char sr[3] = { '\0', '\0', '\0' };
    char sg[3] = { '\0', '\0', '\0' };
    char sb[3] = { '\0', '\0', '\0' };
    char sa[3] = { 'f', 'f', '\0' };

    switch ( strlen(spec) )
    {
    case 8:
        sa[0] = spec[6];
        sa[1] = spec[7];
    case 6:
        sr[0] = spec[0];
        sr[1] = spec[1];
        sg[0] = spec[2];
        sg[1] = spec[3];
        sb[0] = spec[4];
        sb[1] = spec[5];
    break;
    case 4:
        sa[0] = spec[3];
        sa[1] = spec[3];
    case 3:
        sr[0] = spec[0];
        sr[1] = spec[0];
        sg[0] = spec[1];
        sg[1] = spec[1];
        sb[0] = spec[2];
        sb[1] = spec[2];
    break;
    default:
        return false;
    }

    uint32_t r, g, b, a;

    r = strtoul(sr, NULL, 16);
    g = strtoul(sg, NULL, 16);
    b = strtoul(sb, NULL, 16);
    a = strtoul(sa, NULL, 16);

    r = MIN(r, 255);
    g = MIN(g, 255);
    b = MIN(b, 255);
    a = MIN(a, 255);

    colour->r = (double) r / 0xff;
    colour->g = (double) g / 0xff;
    colour->b = (double) b / 0xff;
    colour->a = (double) a / 0xff;
    return true;
}

#endif /* __WW_HELPERS_H__ */
