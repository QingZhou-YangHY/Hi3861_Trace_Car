//定义一个AdcRead函数，将模拟信号转换为数字信号
#ifndef HAL_IOT_ADC_H
#define HAL_IOT_ADC_H
#include "hi_adc.h"
#include "iot_adc.h"
unsigned int AdcRead(IotAdcChannelIndex channel, unsigned const short *data, IotAdcEquModelSel equModel,
    IotAdcCurBais curBais, unsigned const short rstCnt)
{
    return hi_adc_read((hi_adc_channel_index)channel, (hi_u16*)data, (hi_adc_equ_model_sel)equModel,
        (hi_adc_cur_bais)curBais, (hi_u16)rstCnt);
}
#endif