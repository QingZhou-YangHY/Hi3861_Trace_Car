//这个文件进行一些基本的硬件操作和任务管理
#include "iot_errno.h"
#include "iot_gpio_ex.h"
#include "hi_gpio.h"
#include "hi_io.h"
#include "hi_task.h"
#include "hi_types_base.h"
//设置指定IO口的上拉/下拉电阻
unsigned int IoSetPull(unsigned int id, IotIoPull val)
{
    if (id >= HI_GPIO_IDX_MAX) {
        return IOT_FAILURE;
    }
    return hi_io_set_pull((hi_io_name)id, (hi_io_pull)val);
}
//设置指定IO口的功能
unsigned int IoSetFunc(unsigned int id, unsigned char val)
{
    if (id >= HI_GPIO_IDX_MAX) {
        return IOT_FAILURE;
    }
    return hi_io_set_func((hi_io_name)id, val);
}
//使当前任务延迟指定的毫秒数
unsigned int TaskMsleep(unsigned int ms)
{
    if (ms <= 0) {
        return IOT_FAILURE;
    }
    return hi_sleep((hi_u32)ms);
}