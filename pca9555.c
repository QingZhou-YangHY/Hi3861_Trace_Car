//初始化和操作PCA9555 I/O扩展芯片，并通过I2C接口与之通信
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "iot_gpio_ex.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "iot_pwm.h"
#include "hi_pwm.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_i2c.h"
#include "iot_errno.h"
#include "hi_errno.h"
#include "iot_watchdog.h"
#include "hi_i2c.h"
#include "hi_time.h"
#include "iot_gpio.h"
#include "pca9555.h"
#define PCA5555_I2C_IDX 0
#define IOT_I2C_IDX_BAUDRATE         400000 // 400k
#define PCA9555_INT_PIN_NAME    (IOT_IO_NAME_GPIO_11)
#define PCA9555_INT_PIN_FUNC    (IOT_IO_FUNC_GPIO_11_GPIO)
#define PCA_REG_IO0_STATE       (0x00)
volatile int g_extIoIntValid = 0;
PCA_EventProcFunc g_PCAEventProcFunc = NULL;
uint32_t PCA_WriteReg(uint8_t reg_addr, uint8_t reg_val)
{
    uint8_t buffer[2];
    buffer[0] = reg_addr;
    buffer[1] = reg_val;
    uint32_t retval = IoTI2cWrite(PCA5555_I2C_IDX, PCA9555_WRITE_ADDR, buffer, 2);
    if (retval != IOT_SUCCESS) {
        printf("IoTI2cWrite failed\n");
        return retval;
    }
    return IOT_SUCCESS;
}
uint32_t PCA_ReadReg(uint8_t reg_addr, uint8_t *reg_val)
{
    uint32_t status;
    status = IoTI2cWrite(PCA5555_I2C_IDX, PCA9555_WRITE_ADDR, &reg_addr, 1);
    if (status != IOT_SUCCESS) {
        printf("IOTI2cRead phase1 failed\n");
        return 0;
    }
    status = IoTI2cRead(PCA5555_I2C_IDX, PCA9555_READ_ADDR, reg_val, 1);
    if (status != IOT_SUCCESS) {
        printf("IOTI2cRead phase2 failed\n");
        return 0;
    }
    return status;
}
uint32_t PCA_ReadExtIO0(uint8_t *reg_val)
{
    return PCA_ReadReg(PCA_REG_IO0_STATE, reg_val);
}
//外部中断服务函数，当PCA9555的中断引脚触发时，将g_extIoIntValid标记为1。
void ExtIoIntSvr(char *arg)
{
    (void) arg;
    g_extIoIntValid = 1;
}
/*初始化PCA9555，包括I2C接口初始化、GPIO引脚配置、中断引脚注册等。
还初始化了SSD1306 OLED显示屏，并配置了PCA9555的输入输出模式。*/
void InitPCA9555(void)
{
    /* init i2c */
    printf("init pca95555\n");
    IoTI2cInit(0, IOT_I2C_IDX_BAUDRATE); /* baudrate: 400000 */
    IoTI2cSetBaudrate(0, IOT_I2C_IDX_BAUDRATE);
    IoSetFunc(IOT_IO_NAME_GPIO_13, IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(IOT_IO_NAME_GPIO_14, IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    /* register GPIO_11 as INT pin */
    IoTGpioInit(PCA9555_INT_PIN_NAME);
    IoSetFunc(PCA9555_INT_PIN_NAME, PCA9555_INT_PIN_FUNC);
    IoTGpioSetDir(PCA9555_INT_PIN_NAME, IOT_GPIO_DIR_IN);
    IoSetPull(PCA9555_INT_PIN_NAME, IOT_IO_PULL_UP);
    IoTGpioRegisterIsrFunc(PCA9555_INT_PIN_NAME, IOT_INT_TYPE_EDGE, IOT_GPIO_EDGE_FALL_LEVEL_LOW, ExtIoIntSvr, NULL);
    ssd1306_Init(); // 初始化 SSD1306 OLED模块
    ssd1306_Fill(Black);
    /* IO0_X全为可选输入 */
    /* 输入为1，输出为0，IO0 234输入,0x1f代表按键加寻迹，0x7c代表按键编码器同时使用，0x1c代表只用按键 */
    PCA_WriteReg(PCA9555_REG_CFG0, 0x1c);
    /* IO1_X全为输出 */
    PCA_WriteReg(PCA9555_REG_CFG1, 0x00); /* IO1 012345输出 */
    PCA_WriteReg(PCA9555_REG_OUT1, LED_OFF); /* IO1 012345低电平 */
}
//用于注册和注销处理PCA9555事件的函数。
void PCA_RegisterEventProcFunc(PCA_EventProcFunc func)
{
    g_PCAEventProcFunc = func;
}
void PCA_UnregisterEventProcFunc(void)
{
    g_PCAEventProcFunc = NULL;
}
//用于处理PCA9555的错误状态恢复，检测中断引脚的低电平状态并读取IO0状态。
void PressToRestore(void)
{
    uint8_t ext_io_state = 0;
    IotGpioValue value = 0;
    uint8_t intLowFlag = 0;
    uint32_t cTick = 0;
    uint8_t status;
    status = IoTGpioGetInputVal(PCA9555_INT_PIN_NAME, &value);
    if (status != IOT_SUCCESS) {
        printf("status = %d\r\n", status);
    }
    if (value == 1) {
        intLowFlag = 0;
    } else {
        if (intLowFlag == 0) {
            cTick = hi_get_milli_seconds();
            intLowFlag = 1;
        } else {
            if ((hi_get_milli_seconds() - cTick) > 2) { // 2ms
                status = PCA_ReadExtIO0(&ext_io_state);
                intLowFlag = 0;
            }
        }
    }
}
/*主线程函数，不断检查外部中断标志g_extIoIntValid，如果有效则读取IO0状态并调用事件处理函数。
如果没有中断，调用PressToRestore进行错误状态恢复。*/
void PCA_MainThread(void)
{
    uint8_t ext_io0_state;
    uint8_t status;
    InitPCA9555();
    while (1) {
        /* check button pressed interval */
        TaskMsleep(50); // 50ms
        if (g_extIoIntValid == 1) {
            status = PCA_ReadExtIO0(&ext_io0_state);
            if (status != IOT_SUCCESS) {
                printf("i2c error!\r\n");
                g_extIoIntValid = 0;
                return;
            } else if (g_PCAEventProcFunc != NULL) {
                g_PCAEventProcFunc(ext_io0_state);
            }
            g_extIoIntValid = 0;
        } else {
            /* PCA9555 error state recovery */
            PressToRestore();
        }
    }
}
//创建并启动PCA9555的主线程。使用CMSIS-RTOS v2 API创建任务，设置任务属性如名称、优先级、堆栈大小等。
void PCA_CreateThread(void)
{
    osThreadAttr_t attr;
    IoTWatchDogDisable();
    attr.name = "PCA95555Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 5 * 1024; // 堆栈大小5*1024
    attr.priority = osPriorityNormal;
    if (osThreadNew((osThreadFunc_t)PCA_MainThread, NULL, &attr) == NULL) {
        printf("Failed to create Pca9555Task!\n");
    }
}
APP_FEATURE_INIT(PCA_CreateThread);