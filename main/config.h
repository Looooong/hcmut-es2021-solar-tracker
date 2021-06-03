#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "types.h"

typedef enum {
    AUTOMATIC,
    MANUAL
} config_mode_t;

typedef struct {
    config_mode_t mode;
    orientation_t manual_orientation;
} config_t;

#endif // __CONFIG_H__
