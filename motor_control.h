#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H
void GA12N20Init(void);
void car_backward(uint32_t left_pwm_value, uint32_t right_pwm_value);
void car_forward(uint32_t left_pwm_value, uint32_t right_pwm_value);
void car_left(uint32_t right_pwm_value);
void car_right(uint32_t left_pwm_value);
void car_stop(void);
#endif  // AHT20_H