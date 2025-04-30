// 控制一个智能小车的行驶
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio_ex.h"
#include "iot_gpio.h"
#include "iot_adc.h"
#include "hi_adc.h"
#include "iot_errno.h"
#include "ssd1306.h"
#include "pca9555.h"
#include "motor_control.h"
#include "trace_module.h"

static volatile int g_State = 0;

CAR_DRIVE car_drive = {0};
ENUM_MODE g_mode = MODE_ON_OFF;

int g_CarStarted = 0;

#define MASK_BUTTON1 (0x10)
#define MASK_BUTTON2 (0x08)
#define MASK_BUTTON3 (0x04)

void init_ctrl_algo(void)
{
    memset(&car_drive, 0, sizeof(CAR_DRIVE));
    car_drive.LeftForward = 50;   // 10 左轮前进速度
    car_drive.RightForward = 50;  // 10 右轮前进速度
    car_drive.TurnLeft = 50;      // 30 左转弯右轮速度
    car_drive.TurnRight = 50;     // 30 右转弯左轮速度
    car_drive.leftadcdata = 200;  // 200代表左边ADC数据
    car_drive.rightadcdata = 200; // 200代表右边ADC数据
}

void init_oled_mode(void)
{
    g_mode = MODE_ON_OFF;
    ssd1306_ClearOLED();
    // ssd1306_printf("LF:%d, RF:%d", car_drive.LeftForward, car_drive.RightForward);
    // ssd1306_printf("TL:%d, TR:%d", car_drive.TurnRight, car_drive.TurnLeft);
}

void LeftLED(void)
{
    PCA_WriteReg(PCA9555_REG_OUT1, LEFT_LED); /* IO1 012345低电平 */
}

void RightLed(void)
{
    PCA_WriteReg(PCA9555_REG_OUT1, RIGHT_LED); /* IO1 012345低电平 */
}

void LedOff(void)
{
    PCA_WriteReg(PCA9555_REG_OUT1, LED_OFF); /* IO1 012345低电平 */
}

void LedON(void)
{
    PCA_WriteReg(PCA9555_REG_OUT1, LEFT_LED);  /* IO1 012345低电平 */
    PCA_WriteReg(PCA9555_REG_OUT1, RIGHT_LED); /* IO1 012345低电平 */
}

void ButtonDesplay(ENUM_MODE mode)
{
    switch (mode)
    {
    case MODE_ON_OFF:
        ssd1306_printf("LF:%d, RF:%d", car_drive.LeftForward, car_drive.RightForward);
        ssd1306_printf("TL:%d, TR:%d", car_drive.TurnRight, car_drive.TurnLeft);
        ssd1306_printf("leftadcdata:%d", car_drive.leftadcdata);
        ssd1306_printf("rightadcdata:%d", car_drive.rightadcdata);
        break;
    case MODE_SET_LEFT_FORWARD:
        ssd1306_printf("Set LForward=%d", car_drive.LeftForward);
        break;
    case MODE_SET_RIGHT_FORWARD:
        ssd1306_printf("Set RForward=%d", car_drive.RightForward);
        break;
    case MODE_SET_TURN_LEFT:
        ssd1306_printf("Set TurnLeft=%d", car_drive.TurnRight);
        break;
    case MODE_SET_TURN_RIGHT:
        ssd1306_printf("Set TurnRight=%d", car_drive.TurnLeft);
        break;
    case MODE_SET_LEFT_ADC:
        ssd1306_printf("Set LeftADC=%d", car_drive.leftadcdata);
        break;
    case MODE_SET_RIGHT_ADC:
        ssd1306_printf("Set RightADC=%d", car_drive.rightadcdata);
        break;
    default:
        break;
    }
}

void ButtonSet(ENUM_MODE mode, bool button_pressed)
{
    printf("mode = %d\r\n", mode);
    switch (mode)
    {
    case MODE_ON_OFF:
        g_CarStarted = !g_CarStarted;
        ssd1306_ClearOLED();
        printf("g_CarStarted = %d\r\n", g_CarStarted);
        ssd1306_printf(g_CarStarted ? "start" : "stop");
        break;
    case MODE_SET_LEFT_FORWARD:
        car_drive.LeftForward += ((button_pressed) ? -1 : 1);
        ssd1306_printf("LeftForward=%d", car_drive.LeftForward);
        break;
    case MODE_SET_RIGHT_FORWARD:
        car_drive.RightForward += (button_pressed ? -1 : 1);
        ssd1306_printf("RightForward=%d", car_drive.RightForward);
        break;
    case MODE_SET_TURN_LEFT:
        car_drive.TurnRight += ((button_pressed) ? -1 : 1);
        ssd1306_printf("TurnLeft=%d", car_drive.TurnRight);
        break;
    case MODE_SET_TURN_RIGHT:
        car_drive.TurnLeft += ((button_pressed) ? -1 : 1);
        ssd1306_printf("TurnRight=%d", car_drive.TurnLeft);
        break;
    case MODE_SET_LEFT_ADC:
        car_drive.leftadcdata += ((button_pressed) ? -10 : 10); // 10代表adc正负10
        ssd1306_printf("TurnRight=%d", car_drive.leftadcdata);
        break;
    case MODE_SET_RIGHT_ADC:
        car_drive.rightadcdata += ((button_pressed) ? -10 : 10); // 10代表adc正负10
        ssd1306_printf("TurnRight=%d", car_drive.rightadcdata);
        break;
    default:
        break;
    }
}

void ButtonPressProc(uint8_t ext_io_val)
{
    static uint8_t ext_io_val_d = 0xFF;
    uint8_t diff;
    bool button1_pressed, button2_pressed, button3_pressed;
    diff = ext_io_val ^ ext_io_val_d;
    button1_pressed = ((diff & MASK_BUTTON1) && ((ext_io_val & MASK_BUTTON1) == 0)) ? true : false;
    button2_pressed = ((diff & MASK_BUTTON2) && ((ext_io_val & MASK_BUTTON2) == 0)) ? true : false;
    button3_pressed = ((diff & MASK_BUTTON3) && ((ext_io_val & MASK_BUTTON3) == 0)) ? true : false;
    ssd1306_ClearOLED();
    if (button1_pressed)
    {
        g_mode = (g_mode >= (MODE_END - 1)) ? 0 : (g_mode + 1);
        ButtonDesplay(g_mode);
    }
    else if (button2_pressed || button3_pressed)
    {
        ButtonSet(g_mode, button2_pressed);
    }
    ext_io_val_d = ext_io_val;
}

/*
 * init gpio11/12 as a input io
 * GPIO 11 connects the left tracking module
 * GPIO 11 connects the right tracking module
 */
void trace_module_init(void)
{
    // 设置GPIO07的管脚复用关系为GPIO
    IoSetFunc(IOT_IO_NAME_GPIO_7, IOT_IO_FUNC_GPIO_7_GPIO);
    // 设置GPIO07的管脚方向为入
    IoTGpioSetDir(IOT_IO_NAME_GPIO_7, IOT_GPIO_DIR_IN);

    // GPIO12初始化
    // 设置GPIO012的管脚复用关系为GPIO
    IoSetFunc(IOT_IO_NAME_GPIO_12, IOT_IO_FUNC_GPIO_12_GPIO);
    // 设置GPIO12的管脚方向为入
    IoTGpioSetDir(IOT_IO_NAME_GPIO_12, IOT_GPIO_DIR_IN);
}

IotGpioValue get_do_value(IotAdcChannelIndex idx)
{
    unsigned short data = 0;
    int ret = -1;

    for (int i = 0; i < ADC_TEST_LENGTH; i++)
    {
        // ADC_Channal_6  自动识别模式  CNcomment:4次平均算法模式 CNend
        ret = AdcRead(idx, &data, IOT_ADC_EQU_MODEL_4, IOT_ADC_CUR_BAIS_DEFAULT, 0xF0);
        if (ret != HI_ERR_SUCCESS)
        {
            printf("hi_adc_read failed\n");
        }
    }

    if (idx == IOT_ADC_CHANNEL_3)
    {
        printf("gpio7 m_right_value is %d\n", data);
    }
    else if (idx == IOT_ADC_CHANNEL_0)
    {
        printf("gpio12 m_left_value is %d\n", data);
    }

    if (data > car_drive.rightadcdata && idx == IOT_ADC_CHANNEL_3)
    {
        ret = 0;
    }
    else if ((data > car_drive.leftadcdata) && idx == IOT_ADC_CHANNEL_0)
    {
        ret = 1;
    }
    else if (data < car_drive.rightadcdata && idx == IOT_ADC_CHANNEL_3)
    {
        ret = 2; // 2代表右边在白线的状态
    }
    else if (data < car_drive.leftadcdata && idx == IOT_ADC_CHANNEL_0)
    {
        ret = 3; // 3代表左边在白线的状态
    }

    return ret;
}
// 初始化PCA9555、GA12N20、轨迹模块、控制算法和OLED显示
void TraceExampleTask(void)
{
    int m_left_value;
    int m_right_value;
    InitPCA9555();
    GA12N20Init();
    trace_module_init();
    TaskMsleep(100); // 等待100ms初始化完成
    init_ctrl_algo();
    init_oled_mode();
    PCA_RegisterEventProcFunc(ButtonPressProc);
    while (1)
    {
        if (g_State == 1 && g_CarStarted)
        {
            m_right_value = get_do_value(IOT_ADC_CHANNEL_3); // gpio7 ==>ADC5
            m_left_value = get_do_value(IOT_ADC_CHANNEL_0);  // gpio12 ==>ADC0
            ssd1306_printf("m_right:%d", m_right_value);
            ssd1306_printf("m_left:%d", m_left_value);
            if ((m_left_value == 3) && (m_right_value == 0))
            { // 左偏，向右转 3代表左边在白线的状态
                car_right(car_drive.TurnLeft);
                RightLed();
            }
            else if ((m_left_value == 1) && (m_right_value == 2))
            { // 右偏，向左转 2代表右边在白线的状态
                car_left(car_drive.TurnRight);
                LeftLED();
            }
            else if ((m_left_value == 3) && (m_right_value == 2))
            { // 2,3代表左右两边都在白线
                car_forward(car_drive.LeftForward, car_drive.RightForward);
                LedOff();
            }
            else
            { // 脱离轨道
                car_stop();
                LedON();
            }
            g_State = 0;
        }
        else if (!g_CarStarted)
        {
            car_stop();
        }
    }
}
start void cb_timeout_periodic(void *arg)
{
    (void)arg;
    g_State = 1;
}

void timer_periodic(void)
{
    osTimerId_t periodic_tid = osTimerNew(cb_timeout_periodic, osTimerPeriodic, NULL, NULL);
    if (periodic_tid == NULL)
    {
        printf("[Timer Test] osTimerNew(periodic timer) failed.\r\n");
        return;
    }
    else
    {
        printf("[Timer Test] osTimerNew(periodic timer) success, tid: %p.\r\n", periodic_tid);
    }
    osStatus_t status = osTimerStart(periodic_tid, 10);
    if (status != osOK)
    {
        printf("[Timer Test] osTimerStart(periodic timer) failed.\r\n");
        return;
    }
    else
    {
        printf("[Timer Test] osTimerStart(periodic timer) success, wait a while and stop.\r\n");
    }
}

// 入口
void TraceExampleEntry(void)
{
    osThreadAttr_t attr;
    timer_periodic();
    attr.name = "adcTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 5 * 1024; // 堆栈大小为5 *1024
    attr.priority = osPriorityNormal;
    if (osThreadNew((osThreadFunc_t)TraceExampleTask, NULL, &attr) == NULL)
    {
        printf("[LSM6DSTask] Failed to create LSM6DSTask!\n");
    }
}
APP_FEATURE_INIT(TraceExampleEntry);