#ifndef _HHP_CONFIG_H_
#define _HHP_CONFIG_H_

/**
 * Timeout when wait for response message
 */
#ifndef HHP_CONFIG_TIMEOUT_MS
#   define HHP_CONFIG_TIMEOUT_MS 10
#endif

/**
 * Times count when wait for response message
 */
#ifndef HHP_CONFIG_TIMEOUT_CNT
#   define HHP_CONFIG_TIMEOUT_CNT 3
#endif

/**
 * Debug OUTPUT
 */
#ifndef HHP_CONFIG_DEBUG
#   define HHP_CONFIG_DEBUG 1
#endif

#if HHP_CONFIG_DEBUG == 1
#define HHP_DEBUG(...)    \
    {\
    printf("DEBUG:   %s L#%d ", __PRETTY_FUNCTION__, __LINE__);  \
    printf(__VA_ARGS__); \
    printf("\r\n"); \
    }
#define HHP_DEBUG_HEXSTRING(index, data, len)    \
    {\
    for (index = 0; index < len; index++) {     \
        printf("%02X", data[index]); \
    }\
    printf("\r\n"); \
    }
#else
#define HHP_DEBUG(...)
#define HHP_DEBUG_HEXSTRING(...)
#endif

/**
 * Error OUTPUT
 */
#ifndef HHP_CONFIG_ERROR
#   define HHP_CONFIG_ERROR 0
#endif

#if HHP_CONFIG_ERROR == 1
#define HHP_ERROR(...)    \
    {\
    printf("ERROR:   %s L#%d ", __PRETTY_FUNCTION__, __LINE__);  \
    printf(__VA_ARGS__); \
    printf("\r\n"); \
    }
#else
#define HHP_ERROR(...)
#endif

#endif /* _HHP_CONFIG_H_ */
