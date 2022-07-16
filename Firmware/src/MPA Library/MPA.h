/**
 *  Biblioteca principal do MPA, onde são incluidas as outras e executadas codigos
 * basicos como setup, definição de callback, etc.
 *
 */

#ifndef _MPA // Para evitar multipla inclusão do mesmo codigo
#define _MPA

//Bibliotecas de Multitasking
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Arduino.h>
#include "esp_ipc.h"



//Bibliotecas MPA
#include "MPA Library\MPA_CONFIG.h"   //Inclui a HAL de Inputs
#include "MPA Library\MPA_inputs.h" //Monitoramento de Inputs via Multitask
#include "MPA Library\MPA_Virloc.h" //Inclui comunicação APENAS na Serial do VIRLOC (padrao RX2 TX2)

//#include "pitches.h"
bool Serial2InUse = false;

TaskHandle_t Task1_UpdateInputs;
// portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
void Task1_UpdateInputs_code(void *pvParameters)
{
  char buf[30];
  snprintf(buf, sizeof(buf), " running on core %d", xPortGetCoreID());
  Debugln(buf, Debug_MPA, __func__, __LINE__);
  for (;;)
  {
    // unsigned long call_millis = millis();
    //   portENTER_CRITICAL(&myMutex);
    MPA_inputs_UPDATE();
    //  portEXIT_CRITICAL(&myMutex);
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

TaskHandle_t Task2_TransceiveSerial2; // O Handle teve que ser definido no MPA_Virloc.h para que funcionasse, ta ruim e precisa melhorar.
// portMUX_TYPE myMutex2 = portMUX_INITIALIZER_UNLOCKED;
void Task2_TransceiveSerial2_code(void *pvParameters)
{

  for (;;)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
    // bool message_completed = false;
    // bool TIMEOUT_EXPIRED = false;
    // int timeout = 0;
    // if (Serial2InUse == false) // Evita que transcreva quando alguma função estiver usando
    // {
    //   String inData;
    //   if (Serial2.available() > 0)
    //   {
    //     // portENTER_CRITICAL(&myMutex2);
    //     while (!(message_completed == true || TIMEOUT_EXPIRED == true)) // Check if there is something in the serial buffer
    //     {
    //       char recieved;
    //       if (Serial2.available() > 0)
    //       {
    //         recieved = Serial2.read();
    //         inData += recieved;
    //         if (recieved == '\n')
    //         {
    //           message_completed = true;
    //         }
    //       }

    //       else if (timeout > 100)
    //       {
    //         TIMEOUT_EXPIRED = true;
    //         Serial.println("\033[91mTransceive ERRO TIMEOUT\033[0m");
    //       }
    //       else
    //       {
    //         timeout++;
    //         vTaskDelay(1 / portTICK_PERIOD_MS); // espera 1ms pra fazer a proxima leitura se o buffer não tiver nada ainda
    //       }
    //     }
    //     // portEXIT_CRITICAL(&myMutex2);
    //     Serial.print("RX2 TSK: ");
    //     inData.replace("\r", "");
    //     inData.replace("\n", "");
    //     Serial.println(inData); // Ja tem /n
    //   }
    //   vTaskDelay(pdMS_TO_TICKS(1000));
    // }
    // else
    // {
    //   vTaskDelay(pdMS_TO_TICKS(1000));
    // }
    
  }
}

void MPA_Setup_BT()
{
  char btname[15];
  // Tenta obter MARTE_ID, se não der seta o nome com o ID do chip.
  if (MPA_find_id(MARTE_ID_ASCII))
  {
    snprintf(btname, sizeof(btname), "MPA_%s", MARTE_ID_ASCII);
    SerialBT.begin(btname); // Bluetooth device name
  }
  else
  {
    snprintf(btname, sizeof(btname), "MPA_NoID_%04d", getchipID());
    SerialBT.begin(btname); // Bluetooth device name
  }
  SerialBT.onData(callback_ESP_BT);
}

void MPA_Setup_pins()
{
  // Pinos configurados em pins.h
  pinMode(input1_pin, INPUT);
  pinMode(input2_pin, INPUT);
  pinMode(input3_pin, INPUT);
  pinMode(input4_pin, INPUT);
  pinMode(input5_pin, INPUT);
  pinMode(input6_pin, INPUT);
  pinMode(input7_pin, INPUT);
  pinMode(input8_pin, INPUT);

  pinMode(output1_pin, OUTPUT);
  pinMode(output2_pin, OUTPUT);
  pinMode(output3_pin, OUTPUT);

  ledcSetup(channel_buzzer, 2000, 10);
  ledcAttachPin(4, channel_buzzer);
  // pinMode(buzzer_pin, OUTPUT);
  pinMode(2, OUTPUT); // pino do led interno

  // O VL8 lê GND como sinal,  então pra nao ativar sem querer as saidas do esp precisam estar em HIGH.
  digitalWrite(output1_pin, HIGH);
  digitalWrite(output2_pin, HIGH);
  digitalWrite(output3_pin, HIGH);
}

void MPA_Serial_SETUP()
{
  Serial.begin(115200);
  delay(20); //aguarda o serial inicar antes de continuar
}

bool MPA_Virloc_SETUP()
{
  Serial2.begin(MARTE_BAUD);
  return 1;
}

void MPA_Setup()
{
  MPA_Setup_pins();
  //MPA_Serial_SETUP();
  MPA_Virloc_SETUP();
  MPA_Setup_BT();


  xTaskCreatePinnedToCore(
      Task1_UpdateInputs_code, /* Task function. */
      "Task1",                 /* name of task. */
      10000,                   /* Stack size of task */
      NULL,                    /* parameter of the task */
      1,                       /* priority of the task */
      &Task1_UpdateInputs,     /* Task handle to keep track of created task */
      0);                      /* pin task to core 0 */

  // xTaskCreatePinnedToCore(
  //     Task2_TransceiveSerial2_code, /* Task function. */
  //     "Task2",                      /* name of task. */
  //     10000,                        /* Stack size of task */
  //     NULL,                         /* parameter of the task */
  //     1,                            /* priority of the task */
  //     &Task2_TransceiveSerial2,     /* Task handle to keep track of created task */
  //     0);                           /* pin task to core 0 */
  //play_bootOK();
  esp_ipc_call(0, &buzzer_boot,(void*)0);
    //Debug("Boot OK", Debug_MPA, __func__, __LINE__);
    //Smart_Print("MPA_BOOT Finalizado")
}

#endif