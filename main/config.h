#ifndef CONFIG_H
#define CONFIG_H

#include "orientation.h"

typedef enum {
    AUTOMATIC,
    MANUAL
} config_mode_t;

typedef struct {
    config_mode_t mode;
    orientation_t manual_orientation;
} config_t;

#endif // CONFIG_H
