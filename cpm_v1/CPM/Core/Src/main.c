/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>  // abs() 함수를 위해 추가
#include "encoder.h"
#include "servo.h"
#include "HX711.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// 프로토콜 데이터 구조체 정의
typedef struct {
  uint8_t startByte1;
  uint8_t startByte2;
  uint8_t dataLength;
  uint8_t command;
  uint8_t objNum1;
  uint8_t objNum2;
  uint8_t dataByte;
  uint8_t data1;
  uint8_t data2;
} ProtocolData_t;

typedef enum {
  CPM_STATE_IDLE = 0,        // 대기 상태
  CPM_STATE_RUNNING,         // 동작 중
  CPM_STATE_PAUSED,         // 일시정지
  CPM_STATE_STOP,           //정지
  CPM_STATE_EMERGENCY_STOP   // 비상정지
} CPM_State_t;

// CPM 동작 상태 정의
typedef enum {
  CPM_IDLE = 0,
  CPM_ACTIVE,
  CPM_PASSIVE,
  CPM_FORCE_MEASURE  // 힘 측정 모드 추가
} CPM_Mode_t;

// 액티브 모드 파라미터
typedef struct {
  uint8_t speed;          // 동작 속도
  uint8_t angle;          // 동작 각도
  uint8_t repeat;         // 반복 횟수
  uint8_t delay;          // 딜레이
} Active_Params_t;

// 패시브 모드 서브 모드
typedef enum {
  PASSIVE_ASSIST = 0,   // 서포트 모드 (도와주는 모드)
  PASSIVE_RESIST,
  PASSIVE_PROPORTIONAL// 저항 모드
} PassiveMode_t;

// 패시브 모드 파라미터
typedef struct {
  PassiveMode_t mode;   // PASSIVE_ASSIST(서포트) 또는 PASSIVE_RESIST(저항)
  uint8_t torque_level; // 토크 레벨 (1-10)
} Passive_Params_t;

// CPM 메인 구조체
typedef struct {
  CPM_State_t state;    // CPM 동작 상태
  CPM_Mode_t mode;      // 현재 동작 모드 (액티브/패시브)
  Active_Params_t active;    // 액티브 모드 파라미터
  Passive_Params_t passive;  // 패시브 모드 파라미터
} CPM_Params_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define START_BYTE1     0x5A
#define START_BYTE2     0xA5
#define DATA_LENGTH     9
#define MAX_TORQUE_LEVEL    100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t ir_sensor_flag =0;

static volatile bool is_motor_enabled = true;
char uart_buf[100];  // UART 출력용 버퍼

uint8_t rxData;
uint8_t rxBuffer[DATA_LENGTH];
uint8_t dataIndex = 0;
ProtocolData_t protocolData;

static int repeat_count = 0;
static bool cpm_initialized = false;
static bool moving_forward = true;
static bool params_updated = false;   // 파라미터가 업데이트되었는지 표시하는 플래그
static bool stop = false;
static uint32_t timer_counter = 0;

// CPM 파라미터 초기화
static CPM_Params_t cpm_params = {
  .state = CPM_STATE_RUNNING,
  .mode = CPM_ACTIVE,
  .active = {
    .speed = 1,       // 속도 초기값 (1~10)
    .angle = 1,      // 각도 초기값 (10~120)
    .repeat = 10,     // 반복 횟수 초기값 (1~100)
    .delay = 1
  },
  .passive = {
    .mode = PASSIVE_ASSIST,
    .torque_level = 10
  }
};

float calibration_factor = 67000.0f;   // HX711.c와 동일한 값으로 설정
float known_weight = 4000.0f;             // 4000g(그램 단위)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart6, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



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
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  // hmi
  HAL_UART_Receive_IT(&huart2, &rxData, 1);

  // encoder
  Encoder_SetTimHandle(&htim3);
  Encoder_Start();
  HAL_Delay(500);
  // 로드셀
  HX711_Init();
  HAL_Delay(500);  // 안정화 대기

  //motor init
  RS485_MotorInit(&huart1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // 타이머 카운터 업데이트
    timer_counter = HAL_GetTick();

    
//-------------------------------------------------------------------------//
    switch (cpm_params.mode)
    {
      case CPM_IDLE:
        // IDLE 상태에서는 아무 동작도 하지 않음S
        break;
//-------------------------------------------------------------------------//
        //엔코더 값 범위 0 ~ 55 사용 소수점까지
      case CPM_ACTIVE:
        if (cpm_params.state == CPM_STATE_RUNNING)
        {
          char command[10];
          float current_angle = Encoder_GetAngle();
          uint32_t delay = cpm_params.active.delay * 1000;

          // 엔코더 각도 범위 처리 (0~360도)
          if (current_angle > 270 && current_angle <= 360)
            {current_angle = 0.0f;}
          else if (current_angle > 150.0f && current_angle < 270)
            {current_angle = cpm_params.active.angle;}

          // HMI로부터 파라미터가 업데이트된 경우 새 동작을 시작
            if (params_updated) {
              // 모터 정지 후 새 파라미터로 재시작
              RS485_SendCommand("ST");    // 모터 정지
              HAL_Delay(200);

              // 초기화 플래그 리셋하여 새 파라미터로 초기화
              cpm_initialized = false;
              params_updated = false;  // 업데이트 플래그 초기화
            }

          // 초기 설정이 안 된 경우 초기화
          if (!cpm_initialized) {
            // 모터 초기화
            RS485_SendCommand("ME");    // 모터 활성화
            HAL_Delay(300);             // 모터 활성화 대기
            // 기본 파라미터 설정
            RS485_SendCommand("JA100");  // 가속도
            RS485_SendCommand("JL100");  // 감속도
            RS485_SendCommand("JS0.01");   // 저크
            RS485_SendCommand("CJ");    // 조그 모드 설정 적용
            // 반복 카운터 초기화
            repeat_count = cpm_params.active.repeat;
            cpm_initialized = true;

            // 초기 방향 설정 (전진)
            sprintf(command, "CS%d", cpm_params.active.speed);
            RS485_SendCommand(command);
            moving_forward = true;
          }

          // 방향 및 제한 확인
          if (moving_forward && (current_angle >= cpm_params.active.angle)) {
            // 전진 한계 도달, 방향 전환
            RS485_SendCommand("ST");
            HAL_Delay(delay);  // 정수 값 사용
            RS485_SendCommand("CJ");    // 조그 모드 설정 적용
            sprintf(command, "CS-%d", cpm_params.active.speed);
            RS485_SendCommand(command);
            moving_forward = false;
          }
          else if (!moving_forward && current_angle <= 0.0f) {
            // 시작 위치에 도달, 한 사이클 완료
            repeat_count--;

            if (repeat_count <= 0) {
              // 모든 반복 완료
              RS485_SendCommand("ST");  // 정지
              //cpm_params.state = CPM_STATE_IDLE;

              cpm_initialized = false;
            } else {
              // 다음 사이클 시작
              RS485_SendCommand("ST");
              HAL_Delay(delay);
              RS485_SendCommand("CJ");    // 조그 모드 설정 적용
              sprintf(command, "CS%d", cpm_params.active.speed);
              RS485_SendCommand(command);
              moving_forward = true;
            }
          }

          // 루프 간 딜레이
          HAL_Delay(20);
        }
        else if(cpm_params.state == CPM_STATE_PAUSED)
        {
          RS485_SendCommand("ST");
        }
        else if(cpm_params.state == CPM_STATE_STOP)
        {
          RS485_SendCommand("ST");
          RS485_SendCommand("MD");
        }
        else if(cpm_params.state == CPM_STATE_EMERGENCY_STOP)
        {
          RS485_SendCommand("ST");
          RS485_SendCommand("MD");
        }
        else if(cpm_params.state == CPM_STATE_IDLE)
        {
          RS485_SendCommand("ST");
        }
        break;
//-----------------------------------------------------------------------------------//
      case CPM_PASSIVE:
        if (cpm_params.state == CPM_STATE_RUNNING)
        {
          static uint32_t passive_loadcell_tick = 0;
          static bool cs_command_sent_CW = false;    // CS 명령 전송 상태 추적 플래그
          static bool cs_command_sent_CCW = false;   // 반대 방향용 플래그
          static bool st_command_sent = false;       // ST 명령 전송 상태 추적 플래그 추가

          if(timer_counter - passive_loadcell_tick >= 50)
          {
            float weight = HX711_GetGrams();
            printf("%d\r\n",(int)weight);

            // 초기화 체크
            if (!cpm_initialized)
            {
              // 모터 초기화 코드
//              RS485_SendCommand("ME");
//              RS485_SendCommand("JA100");
//              RS485_SendCommand("JD100");
//              RS485_SendCommand("JS5");
//              RS485_SendCommand("CJ");

              RS485_SendCommand("ME");


              cpm_initialized = true;
            }

            // 데드존 체크 (-500g ~ +500g 무시)
            if (abs(weight) > 1000)
            {
              // 데드존을 벗어났으므로 ST 명령 플래그 초기화
              st_command_sent = false;

              uint8_t limited_torque = (cpm_params.passive.torque_level > MAX_TORQUE_LEVEL) ?
                                      MAX_TORQUE_LEVEL : cpm_params.passive.torque_level;
              char torque_cmd[10];

              switch(cpm_params.passive.mode)
              {
              case PASSIVE_ASSIST:  // 서포트 모드
                if (weight > 1000)  // 미는 힘
                {
                  if (!cs_command_sent_CW)  // 아직 명령이 전송되지 않았을 때만 실행
                  {
                      RS485_SendCommand("CJ");
                      sprintf(torque_cmd, "CS-%d", limited_torque);
//                      sprintf(torque_cmd, "GC-%d", limited_torque);
                      RS485_SendCommand(torque_cmd);  // 양의 토크로 도와줌
                      cs_command_sent_CW = true;      // 명령 전송 플래그 설정
                      cs_command_sent_CCW = false;    // 반대 방향 초기화
                  }
                }
                else if (weight < -1000)  // 당기는 힘
                {
                  if (!cs_command_sent_CCW)  // 아직 명령이 전송되지 않았을 때만 실행
                  {
                     RS485_SendCommand("CJ");
                      sprintf(torque_cmd, "CS%d", limited_torque);
//                      sprintf(torque_cmd, "GC%d", limited_torque);
                      RS485_SendCommand(torque_cmd);  // 음의 토크로 도와줌
                      cs_command_sent_CCW = true;     // 명령 전송 플래그 설정
                      cs_command_sent_CW = false;     // 반대 방향 초기화
                  }
                }
                break;
              }
            }


            else  // 데드존 내의 힘 (500 이하)
            {
              // ST 명령이 아직 전송되지 않았을 때만 실행
              if (!st_command_sent)
              {
                RS485_SendCommand("ST");  // 토크 0 (한 번만 실행)
                RS485_SendCommand("ST");  // 토크 0 (한 번만 실행)
//                RS485_SendCommand("GC0");
                st_command_sent = true;   // ST 명령 전송 플래그 설정

                // CS 명령 플래그 초기화 - 다시 500 이상이 되면 명령을 보낼 수 있게 함
                cs_command_sent_CW = false;
                cs_command_sent_CCW = false;
              }
            }

            passive_loadcell_tick = timer_counter;
          }

        }

        else if(cpm_params.state == CPM_STATE_PAUSED)
        {
          RS485_SendCommand("ST");


        }
        else if(cpm_params.state == CPM_STATE_STOP)
        {
          RS485_SendCommand("ST");

        }
        else if(cpm_params.state == CPM_STATE_EMERGENCY_STOP)
        {
          RS485_SendCommand("ST");
          RS485_SendCommand("MD");

        }
        break;
 //-------------------------------------------------------------------------//
      case CPM_FORCE_MEASURE:
        // 힘 측정 모드 구현
        break;
    }

    /* USER CODE END WHILE */

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  //dwin hmi lcd
  if (huart->Instance == USART2)
  {
    switch(dataIndex)
    {
      case 0:  // 첫 바이트
        if(rxData == START_BYTE1)
        {
          rxBuffer[dataIndex++] = rxData;
        }
        break;

      case 1:  // 두번째 바이트
        if(rxData == START_BYTE2)
        {
          rxBuffer[dataIndex++] = rxData;
        }
        else
        {
          dataIndex = 0;
        }
        break;

      default:  // 나머지 데이터
        rxBuffer[dataIndex++] = rxData;
        if(dataIndex >= DATA_LENGTH)
        {
          protocolData.startByte1 = rxBuffer[0];
          protocolData.startByte2 = rxBuffer[1];
          protocolData.dataLength = rxBuffer[2];
          protocolData.command = rxBuffer[3];
          protocolData.objNum1 = rxBuffer[4];
          protocolData.objNum2 = rxBuffer[5];
          protocolData.dataByte = rxBuffer[6];
          protocolData.data1 = rxBuffer[7];
          protocolData.data2 = rxBuffer[8];

          // objNum1과 objNum2를 조합하여 파라미터 식별
          uint16_t param_id = (protocolData.objNum1 << 8) | protocolData.objNum2;

          // 파라미터 업데이트
          switch(param_id)
          {
            case 0x1000:  // 속도 (1-10)
              cpm_params.active.speed = protocolData.data2;
              // 실행 중에는 값만 저장하고 업데이트 플래그는 설정하지 않음
              break;

            case 0x1001:  // 각도 (1-10)
              cpm_params.active.angle = protocolData.data2 * 5.5;
              // 실행 중에는 값만 저장하고 업데이트 플래그는 설정하지 않음
              break;

            case 0x1002:  // 반복 (10-100)
              cpm_params.active.repeat = protocolData.data2;
              // 실행 중에는 값만 저장하고 업데이트 플래그는 설정하지 않음
              break;

            case 0x1003:  // 딜레이 (1-10)
              cpm_params.active.delay = protocolData.data2;
              // 실행 중에는 값만 저장하고 업데이트 플래그는 설정하지 않음
              break;

            case 0x1100:
              // 패시브 모드 토크 레벨 설정
              cpm_params.passive.torque_level = 10 + ((protocolData.data2 - 1) * 2);
              break;

            case 0x3000:
              // 상호작용 버튼
              switch(protocolData.data2)
              {
                case 0x01:  // 시작
                  cpm_params.state = CPM_STATE_RUNNING;
                  params_updated = true;  // 시작 버튼 누를 때만 파라미터 적용
                  break;

                case 0x02:  // 일시정지
                  cpm_params.state = CPM_STATE_PAUSED;
                  break;

                case 0x03:  // 정지
                  cpm_params.state = CPM_STATE_STOP;
                  stop = false;
                  break;

                case 0x04:  // 0점(초기화)
                  cpm_params.state = CPM_STATE_IDLE;
                  break;

                case 0x05:  // 비상정지
                  cpm_params.state = CPM_STATE_EMERGENCY_STOP;
                  break;
              }
              break;
              
            case 0x3100:
              // 상호작용 버튼
              switch(protocolData.data2)
              {
                case 0x01:  // 시작
                  cpm_params.state = CPM_STATE_RUNNING;
                  params_updated = true;  // 시작 버튼 누를 때만 파라미터 적용
                  break;

                case 0x02:  // 일시정지
                  cpm_params.state = CPM_STATE_PAUSED;
                  break;

                case 0x03:  // 정지
                  cpm_params.state = CPM_STATE_STOP;

                  break;

                case 0x04:  // 0점(초기화)
                  cpm_params.state = CPM_STATE_IDLE;
                  break;

                case 0x05:  // 비상정지
                  cpm_params.state = CPM_STATE_EMERGENCY_STOP;
                  break;
                  
                case 0x06:  // 서포트모드
                  cpm_params.mode = CPM_PASSIVE;
                  cpm_params.passive.mode = PASSIVE_ASSIST;
                  break;
                  
                case 0x07:  // 저항모드 not used
                  cpm_params.mode = CPM_PASSIVE;
                  cpm_params.passive.mode = PASSIVE_RESIST;
                  break;
                  
                case 0x08:  // 비례모드
                  cpm_params.mode = CPM_PASSIVE;
                  cpm_params.passive.mode = PASSIVE_PROPORTIONAL;
                  break;
              }
              break;

            case 0x4000:  // 모드 선택
              switch(protocolData.data2)
              {
                case 0x01:  // 액티브 모드
                  cpm_params.mode = CPM_ACTIVE;
                  cpm_initialized = false;
                  break;

                case 0x02:  // 패시브 모드
                  cpm_params.mode = CPM_PASSIVE;
                  cpm_initialized = false;
                  break;

                case 0x03:  // 힘측정 모드
                  cpm_params.mode = CPM_FORCE_MEASURE;
                  break;
              }
              break;
          }

          dataIndex = 0;
        }
        break;
    }
    HAL_UART_Receive_IT(huart, &rxData, 1);
  }
}

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
