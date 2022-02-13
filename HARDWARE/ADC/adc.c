#include "adc.h"
#include "stdio.h"

uint16_t RegularConvData_Tab[2];
unsigned int ADC_value = 0;
u16 ADCConvertedValue;


/***********************************************************************************
函数:ADC_INIT(void)
功能:ADC初始化函数
参数:无
版本:V0.0.0
作者:XiaoNiu
备注:初始化ADC寄存器
************************************************************************************/
void ADC_INIT(void)
{
    ADC_InitTypeDef     ADC_InitStructure;
    DMA_InitTypeDef     DMA_InitStructure;    
    GPIO_InitTypeDef    GPIO_Temp; 

    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);  
    
    GPIO_Temp.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_Temp.GPIO_Pin   = GPIO_Pin_0;  
    GPIO_Init(GPIOB,&GPIO_Temp);               // 
    
	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel1);			//开启DMA1的第一通道
	DMA_InitStructure.DMA_PeripheralBaseAddr =  (u32)&ADC1->DR;//ADC1_DR_Address; 		//DMA对应的外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;   //内存存储基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//DMA的转换模式为SRC模式，由外设搬移到内存
	DMA_InitStructure.DMA_BufferSize = 1;		 //DMA缓存大小，1个
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //接收一次数据后，设备地址禁止后移
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;  //关闭接收一次数据后，目标内存地址后移
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //定义外设数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  //DMA搬移数据尺寸，HalfWord就是为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   //转换模式，循环缓存模式。
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;   //DMA优先级高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		//M2M模式禁用
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);		  
	
	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;		  //独立的转换模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;		//开启扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	 //开启连续转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;   //ADC外部开关，关闭状态
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	 //对齐方式,ADC为12位中，右对齐方式
	ADC_InitStructure.ADC_NbrOfChannel = 1;    //开启通道数，1个
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channel9 configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_71Cycles5);
					   //ADC通道组， 第9个通道 采样顺序1，转换时间	
	
	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);	//ADC命令，使能
	
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);	//开启ADC1
	
	/* Enable ADC1 reset calibaration register */	
	ADC_ResetCalibration(ADC1); 	//重新校准
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));  //等待重新校准完成
	
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1); 	  //开始校准
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));		 //等待校准完成
	   
	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);   //连续转换开始，ADC通过DMA方式不断的更新RAM区。

}
/***********************************************************************************
函数:GetTemp(void)
功能:把芯片内部温度传感器的ADC值转换成对应的温度值
参数:无
版本:V0.0.0
作者:XiaoNiu
备注:
温度转换计算公式：
			┌─────────────────────────────────────┐
			│						  110 ℃	-	30 ℃								│
			│Temperature (int ℃) = ────────── × (TS_DATA - TS_CAL1) + 30℃ │
			│						   TS_CAL2 - TS_CAL1								│
			└─────────────────────────────────────┘
			TS_CAL2 为110℃对应的AD值，官方文档提示其值储存在 0x1FFFF7C2 地址处
			TS_CAL2 为 30℃对应的AD值，官方文档提示其值储存在 0x1FFFF7B8 地址处
			TS_DATA 为当前采到的温度传感器的AD值
************************************************************************************/
u16 GetADC(void)
{

//	int32_t temperature = 0; /* 用来把转换的ADC值转换为对应的温度值 */
	while((DMA_GetFlagStatus(DMA1_FLAG_TC1)) == RESET ); 
	DMA_ClearFlag(DMA1_FLAG_TC1);

	return ADCConvertedValue;
	
    //ADC_value = ADCConvertedValue;//RegularConvData_Tab[0];
    //printf("%d\r\n",ADC_value);
//	temperature = ((int32_t) RegularConvData_Tab[0] - (int32_t) *TEMP30_CAL_ADDR );
//	temperature = temperature * (int32_t)(110 - 30);
//	temperature = temperature / (int32_t)(*TEMP110_CAL_ADDR - *TEMP30_CAL_ADDR);
//	temperature = temperature + 30;
//	return temperature;
}
u16 Get_Adc_Average(u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		
		temp_val+=GetADC();
		//delay_ms(5);
	}
	return temp_val/times;
} 	


