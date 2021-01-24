
#include "debug.h"

#include <string.h>

/*
 * Arguments structure to provide Input and output operations
 * 
 */
struct debug_args {
    void (*putc)(char, void *);  // The putc() function to invoke
    void *argp;                  // Associated data struct
};

typedef struct debug_args debug_args_t;  // Abbreviated ref to s_mini_args

/*
 * Internal: Write string msg until null byte, to the I/O
 *           routine described by s_mini_args.
 */
static void
debug_write(debug_args_t *args, const char *msg) {
    char ch;

    while ((ch = *msg++) != 0)
        args->putc(ch, args->argp);
}

/*
 * Internal: Pad % field to width, give text buffer.
 */
static void
debug_pad(debug_args_t *args, char pad, int width, const char *text) {
    int slen;

    if (width > 0) {
        slen = strlen(text);

        for (width -= slen; width > 0; --width)
            args->putc(pad, args->argp);
    }
}

/*
 * Internal: mini_printf() engine.
 */
static void
internal_vprintf(debug_args_t *args, const char *format, va_list arg) {
    char ch, pad, sgn;   /* Current char, pad char and sign char */
    int vint, width;     /* Integer value to print and field width */
    unsigned uint;       /* Unsigned value to print */
    const char *sptr;    /* String to print */
    char buf[32], *bptr; /* Formatting buffer for int/uint */

    while ((ch = *format++) != 0) {
        if (ch != '%') {
            /* Non formatting field: copy as is */
            args->putc(ch, args->argp);
            continue;
        }

        /*
		 * Process a format item:
		 */
        pad = ' ';      /* Default pad char is space */
        sgn = 0;        /* Assume no format sign char */
        ch = *format++; /* Grab next format char */

        if (ch == '+' || ch == '-') {
            sgn = ch;       /* Make note of format sign */
            ch = *format++; /* Next format char */
        }

        if (ch == '0') {
            pad = ch; /* Pad with zeros */
            ch = *format++;
        }

        /*
		 * Extract width when present:
		 */
        for (width = 0; ch && ch >= '0' && ch <= '9'; ch = *format++) {
            width = width * 10 + (ch & 0x0F);
        }

        if (!ch) {
            break; /* Exit loop if we hit end of format string (in error) */
        }

        /*
		 * Format according to type: d, x, or s
		 */
        switch (ch) {
            case 'c':
                vint = va_arg(arg, int);
                args->putc((char)vint, args->argp);
                break;
            case 'd': /* Decimal format */
                vint = va_arg(arg, int);
                if (vint < 0) {
                    args->putc('-', args->argp);
                    vint = -vint;
                } else if (sgn == '+')
                    args->putc(sgn, args->argp);
                bptr = buf + sizeof buf;
                *--bptr = 0;
                do {
                    *--bptr = vint % 10 + '0';
                    vint /= 10;
                } while (vint != 0);
                debug_pad(args, pad, width, bptr);
                debug_write(args, bptr);
                break;

            case 'x': /* Hexadecimal format */
                uint = va_arg(arg, unsigned);
                bptr = buf + sizeof buf;
                *--bptr = 0;
                do {
                    ch = uint & 0x0F;
                    *--bptr = ch + (ch <= 9 ? '0' : 'A' - 10);
                    uint >>= 4;
                } while (uint != 0);
                debug_pad(args, pad, width, bptr);
                debug_write(args, bptr);
                break;

            case 's': /* String format */
                sptr = va_arg(arg, const char *);
                if (sgn != '-')
                    debug_pad(args, pad, width, sptr);
                debug_write(args, sptr);
                if (sgn == '-')
                    debug_pad(args, pad, width, sptr);
                break;

            case '%': /* "%%" outputs as "%" */
                args->putc(ch, args->argp);
                break;

            default: /* Unsupported stuff here */
                args->putc('%', args->argp);
                args->putc('?', args->argp);
                args->putc(ch, args->argp);
        }
    }
}

/*
 * s_internal trackes the count of bytes output:
 */
struct s_internal {
    void (*putc)(char);  /* User's putc() routine to be used */
    unsigned count;      /* Bytes output */
    unsigned cooked : 1; /* When true, '\n' also emits '\r' */
};

static void
debug_putc(char ch, void *argp) {
    struct s_internal *internp = (struct s_internal *)argp;

    internp->putc(ch); /* Perform I/O */
    ++internp->count;  /* Count bytes out */

    if (ch == '\n' && internp->cooked != 0) {
        /* In cooked mode, issue CR after LF */
        internp->putc('\r');
        ++internp->count; /* Count CR */
    }
}

/*
 * Internal: Perform cooked/uncooked printf()
 */
static int
debug_vprintf0(void (*putc)(char), int cooked, const char *format, va_list args) {
    debug_args_t dbg_args;
    struct s_internal intern;

    intern.putc = putc;       /* User's putc() routine to be used */
    intern.count = 0u;        /* Byte counter */
    intern.cooked = !!cooked; /* True if LF to add CR */

    dbg_args.putc = debug_putc;      /* Internal interlude routine */
    dbg_args.argp = (void *)&intern; /* Ptr to internal struct */

    internal_vprintf(&dbg_args, format, args);
    return intern.count; /* Return byte count */
}

/*
 * External: Perform cooked mode printf()
 */
int debug_vprintf(void (*putc)(char), const char *format, va_list args) {
    return debug_vprintf0(putc, 1, format, args);
}

/*********************************************************************
 * Sprintf
 *********************************************************************/

struct s_debug_sprintf {
    char *buf;       /* Ptr to output buffer */
    unsigned maxbuf; /* Max bytes for buffer */
    char *ptr;       /* Ptr to next byte */
};

static void
debug_sputc(char ch, void *argp) {
    struct s_debug_sprintf *ctl = (struct s_debug_sprintf *)argp;

    if ((unsigned)(ctl->ptr - ctl->buf) >= ctl->maxbuf)
        return;
    *ctl->ptr++ = ch;
}

/*
 * External: sprintf() to buffer (not cooked)
 */
int debug_snprintf(char *buf, unsigned maxbuf, const char *format, ...) {
    debug_args_t dbg_args;      /* printf struct */
    struct s_debug_sprintf ctl; /* sprintf control */
    va_list args;               /* format arguments */
    unsigned count;             /* Return count */

    dbg_args.putc = debug_sputc;  /* Internal routine */
    dbg_args.argp = (void *)&ctl; /* Using ctl to guide it */

    ctl.ptr = ctl.buf = buf; /* Destination for data */
    ctl.maxbuf = maxbuf;     /* Max size in bytes */

    va_start(args, format);
    internal_vprintf(&dbg_args, format, args);
    va_end(args);

    count = (unsigned)(ctl.ptr - ctl.buf); /* Calculate count */
    debug_sputc(0, &ctl);                  /* Null terminate output if possible */
    return count;                          /* Return formatted count */
}
