/** CODIGO PARA FILIPE
 * @file main.h
 * @brief main.cpp. Codigo base para uso da placa SimovaMPA.
 */

//=========================== CONFIGURAÇÕES DO MPA =============================
#include <Arduino.h>
#include "BluetoothSerial.h" //Inclusão da Lib do bluetooth
BluetoothSerial SerialBT;    // Criando objeto do Serial

//=========================== Inclusão de bibliotecas =========================

int getchipID()
{
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  char buf[20];
  snprintf(buf, sizeof(buf), "CHIP_ID=%d", chipId);
  // Debug(buf, Debug_MPA, __func__, __LINE__);
  Serial.println(buf);
  return chipId;
}

/* #region  Callback BT */
char bt_serial_temporario[150];
// Processamento da entrada via bluetooth - em desenvolvimento
void callback_ESP_BT(const uint8_t *buffer, size_t size) // Funçao construida mas não desenvolvida nesse programa.  Processa a entrada
{
  String inData;
  for (size_t i = 0; i < size; i++) // loop para colocar todo o buffer numa array
  {
    Serial2.write(buffer[i]);
//    const char c = buffer[i];
//    bt_serial_temporario[i] = c;
//    inData += c;
   // Serial.write(c);
  }
  //Serial.print(inData);

  // if (inData == "restart\n")
  // {
  //     // O ESP.restart misteriosamente roda antes do bluetooth enviar, bizarro demais
  //     ESP.restart();
  // }
  // else if (inData == "clean_vars\n")
  // {
  //     SerialBT.println("Not working yet CLEANVARS");
  // }
  // else if (inData == "report\n")
  // {
  //     SerialBT.println("Not working yet REPORT");
  // }
  // else if (inData == "setBTname\n")
  // {
  //     SerialBT.println("Not working yet SETBTNAME");
  // }
  // else
  // {
  //     SerialBT.println("unknown command");
  // }

  // inData = ""; // Clear recieved buffer
  //  ELSE
}
/* #endregion */

/* Rotina de setup, roda apenas uma vez */
// void setup(void)
// {
//   Serial.begin(9600);
//   char btname[15];
//   snprintf(btname, sizeof(btname), "xBTSAT_%4d", getchipID());
//   SerialBT.begin(btname); // Bluetooth device name
//   SerialBT.onData(callback_ESP_BT);
// }

TaskHandle_t Task1_handle; // Esse handle não funciona se estiver em outro contexto, utilizei a variavel booleana "Serial2InUse"
// portMUX_TYPE myMutex2 = portMUX_INITIALIZER_UNLOCKED;
void Task1_LED(void *pvParameters)
{
  Serial.print("Task3 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;)
  {
     if ((SerialBT.hasClient() > 0))
    {
        digitalWrite(23,LOW);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        digitalWrite(23,HIGH);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        digitalWrite(23,LOW);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        digitalWrite(23,HIGH);
        vTaskDelay(1600 / portTICK_PERIOD_MS);
    }
    else //nao conectado
    {
        digitalWrite(23,LOW);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        digitalWrite(23,HIGH);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    //vTaskDelay(30000 / portTICK_PERIOD_MS); // consulta mais rapido para nao perder cortes
  }
}

void setup(void)
{
  //Status LED
  pinMode(23,OUTPUT);
  digitalWrite(23,LOW);
  Serial.begin(9600);
  Serial2.begin(9600);
  char btname[15];
  snprintf(btname, sizeof(btname), "MARTE_%4d", getchipID());
  SerialBT.begin(btname); // Bluetooth device name
  SerialBT.onData(callback_ESP_BT);


  xTaskCreatePinnedToCore(
      Task1_LED, /* Task function. */
      "Task3",                    /* name of task. */
      10000,                      /* Stack size of task */
      NULL,                       /* parameter of the task */
      1,                          /* priority of the task */
      &Task1_handle,     /* Task handle to keep track of created task */
      1);                         /* pin task to core 0 */

}


void transceive()
{
  // ESTA NO CALLBACK
  //   if (SerialBT.available())
  //   Serial.write(SerialBT.read());
  // from termial to bluetooth
  if (Serial2.available())
    SerialBT.write(Serial2.read());
}

void loop(void) // sempre será atribuído ao core 1 automaticamente pelo sistema, com prioridade 1
{
  transceive();
  // delay(1);
}



// while (Serial.available())
// {
//   SerialBT.print(Serial.read());
//   /* code */
// }
