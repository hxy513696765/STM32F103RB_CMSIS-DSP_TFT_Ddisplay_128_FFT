#ifndef	_ADC_H_
#define	_ADC_H_
//#include  "STM32F0XX_CONF.H"
#include  "stm32f10x.h"
#include  "stm32f10x_adc.h"
#include  "stm32f10x_dma.h"
/* 温度传感器标定值地址 */
#define TEMP110_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7C2))
#define TEMP30_CAL_ADDR  ((uint16_t*) ((uint32_t) 0x1FFFF7B8))
#define VOLTAGE_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FFFF7BA))
#define ADC1_DR_Address	((uint32_t)0x4001244C)				//ADC数据转换完成后的结果保存地址
extern u16 ADCConvertedValue;

extern unsigned int ADC_value;
void ADC_INIT(void);
u16 GetADC(void);
u16 Get_Adc_Average(u8 times);
#endif

