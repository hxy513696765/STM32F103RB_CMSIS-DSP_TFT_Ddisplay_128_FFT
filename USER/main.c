#include "led.h"
#include "delay.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "adc.h"

//ALIENTEK Mini STM32开发板范例代码11
//TFTLCD显示实验   
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司 

#define TEST_LENGTH_SAMPLES 512
/* -------------------------------------------------------------------
* External Input and Output buffer Declarations for FFT Bin Example
* ------------------------------------------------------------------- */
float32_t testInput_f32_10khz[TEST_LENGTH_SAMPLES];
//extern const float32_t demoarry[TEST_LENGTH_SAMPLES];
float32_t testOutput[TEST_LENGTH_SAMPLES/2];
float32_t dispOutput[TEST_LENGTH_SAMPLES/2];


/* ------------------------------------------------------------------
* Global variables for FFT Bin Example
* ------------------------------------------------------------------- */
uint32_t fftSize = 256;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;
uint32_t loop,asb_val;

/* Reference index at which max energy of bin ocuurs */
uint32_t refIndex = 213, testIndex = 0;
u16 lcd_index;

/* ----------------------------------------------------------------------
* Max magnitude FFT Bin test
* ------------------------------------------------------------------- */
 int main(void)
 { 
	arm_status status;
	float32_t maxValue;
	 
//	u8 x=0;
	u16 loop = 0;
	u8 lcd_id[12];			//存放LCD ID字符串	
	delay_init();	    	 //延时函数初始化	  
	uart_init(115200);	 	//串口初始化为9600
	LED_Init();		  		//初始化与LED连接的硬件接口	
 	LCD_Init();
	
	ADC_INIT();		  		//ADC初始化	
	POINT_COLOR=RED; 
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组。	
	printf("%s\r\n",lcd_id);

	LCD_Clear(BLACK);
	//LCD_Fill(0,0,128,128,YELLOW);
  	while(1) 
	{		 

#if 0
		switch(x)
		{
			case 0:LCD_Clear(WHITE);break;
			case 1:LCD_Clear(BLACK);break;
			case 2:LCD_Clear(BLUE);break;
			case 3:LCD_Clear(RED);break;
			case 4:LCD_Clear(MAGENTA);break;
			case 5:LCD_Clear(GREEN);break;
			case 6:LCD_Clear(CYAN);break;

			case 7:LCD_Clear(YELLOW);break;
			case 8:LCD_Clear(BRRED);break;
			case 9:LCD_Clear(GRAY);break;
			case 10:LCD_Clear(LGRAY);break;
			case 11:LCD_Clear(BROWN);break;
		}
		POINT_COLOR=RED;	  
		LCD_ShowString(30,40,200,24,24,"Mini STM32 ^_^");	
		LCD_ShowString(30,70,200,16,16,"TFTLCD TEST");
		LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
 		LCD_ShowString(30,110,200,16,16,lcd_id);		//显示LCD ID	      					 
		LCD_ShowString(30,130,200,12,12,"2014/3/7");	      					 
	    x++;
		if(x==12)x=0;
		LED0=!LED0;	 
		//delay_ms(1000);	
#endif

		
		arm_cfft_radix4_instance_f32 S;	/* ARM CFFT module */
		uint32_t maxIndex;				/* Index in Output array where max value is */
		
		for(loop = 0;loop < TEST_LENGTH_SAMPLES;loop++)
		{
			//GetADC();
			testInput_f32_10khz[loop] = Get_Adc_Average(3)-2048;//demoarry[loop];
		}

		/* Initialize the CFFT/CIFFT module, intFlag = 0, doBitReverse = 1 */
		arm_cfft_radix4_init_f32(&S, fftSize, 0, 1);
		//printf("#1 \r\n");

		/* Process the data through the CFFT/CIFFT module */
		arm_cfft_radix4_f32(&S, testInput_f32_10khz);
		//LCD_Clear(BLACK);


//		for(loop = 0;loop < 32;loop++)
//			LCD_Fill(0,(loop << 3)+(loop << 1),32+loop,(loop << 3)+(loop << 1)+8,YELLOW);

		//printf("\r\n====\r\n");
		//printf("#2 \r\n");

		/* Process the data through the Complex Magniture Module for calculating the magnitude at each bin */
		arm_cmplx_mag_f32(testInput_f32_10khz, testOutput, fftSize);
		//printf("#3 \r\n");

		/* Calculates maxValue and returns corresponding value */
		testOutput[0] = 0;
		testOutput[1] = 0;
		arm_max_f32(testOutput, fftSize/2, &maxValue, &maxIndex);
		//printf("#4 \r\n");
		
		if( status != ARM_MATH_SUCCESS)
		{
	//	while(1);
		}
		if(maxValue >= 240)
			maxValue = maxValue / 100;///2000;
		//else
		//	maxValue = 240 / maxValue;
		//maxValue = 1;
		for(loop=2;loop<(fftSize>>1);loop++)
		{
			lcd_index = loop - 1;
#if 0	// 经过处理后的带拖影的仿VLC播放器效果
			asb_val = abs(testInput_f32_10khz[loop]) % 240;
			//if(asb_val >= 240)
			//	asb_val = 240;
			if(dispOutput[loop]<asb_val)
				dispOutput[loop] = asb_val;
			else
				if(dispOutput[loop] - 10 >= 0)
					dispOutput[loop] = dispOutput[loop] - 10;
				else
					dispOutput[loop]=0;

				
		LCD_Fill(0,(lcd_index << 1)+lcd_index,dispOutput[loop],(lcd_index << 1)+lcd_index+1,YELLOW);			

#else	// 正常未处理的频谱FFT效果	
		asb_val = abs(testOutput[loop]) / maxValue;
		//if(loop>15)
		//	asb_val = asb_val << 3;
		if(asb_val > 240)
			asb_val = 200 + (asb_val % 40);
	#if 1 // 带拖影效果
		//if(asb_val >= 240)
		//	asb_val = 240;
		if(dispOutput[loop]<asb_val)
			dispOutput[loop] = asb_val;
		else
			if(dispOutput[loop] - 40 >= 0)
				dispOutput[loop] = dispOutput[loop] - 40;
			else
				dispOutput[loop]=0;
			
		LCD_Fill(0,(lcd_index << 1)+lcd_index,dispOutput[loop],(lcd_index << 1)+lcd_index+1,YELLOW);
	#else // 不带拖影效果
		LCD_Fill(0,(lcd_index << 1)+lcd_index,asb_val,(lcd_index << 1)+lcd_index+1,YELLOW);
	#endif

#endif
			//LCD_Fill(0,(lcd_index << 1)+lcd_index,asb_val%240,(lcd_index << 1)+lcd_index+1,YELLOW);
			//LCD_Fill(0,(loop << 3)+(loop << 1),abs(testInput_f32_10khz[loop])/maxValue,(loop << 3)+(loop << 1)+8,YELLOW);
//			printf("%d\r\n",Get_Adc(ADC_Channel_9));
			
			//printf("ADC:%d\r\n",ADC_value);
			//delay_ms(100);	
			//printf("%f ",testInput_f32_10khz[loop]);
			//testOutput[loop] = 0;
		}

		sprintf((char*)lcd_id,"%d:%f",maxIndex,maxValue);
		LCD_ShowString(180,300,200,12,12,lcd_id);	 
		//delay_ms(20);
		//delay_ms(50);
		
		//printf("# 5\r\n");

		#if 0
		for(loop=0;loop<(fftSize>>1);loop++)
		{
			printf("%f ",testOutput[loop]);
		}
		printf("\r\n====\r\n");
		#endif
	} 
}
