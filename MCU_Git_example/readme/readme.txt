GT411 ����STM32L0ϵ�еİ汾�⹤�̣�����������


1.��ʼ����CUBEX�Զ�����Ȼ�����

2.Track �ļ���Ϊ�Զ����һЩ .c .h �ļ� ���Ż�����

  BSP ��STM32L0xxnucleo �İ���֧�ְ� �ɹ�ʹ��
  
  BSP ֧�� IIC SPI LCD SDIO LED �ȵȹ���
  

3.IIC�ǳɹ���һ�� ����ر���������������




�жϷ���
typedef enum
{
/******  Cortex-M0 Processor Exceptions Numbers ******************************************************/
  NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                                */
  HardFault_IRQn              = -13,    /*!< 3 Cortex-M0+ Hard Fault Interrupt                       */
  SVC_IRQn                    = -5,     /*!< 11 Cortex-M0+ SV Call Interrupt                         */
  PendSV_IRQn                 = -2,     /*!< 14 Cortex-M0+ Pend SV Interrupt                         */
  SysTick_IRQn                = -1,     /*!< 15 Cortex-M0+ System Tick Interrupt                     */

/******  STM32L-0 specific Interrupt Numbers *********************************************************/
  WWDG_IRQn                   = 0,      /*!< Window WatchDog Interrupt                               */
  PVD_IRQn                    = 1,      /*!< PVD through EXTI Line detect Interrupt                  */
  RTC_IRQn                    = 2,      /*!< RTC through EXTI Line Interrupt                         */
  FLASH_IRQn                  = 3,      /*!< FLASH Interrupt                                         */
  RCC_IRQn                    = 4,      /*!< RCC Interrupt                                           */
  EXTI0_1_IRQn                = 5,      /*!< EXTI Line 0 and 1 Interrupts                            */
  EXTI2_3_IRQn                = 6,      /*!< EXTI Line 2 and 3 Interrupts                            */
  EXTI4_15_IRQn               = 7,      /*!< EXTI Line 4 to 15 Interrupts                            */
  DMA1_Channel1_IRQn          = 9,      /*!< DMA1 Channel 1 Interrupt                                */
  DMA1_Channel2_3_IRQn        = 10,     /*!< DMA1 Channel 2 and Channel 3 Interrupts                 */
  DMA1_Channel4_5_6_7_IRQn    = 11,     /*!< DMA1 Channel 4, Channel 5, Channel 6 and Channel 7 Interrupts */
  ADC1_COMP_IRQn              = 12,     /*!< ADC1, COMP1 and COMP2 Interrupts                        */
  LPTIM1_IRQn                 = 13,     /*!< LPTIM1 Interrupt                                        */
  TIM2_IRQn                   = 15,     /*!< TIM2 Interrupt                                          */
  TIM21_IRQn                  = 20,     /*!< TIM21 Interrupt                                         */
  TIM22_IRQn                  = 22,     /*!< TIM22 Interrupt                                         */
  I2C1_IRQn                   = 23,     /*!< I2C1 Interrupt                                          */
  SPI1_IRQn                   = 25,     /*!< SPI1 Interrupt                                          */
  USART2_IRQn                 = 28,     /*!< USART2 Interrupt                                        */
  LPUART1_IRQn                = 29,     /*!< LPUART1 Interrupt                                       */
} IRQn_Type;


�жϷ�����      д��stm32l0xx_it.c
Default_Handler PROC

                EXPORT  WWDG_IRQHandler                [WEAK]
                EXPORT  PVD_IRQHandler                 [WEAK]
                EXPORT  RTC_IRQHandler                 [WEAK]
                EXPORT  FLASH_IRQHandler               [WEAK]
                EXPORT  RCC_IRQHandler                 [WEAK]
                EXPORT  EXTI0_1_IRQHandler             [WEAK]
                EXPORT  EXTI2_3_IRQHandler             [WEAK]
                EXPORT  EXTI4_15_IRQHandler            [WEAK]
                EXPORT  DMA1_Channel1_IRQHandler       [WEAK]
                EXPORT  DMA1_Channel2_3_IRQHandler     [WEAK]
                EXPORT  DMA1_Channel4_5_6_7_IRQHandler [WEAK]
                EXPORT  ADC1_COMP_IRQHandler           [WEAK]
                EXPORT  LPTIM1_IRQHandler              [WEAK]
                EXPORT  TIM2_IRQHandler                [WEAK]
                EXPORT  TIM21_IRQHandler               [WEAK]
                EXPORT  TIM22_IRQHandler               [WEAK]
                EXPORT  I2C1_IRQHandler                [WEAK]
                EXPORT  SPI1_IRQHandler                [WEAK]
                EXPORT  USART2_IRQHandler              [WEAK]
                EXPORT  LPUART1_IRQHandler             [WEAK]




�ⲿ�ж����̣��� ����˳��
//���������ж�����                                           �κ�λ��
void sensor_EXTI_config(void)    //�𶯴��������жϳ�ʼ��
{
	printf("configggggggggggggg \n");
	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
  GPIO_InitStruct.Pin = GPIO_PIN_0;                          //������  ���ж�����
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pull = GPIO_NOPULL;	
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	
  GPIO_InitStruct.Pin =  GPIO_PIN_1;                         //��б�ж�����
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
}

1.  stm32l0xx_it.c
void EXTI0_1_IRQHandler(void)      //�𶯱���������    �жϵ�����-- EXTI0_1_IRQHandler->HAL_GPIO_EXTI_IRQHandler -> HAL_GPIO_EXTI_Callback
{
  /* USER CODE BEGIN EXTI0_1_IRQn 0 */

  /* USER CODE END EXTI0_1_IRQn 0 */
	printf("EXTI0_1_IRQHandler \n");
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);   
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

2.  stm32l0xx_gpio.c
void HAL_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin)
{
  /* EXTI line interrupt detected */
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_Pin) != RESET) 
  { 
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
    HAL_GPIO_EXTI_Callback(GPIO_Pin);
  }
}

3.       ����λ��
 void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	printf("HAL_GPIO_EXTI_Callback\n");
    switch(GPIO_Pin)
    {
        case GPIO_PIN_0:         //sensor �����ж�
		printf("shock warning happened !! ");
        break;		
        case GPIO_PIN_1:
		printf("sensor  sensor  shake shake");
        break;	
    }
}







22222222222222��  ���ڽ��պ���

	 if(HAL_UART_DeInit(&UartHandle) != HAL_OK)
	{
		Error_Handler();
	}  
	if(HAL_UART_Init(&UartHandle) != HAL_OK)
	{
		Error_Handler();
	}
if(HAL_UART_Receive_IT(&UartHandle, (uint8_t *)RXBUFFER, 1) != HAL_OK)
	{
		Error_Handler();
	}

����������������������������������

int main(void)
{
    u8 len;	
	u16 times=0; 
    HAL_Init();                     //��ʼ��HAL��   
    Stm32_Clock_Init(360,25,2,8);   //����ʱ��,180Mhz
    delay_init(180);                //��ʼ����ʱ����
    uart_init(115200);              //��ʼ��USART
    LED_Init();                     //��ʼ��LED 
    KEY_Init();                     //��ʼ������

    while(1)
    {
			
       if(USART_RX_STA&0x8000)
		{					   
			len=USART_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
			printf("\r\n�����͵���ϢΪ:\r\n");
			HAL_UART_Transmit(&UART1_Handler,(uint8_t*)USART_RX_BUF,len,1000);	//���ͽ��յ�������
			while(__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_TC)!=SET);		//�ȴ����ͽ���
			printf("\r\n\r\n");//���뻻��
			USART_RX_STA=0;
		}else
		{
			times++;
			if(times%5000==0)
			{
				printf("\r\nALIENTEK ������STM32F429������ ����ʵ��\r\n");
				printf("����ԭ��@ALIENTEK\r\n\r\n\r\n");
			}
			if(times%200==0)printf("����������,�Իس�������\r\n");  
			if(times%30==0)LED0=!LED0;//��˸LED,��ʾϵͳ��������.
			delay_ms(10);   
		} 
    } 
}






