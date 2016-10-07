#ifndef _COMPILER_H_
#define _COMPIER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * \brief Emit the compiler pragma \a arg.
 *
 * \param[in] arg  The pragma directive as it would appear after \e \#pragma
 *             (i.e. not stringified).
 */
#define COMPILER_PRAGMA(arg)          _Pragma(#arg)

/**
 * \def COMPILER_PACK_SET(alignment)
 * \brief Set maximum alignment for subsequent struct and union definitions to \a alignment.
 */
#define COMPILER_PACK_SET(alignment)  COMPILER_PRAGMA(pack(alignment))

/**
 * \def COMPILER_PACK_RESET()
 * \brief Set default alignment for subsequent struct and union definitions.
 */
#define COMPILER_PACK_RESET()         COMPILER_PRAGMA(pack())

/**
 * \brief Set aligned boundary.
 */
#if defined __GNUC__
#   define COMPILER_ALIGNED(a) __attribute__((__aligned__(a)))
#elif defined __ICCARM__
#   define COMPILER_ALIGNED(a) COMPILER_PRAGMA(data_alignment = a)
#elif defined __CC_ARM
#   define COMPILER_ALIGNED(a) __attribute__((__aligned__(a)))
#endif

static inline int is_big_endian(void)
{
    union {
        uint32_t i;
        char c[4];
    } e = { 0x01000000 };

    return e.c[0];
}

#ifdef __cplusplus
}
#endif

#endif
