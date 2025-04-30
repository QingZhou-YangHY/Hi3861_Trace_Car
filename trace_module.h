#ifndef TRACE_MODULE_H
#define TRACE_MODULE_H

#include <stdint.h>

typedef enum {
    MODE_ON_OFF = 0,
    MODE_SET_LEFT_FORWARD,
    MODE_SET_RIGHT_FORWARD,
    MODE_SET_TURN_LEFT,
    MODE_SET_TURN_RIGHT,
    MODE_SET_LEFT_ADC,
    MODE_SET_RIGHT_ADC,
    MODE_END,
} ENUM_MODE;

typedef struct {
    uint32_t LeftForward;
    uint32_t RightForward;
    uint32_t TurnLeft;
    uint32_t TurnRight;
    unsigned short leftadcdata;
    unsigned short rightadcdata;
} CAR_DRIVE;

#define     ADC_TEST_LENGTH             (20)
#endif
