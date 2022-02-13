#include "adc.h"
#include "stdio.h"

uint16_t RegularConvData_Tab[2];
unsigned int ADC_value = 0;
u16 ADCConvertedValue;


/***********************************************************************************
����:ADC_INIT(void)
����:ADC��ʼ������
����:��
�汾:V0.0.0
����:XiaoNiu
��ע:��ʼ��ADC�Ĵ���
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
	DMA_DeInit(DMA1_Channel1);			//����DMA1�ĵ�һͨ��
	DMA_InitStructure.DMA_PeripheralBaseAddr =  (u32)&ADC1->DR;//ADC1_DR_Address; 		//DMA��Ӧ���������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;   //�ڴ�洢����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;					//DMA��ת��ģʽΪSRCģʽ����������Ƶ��ڴ�
	DMA_InitStructure.DMA_BufferSize = 1;		 //DMA�����С��1��
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //����һ�����ݺ��豸��ַ��ֹ����
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;  //�رս���һ�����ݺ�Ŀ���ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //�����������ݿ��Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  //DMA�������ݳߴ磬HalfWord����Ϊ16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   //ת��ģʽ��ѭ������ģʽ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;   //DMA���ȼ���
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;		//M2Mģʽ����
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);		  
	
	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;		  //������ת��ģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;		//����ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	 //��������ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;   //ADC�ⲿ���أ��ر�״̬
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	 //���뷽ʽ,ADCΪ12λ�У��Ҷ��뷽ʽ
	ADC_InitStructure.ADC_NbrOfChannel = 1;    //����ͨ������1��
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channel9 configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_71Cycles5);
					   //ADCͨ���飬 ��9��ͨ�� ����˳��1��ת��ʱ��	
	
	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);	//ADC���ʹ��
	
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);	//����ADC1
	
	/* Enable ADC1 reset calibaration register */	
	ADC_ResetCalibration(ADC1); 	//����У׼
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));  //�ȴ�����У׼���
	
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1); 	  //��ʼУ׼
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));		 //�ȴ�У׼���
	   
	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);   //����ת����ʼ��ADCͨ��DMA��ʽ���ϵĸ���RAM����

}
/***********************************************************************************
����:GetTemp(void)
����:��оƬ�ڲ��¶ȴ�������ADCֵת���ɶ�Ӧ���¶�ֵ
����:��
�汾:V0.0.0
����:XiaoNiu
��ע:
�¶�ת�����㹫ʽ��
			������������������������������������������������������������������������������
			��						  110 ��	-	30 ��								��
			��Temperature (int ��) = �������������������� �� (TS_DATA - TS_CAL1) + 30�� ��
			��						   TS_CAL2 - TS_CAL1								��
			������������������������������������������������������������������������������
			TS_CAL2 Ϊ110���Ӧ��ADֵ���ٷ��ĵ���ʾ��ֵ������ 0x1FFFF7C2 ��ַ��
			TS_CAL2 Ϊ 30���Ӧ��ADֵ���ٷ��ĵ���ʾ��ֵ������ 0x1FFFF7B8 ��ַ��
			TS_DATA Ϊ��ǰ�ɵ����¶ȴ�������ADֵ
************************************************************************************/
u16 GetADC(void)
{

//	int32_t temperature = 0; /* ������ת����ADCֵת��Ϊ��Ӧ���¶�ֵ */
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


