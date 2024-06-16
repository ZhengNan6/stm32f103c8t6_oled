/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "gui.h"
#include "bmp.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Write_BMP_Byte(uint8_t Byte);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define BMP_Flash_SAVE  (0x08008000) //使用的地址，扇区32
//检查oled屏幕尺寸定义
#warning "记得检查在oled.h中的屏幕尺寸定义 check the oled size define in oled.h!!!!!!!!!!!!"
uint8_t BMP_Receive_Buff[1024] = {0};
uint64_t BMP_Receive_Byte_num = 0;
uint64_t BMP_Flash_Write_Byte_idx = 0;
uint64_t BMP_Flash_Need_Write_Byte = 0;//需写入flash的字节数

int BMP_Flash_Init(uint32_t Flash_Addr)
{
    //FLASH解锁
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
    
    //FLASH擦除
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;    //页擦除，一页为1kb，刚好储存一张128*64像素图片
    EraseInitStruct.PageAddress = Flash_Addr;    //页地址
    EraseInitStruct.NbPages = 1;    //擦除多少个页
    uint32_t PageError = 0;            //记录擦除出错时的起始地址
    if(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError)!=HAL_OK)
    {
        return 0;
    }
    //FLASH上锁
    HAL_FLASH_Lock();    
    return 1;
}


int Flash_HAL_Write_N_2Byte_Data(uint32_t Write_addr, uint8_t *data, uint16_t num)
{
    //1、FLASH解锁
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
    
//    //2、FLASH擦除
//    FLASH_EraseInitTypeDef EraseInitStruct;
//    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;    //页擦除
//    EraseInitStruct.PageAddress = BMP_Flash_SAVE;    //从第几个页开始擦除（0开始）
//    EraseInitStruct.NbPages = 1;    //擦除多少个页
//    uint32_t PageError = 0;            //记录擦除出错时的起始地址
//    if(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError)!=HAL_OK)
//    {
////        printf("FLASH擦除出错,开始出错地址:%#x\r\n", PageError);
//        return -1;
//    }
    //3、FLASH写入
    for(uint16_t i=0; i<num; i++)
    {
        uint16_t Write_Data = (data[i*2+1] << 8 | data[i*2]);
        if(HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, Write_addr, Write_Data)!=HAL_OK)
        {
//            printf("FLASH写入失败\r\n");
            return -1;
        }
        Write_addr += sizeof(uint16_t);
    }
    //4、FLASH上锁
    HAL_FLASH_Lock();    
    return 0;
}


void Write_BMP_Byte(uint8_t Byte)
{
    static uint16_t Flash_write_time = 0;
}
        uint64_t Write_2Byte_num;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();			         //初始化OLED  
  OLED_Clear(0);             //清屏（全黑）
  memcpy(BMP_Receive_Buff, (uint8_t*)BMP_Flash_SAVE, 1024); //从flash中搬运历史图片
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
      //开机后首次接收到图片信息 或 接收完一整幅图片后再接收到图片 时刷新flash空间
    if (BMP_Flash_Need_Write_Byte > 1 && BMP_Flash_Write_Byte_idx == 0)
        BMP_Flash_Init(BMP_Flash_SAVE);
    
    if (BMP_Flash_Need_Write_Byte >= 2)//由于flash的写入是以2字节为最小单位，所以每有大于或等于两个字节再写入flash
    {
        //uint64_t Write_2Byte_num;
        if (BMP_Flash_Write_Byte_idx + BMP_Flash_Need_Write_Byte > 1023)
        {   //接收完一整幅图片，把上一副图片剩余部分全写入flash,剩下新的图片下次循环再写入
            Write_2Byte_num = (1024-BMP_Flash_Write_Byte_idx)/2;   
        }
        else
        {
            Write_2Byte_num = BMP_Flash_Need_Write_Byte/2;
        }
        //写入图片flash
        Flash_HAL_Write_N_2Byte_Data(BMP_Flash_SAVE+BMP_Flash_Write_Byte_idx*sizeof(uint8_t), 
                                     BMP_Receive_Buff+BMP_Flash_Write_Byte_idx,
                                     Write_2Byte_num);
                                    
        BMP_Flash_Write_Byte_idx += Write_2Byte_num*2;
        BMP_Flash_Write_Byte_idx = BMP_Flash_Write_Byte_idx%1024;
        
        BMP_Flash_Need_Write_Byte -= Write_2Byte_num*2;
    }
    
    GUI_DrawBMP(0, 0, 128, 64, BMP_Receive_Buff, 1);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
