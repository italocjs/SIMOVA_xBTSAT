/** @file main.h
 * 
 * @brief main.cpp. Codigo para FELLER MS,  Placa SimovaMPA
 * @par       
 * COPYRIGHT NOTICE: (c) 2021 Italo C J Soares / SIMOVA.  All rights reserved.
 */


/*
Melhorias a fazer no codigo:
1 - Criar LIB dos protocolos MARTE e e codigo base do MPA.
2 - Melhorar logica da comunicação bluetooth para nao interferir no processamento (CPU1?)
3 - Adicionar comunicação/resposta por bluetooth (ex: pergunta total e responde algo)
4 - Implementar watchdog
5 - Padronizar variaveis de alguma forma.


*/


//Configuração do MARTE
#define MARTE_ID 107
int MARTE_MSG_NUM = 0;
int STATUS_Flag_Motor_Ligado = 0 ;
int vezes_pressionado_fecha_garra_menor = 0;
#define MARTE_BAUD 19200 //VL6 19200 ; VL8 57600
//#define MARTE_BAUD 57600 //VL6 19200 ; VL8 57600
#define FLAG_Motor_Ligado 2
#define CT_Eucaliptos 3
#define Flag_TTS_Eucalipto_Cortado 10
bool processando_serial2 = false;
#define debug_BT false

//VARIAVEIS FELLER
int Estado_GarraMenor = -1;
int Estado_GarraMaior = -1;
int cortes_na_sessao = 0;
int cortes_invalidos_na_sessao = 0;
void add_CT(int CT_NUMBER);
/* #region  =====================================  CONFIG DO PROGRAMA                  =====================================*/

//#define usarPCF  //Usar o módulo PCF8574 como expansor de portas,  ativa o "setup_pcf e loop_pcf"
//#define NOMEDOPROGRAMA

/* #endregion */

/* #region  =====================================  INCLUSÃO DE BIBLIOTECAS E VARIAVEIS =====================================*/

#include <Arduino.h> //Biblioteca do Arduino, contém inumeras funções úteis
#include <pins.h>    //Definição de pinos usados.
#include <Ferramentas.h>

// ##### Configuração do programa
//#define debug //Se ativado, printa mais informações para debug
#define fastboot                   //se ativado utiliza um delay menor no boot (só para testes)
#define marte_output_duration 2000 //Duração do sinal (LOW) para o MARTE
// #define max_timeout_1 15000        //Tempo maximo para apertar auto ou manual              (trigger inicial é o sinal da garra)
// #define max_timeout_2 15000        //Tempo maximo para finalizar manual                    (apertar o botão corte e descer o cabeçote)
// #define max_timeout_3 35000        //Tempo maximo para finalizar automatico                (apertar o botão AUTO e subir o cabeçote)
// #define min_timeout_3 7000         //Tempo minimo para considerar um corte valido no AUTO  (menos que isso pode ser corte no ar)
#define millis_print_vertical 1000 //Tempo minimo para printar se foi pressionado sobe/desce cabeçote (evita spam)

// ##### Declaração de variaveis #####
char buf[100];                              //Variavel global para preparar textos pequenos com o snprintf.  Nao deve ser usada com dados sensiveis
char bt_serial_temporario[50];              //variavel para guardar o que tiver no bluetooth
int input1, input2, input3, input4, input5; //Variavel para guardar o ultimo estado da leitura dos pinos
//int navertical = -1;                        //Variavel para definir estado em pé, deitado ou desconhecido (por isso int)
uint32_t chipId = 0; //Variavel para guardar a UUID do chip
char btname[30];     //Variavel para criar o nome do bluetooth baseado na UUID do chip
//int cortes_na_sessao = 0;                   //Soma a quantidade de cortes validos na sessão (para referencia)
//int cortes_auto_na_sessao = 0;              //Soma a quantidade de cortes automaticos       (para referencia)
//int cortes_manual_na_sessao = 0;            //Soma a quantidade de cortes manuais           (para referencia)

//int timeout1;          //Contador de tempo para seleção Manual/AUTO
//int timeout2;          //Contador de tempo para finalizar Manual
//int timeout3;          //Contador de tempo para finalizar Auto

// ##### Biblioteca Bluetooth #####
#include "BluetoothSerial.h" //Inclusão da Lib do bluetooth
BluetoothSerial SerialBT;    //Criando objeto do Serial

// ##### Biblioteca do display I2C 128x64 #####
#include "SSD1306Wire.h"                     //Inclusão da Lib do Display
SSD1306Wire display(0x3c, pin_SDA, pin_SCL); //pin_SDA e pin_SCL são setados em pins.h

// ##### Lib do expansor de portas #####
#ifdef usarPCF
#include "PCF8574.h"
PCF8574 pcf8574(0x20); // Set i2c address
#define P0 0
#define P1 1
#define P2 2
#define P3 3
#define P4 4
#define P5 5
#define P6 6
#define P7 7
#endif
/* #endregion */

/* #region  =====================================  MULTITASKING - FREERTOS TASKS       =====================================*/
//Core 1 -  = Loop  |   //Core 0 = Livre

TaskHandle_t Task1_UpdateInputs;
TaskHandle_t Task2;
TaskHandle_t Task3_Consulta_Motor;

/* #region ##### TASK 01 - Leitura dos pinos de entrada #####  */

//Cria variaveis para guardar os estados atuais e passados da porta.
int current_input_state[9];
int last_input_state[9];
int input_pin_index[9] = {0, input1_pin, input2_pin, input3_pin, input4_pin, input5_pin, input6_pin, input7_pin, input8_pin};

int output_state[4];
int output_pin_index[4] = {0, output1_pin, output2_pin, output3_pin};

//#define PRINT_INPUTS_TASK01

unsigned long last_print_info = 0;
int poluicao_task1 = 0;
void Task1_UpdateInputs_code(void *pvParameters)
{

  Serial.print("Task1_UpdateInputs running on core ");
  Serial.println(xPortGetCoreID());

  for (;;)
  {
    unsigned long call_millis = millis();

    //Atualizar o LAST STATE
    for (int i = 1; i < (sizeof(last_input_state) / sizeof(last_input_state[0])); i++)
    {
      last_input_state[i] = current_input_state[i];
    }

    //Atualizar o CURRENT STATE
    for (int i = 1; i < (sizeof(current_input_state) / sizeof(current_input_state[0])); i++)
    {
      current_input_state[i] = digitalRead(input_pin_index[i]);
    }

    //Atualizar estado da saida (só para informacao)
    for (int i = 1; i < (sizeof(output_state) / sizeof(output_state[0])); i++)
    {
      output_state[i] = digitalRead(output_pin_index[i]);
    }

    //CODIGO ESPECIFICO DA FELLER MS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    // input1 = current_input_state[1]; //Abre garra de CIMA/PEQUENA
    // input2 = current_input_state[2]; //Fecha garra de CIMA/PEQUENA
    // input3 = current_input_state[3]; //Abre garra de BAIXO/GRANDE
    // input4 = current_input_state[4]; //fecha garra de BAIXO/GRANDE
    // input5 = current_input_state[5]; //Não usada

    //Processamento de estado da garra de CIMA/pequena
    if (current_input_state[1] == 1 && current_input_state[2] == 0)
    {
      Estado_GarraMenor = 1;
      if (call_millis - last_print_info >= millis_print_vertical)
      {
        if (debug_BT)SerialBT.println("PRESIONADO: Abrir garra de CIMA (INPUT1)");
        last_print_info = call_millis;
      }
    }
    else if (current_input_state[1] == 0 && current_input_state[2] == 1) //Apertou fechar garra MENOR
    {
      Estado_GarraMenor = 0;
      if (call_millis - last_print_info >= millis_print_vertical)
      {
        if (debug_BT) SerialBT.println("PRESIONADO: Fechar garra de CIMA (INPUT2)");
        last_print_info = call_millis;
        if (current_input_state[2] == 1 && last_input_state[2] == 0)
        {
          vezes_pressionado_fecha_garra_menor++;
          if (debug_BT) SerialBT.println("+1 no contador CT20");
          add_CT(20);/* code */ 
        }
        
      }
    }

    //Processamento do estado da garra de baixo
    if (current_input_state[3] == 1 && current_input_state[4] == 0)
    {
      Estado_GarraMaior = 1;
      if (call_millis - last_print_info >= millis_print_vertical)
      {
        if (debug_BT) SerialBT.println("PRESIONADO: Abrir garra de BAIXO (INPUT3)");
        Estado_GarraMaior = 1;
        last_print_info = call_millis;
      }
    }
    else if (current_input_state[3] == 0 && current_input_state[4] == 1) //Apertou fechar garra MAIOR
    {
      Estado_GarraMaior = 0;
      if (call_millis - last_print_info >= millis_print_vertical)
      {
        if (debug_BT) SerialBT.println("PRESIONADO: Fechar garra de BAIXO (INPUT4)");
        last_print_info = call_millis;
      }
    }
    //#define debug_inputs
    #ifdef debug_inputs
    poluicao_task1 += 10;
    if (poluicao_task1 > 1000)
    {
      print_pin_states();
      poluicao_task1 = 0;
    }
    #endif

    vTaskDelay(1);
  }
}

/* #endregion */

/* #region ##### TASK 02 - Blink do led interno (i'm alive yo) #####  */
void Task2code(void *pvParameters)
{
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;)
  {
    digitalWrite(2, !digitalRead(2)); //pisca o led do esp
    vTaskDelay(500);
  }
}
/* #endregion */


/* #region ##### TASK 03 - Consulta de motor ligado #####  */
int check_flag(int FLAG_ID);
void Task3code(void *pvParameters)
{
  Serial.print("Task3 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;)
  {
    //digitalWrite(2, !digitalRead(2)); //pisca o led do esp
    STATUS_Flag_Motor_Ligado = check_flag(FLAG_Motor_Ligado);
    if (STATUS_Flag_Motor_Ligado == 1)
    {
      vTaskDelay(60000); //consulta em periodos maiores
    }
    else
    {
      vTaskDelay(1000); //consulta mais rapido para nao perder cortes
    }
  }
}
/* #endregion */

/* #endregion */

/* #region  =====================================  FUNCOES DE SETUP E INTERRUPTS       =====================================*/

//Função para mapear uma variavel float (regra de tres), a função padrão do arduino só suporta tipo INT
float mapfloat(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}


void callback_ESP_BT_data(const uint8_t *buffer, size_t size) //Funçao construida mas não desenvolvida nesse programa.  Processa a entrada
{
  String inData;
  while (SerialBT.available() > 0) //Check if there is something in the serial buffer
    {
        char recieved = SerialBT.read();
        inData += recieved;
        if (recieved == '\n') // Add data into the "inData" until a \n is received
        {
            Serial.print("Arduino Received: ");
            Serial.print(inData);
            if (inData == "total\n")
            {
                char buf[100];
                snprintf(buf, sizeof(buf), "Corte valido. TOTAL = %d | Invalidos %d", cortes_na_sessao, cortes_invalidos_na_sessao);
                SerialBT.println(buf);
                //deleteFile(SD, "/LOG.txt");
            }
            else if (inData == "delete train\n")
            {
                Serial.println("TRAIN.txt deleted.");
                //deleteFile(SD, "/TRAIN.txt");
            }            
            else if (inData == "dump log\n")
            {
                Serial.println("Dumping LOG.txt");
                //readFile(SD, "/LOG.txt");
            }
            else if (inData == "dump train\n")
            {
                Serial.println("Dumping TRAIN.txt");
                //readFile(SD, "/TRAIN.txt");
            }            
            else if (inData == "reboot\n")
            {
                //throwerror("Rebooting - User requested");
            }

            else if (inData == "s\n")
            {
                //sabre = 1;
                //ESP_BT.println("SABRE ATUADO");
            }

            else
            {
                Serial.println("unknown command");
            }

            inData = ""; // Clear recieved buffer
            //ELSE
        }
    }
}

// void callback_ESP_BT_data(const uint8_t *buffer, size_t size) //Funçao construida mas não desenvolvida nesse programa.  Processa a entrada
// {
//   for (size_t i = 0; i < size; i++) //loop para colocar todo o buffer numa array
//   {
//     const char c = buffer[i];
//     bt_serial_temporario[i] = c;
//   }
//   //Serial.print(bt_serial_temp); //Se quiser debugar a entrada crua

//   //"MPU,%0.2f,%0.2f,%0.2f,%0.2f\n"
//   char *tipo = strtok(bt_serial_temporario, ",;"); //ainda nao usado, mas no futuro pode ser util pra separar diversos tipos de dados
//   if (strcmp(tipo, "MPU") == 0)
//   {
//   }
//   else
//   {
//     SerialBT.println("Unknown input");
//   }
// }

void getchipID() //Função para obter o UUID do chip e setar o nome do bluetooth com ele, evita nomes duplicados
{
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
#ifdef debug
  SerialBT.printf("ESP32 Chip model = %s Rev %d", ESP.getChipModel(), ESP.getChipRevision());
  SerialBT.printf("| This chip has %d cores  ", ESP.getChipCores());
  SerialBT.print(" | Chip ID:");
  SerialBT.println(chipId);
#endif
}

void setup_lcd() // Inicialização do LCD
{
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.display();
  display.clear();
  display.setColor(WHITE);
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 0, "Contador Harvester");
  display.drawString(64, 15, "Iniciando modulo");
  display.drawString(64, 32, btname);
#ifndef fastboot                                     //se estiver ligado, pula o contador para fazer boot mais rapido, o contador é so um tempo para dar para ler o nome BT
  for (size_t counter = 0; counter < 100; counter++) //Loop para "crescer" o contador.
  {
    display.drawProgressBar(0, 52, 120, 10, counter);
    display.display();
    delay(100);
  }
#endif
}

void start_bluetooth()
{
  snprintf(btname, sizeof(btname), "SimovaMPA_%04d", MARTE_ID);
  SerialBT.begin(btname); //Bluetooth device name
#ifdef debug
  Serial.println("Conexao Bluetooth ativada, ja e possivel parear!");
  SerialBT.println("Conexao Bluetooth ativada, ja e possivel parear!");
#endif
}

void setup_pins()
{
  //Pinos configurados em pins.h
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

  pinMode(2, OUTPUT); //pino do led interno

  //O VL8 lê GND como sinal,  então pra nao ativar sem querer as saidas do esp precisam estar em HIGH.
  digitalWrite(output1_pin, HIGH);
  digitalWrite(output2_pin, HIGH);
  digitalWrite(output3_pin, HIGH);
}

unsigned long last_imalive = 0;
void i_am_alive(int waittime)
{
  unsigned long call_millis = millis();
  //SerialBT.println(call_millis - last_imalive);
  if (call_millis - last_imalive >= waittime)
  {
    SerialBT.println("I'm Alive");
    last_imalive = call_millis;
  }
}

#ifdef usarPCF
int PCF1_INPUT1 = 0;
void setup_expander()
{
  pcf8574.pinMode(P0, INPUT);
  pcf8574.pinMode(P1, INPUT);
  pcf8574.pinMode(P2, INPUT);
  pcf8574.pinMode(P3, INPUT);
  pcf8574.pinMode(P4, INPUT);
  pcf8574.pinMode(P5, INPUT);
  pcf8574.pinMode(P6, INPUT);
  pcf8574.pinMode(P7, INPUT);

  Serial.print("Init pcf8574...");
  if (pcf8574.begin())
  {
    Serial.println("PCF OK");
  }
  else
  {
    Serial.println("PCF DEU RUIM");
  }
}
#endif

void setup()
{
  setup_pins();
  Serial.begin(115200);
  Serial2.begin(MARTE_BAUD);
  getchipID();
  start_bluetooth();
  setup_lcd();
#ifdef usarPCF
  setup_expander();
#endif
  SerialBT.onData(callback_ESP_BT_data); //Função construida mas não desenvolvida, processa entradas no serial via interrupt
  SerialBT.println("Boot completo");
  Serial.println("Boot completo");

  xTaskCreatePinnedToCore(
      Task1_UpdateInputs_code, /* Task function. */
      "Task1_UpdateInputs",    /* name of task. */
      10000,                   /* Stack size of task */
      NULL,                    /* parameter of the task */
      1,                       /* priority of the task */
      &Task1_UpdateInputs,     /* Task handle to keep track of created task */
      0);                      /* pin task to core 0 */

  xTaskCreatePinnedToCore(
      Task2code, /* Task function. */
      "Task2",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task2,    /* Task handle to keep track of created task */
      1);        /* pin task to core 1 */

    
  xTaskCreatePinnedToCore(
      Task3code,              /* Task function. */
      "Task3_Consulta_Motor", /* name of task. */
      10000,                  /* Stack size of task */
      NULL,                   /* parameter of the task */
      1,                      /* priority of the task */
      &Task2,                 /* Task handle to keep track of created task */
      1);                     /* pin task to core 1 */

  delay(5000);
  STATUS_Flag_Motor_Ligado = check_flag(FLAG_Motor_Ligado);
}

/* #endregion */

/* #region  =====================================  TELAS DO LCD (comentado)            =====================================*/

// void screen_update_inputs()
// {
//   display.clear();
//   display.setColor(WHITE);
//   display.setFont(ArialMT_Plain_10);
//   display.setTextAlignment(TEXT_ALIGN_CENTER);
//   display.drawString(64, 0, "Contador cortes");
//   snprintf(buf, sizeof(buf), "QTDE: %d", cortes_na_sessao);
//   display.drawString(64, 15, buf);

//   if (navertical == 1)
//   {
//     display.drawString(64, 32, "Cabecote em PE");
//   }
//   else
//   {
//     display.drawString(64, 32, "Cabecote deitado");
//   }

//   snprintf(buf, sizeof(buf), "INPUTS: %d  %d  %d  %d  %d", input1, input2, input3, input4, input5);
//   display.drawString(64, 52, buf);
//   display.display();
// }

// void screen_corte_valido()
// {
//   display.clear();
//   display.setColor(WHITE);
//   display.setFont(ArialMT_Plain_10);
//   display.setTextAlignment(TEXT_ALIGN_CENTER);
//   display.drawString(64, 0, "CORTE VALIDO");
//   display.drawString(64, 15, "Total de cortes");
//   snprintf(buf, sizeof(buf), "%d", cortes_na_sessao);
//   display.drawString(64, 32, buf);
//   snprintf(buf, sizeof(buf), "M: %4d  A: %4d", cortes_manual_na_sessao, cortes_auto_na_sessao);
//   display.drawString(64, 50, buf);
//   display.display();
// }

/* #endregion */

/* #region  =====================================  COMMS MARTE                         =====================================*/
/*
Funções suportadas até o momento:
  Contadores:
    - Setar valor  de contador  - set_CT(int num_ct)
    - Adicionar +1 em contador  - add_CT(int num_ct)
    - Subtrair  -1 em contador  - sub_CT(int num_ct)

  Flags:
    - Inverter valor de flag    - toggle_flag_SSH(int num_flag)
    - Setar valor de flag       - set_flag(int num_flag, bool valor)
    - Consultar valor de flag   - check_flag_QSH(int num_flag), retorna int -1(erro), 0 ou 1
*/

void update_msg_num()
{
  MARTE_MSG_NUM++;
  if (MARTE_MSG_NUM >= 0xFFFF)
    MARTE_MSG_NUM = 0;
}

void set_CT(int CT_NUMBER, int VALUE)
{
  //Exemplo de MSG: TX  >SCT03 1234;ID=0071;#8003;*78<
  //Exemplo de MSG: RX  >RCT03 01234;ID=0071;#8003;*49<

  char SCT_buffer[40];
  snprintf(SCT_buffer, sizeof(SCT_buffer), ">SCT%02d %d;ID=%04d;#%04X;*", CT_NUMBER, MARTE_ID, MARTE_MSG_NUM, VALUE);

  int r;
  unsigned char chksum;
  chksum = 0;
  for (r = 0; r < strlen(SCT_buffer); r++)
  {
    if ((SCT_buffer[r] == '*') && (SCT_buffer[r - 1] == ';'))
      break;
    else
      chksum = chksum ^ SCT_buffer[r];
  }
  snprintf(SCT_buffer, sizeof(SCT_buffer), ">SCT%02d %d;ID=%04d;#%04X;*%02X<\r\n", CT_NUMBER, VALUE, MARTE_ID, MARTE_MSG_NUM, chksum);
  Serial.print(SCT_buffer);
  Serial2.print(SCT_buffer);
  update_msg_num();
}

void add_CT(int CT_NUMBER)
{
  //Exemplo de MSG: TX  >SCT03 1234;ID=0071;#8003;*78<
  //Exemplo de MSG: RX  >RCT03 01234;ID=0071;#8003;*49<

  char SCT_buffer[40];
  snprintf(SCT_buffer, sizeof(SCT_buffer), ">SCT%02d +1;ID=%04d;#%04X;*", CT_NUMBER, MARTE_ID, MARTE_MSG_NUM);

  int r;
  unsigned char chksum;
  chksum = 0;
  for (r = 0; r < strlen(SCT_buffer); r++)
  {
    if ((SCT_buffer[r] == '*') && (SCT_buffer[r - 1] == ';'))
      break;
    else
      chksum = chksum ^ SCT_buffer[r];
  }
  snprintf(SCT_buffer, sizeof(SCT_buffer), ">SCT%02d +1;ID=%04d;#%04X;*%2X<\r\n", CT_NUMBER, MARTE_ID, MARTE_MSG_NUM, chksum);
  Serial.print(SCT_buffer);
  Serial2.print(SCT_buffer);
  update_msg_num();
}

void sub_CT(int CT_NUMBER)
{
  //Exemplo de MSG: TX  >SCT03 1234;ID=0071;#8003;*78<
  //Exemplo de MSG: RX  >RCT03 01234;ID=0071;#8003;*49<

  char SCT_buffer[40];
  snprintf(SCT_buffer, sizeof(SCT_buffer), ">SCT%02d -1;ID=%04d;#%04X;*", CT_NUMBER, MARTE_ID, MARTE_MSG_NUM);

  int r;
  unsigned char chksum;
  chksum = 0;
  for (r = 0; r < strlen(SCT_buffer); r++)
  {
    if ((SCT_buffer[r] == '*') && (SCT_buffer[r - 1] == ';'))
      break;
    else
      chksum = chksum ^ SCT_buffer[r];
  }
  snprintf(SCT_buffer, sizeof(SCT_buffer), ">SCT%02d -1;ID=%04d;#%04X;*%2X<\r\n", CT_NUMBER, MARTE_ID, MARTE_MSG_NUM, chksum);
  Serial.print(SCT_buffer);
  Serial2.print(SCT_buffer);
  update_msg_num();
}

bool flag_store[100];
//função para alterar de 0 -> 1 ou 1 -> 0 o valor de uma Flag, que será usada para gerar um evento de log GP
void toggle_flag(int FLAG_ID)
{
  //Exemplo de MSG: TX  >SCT03 1234;ID=0071;#8003;*78<
  //Exemplo de MSG: RX  >RCT03 01234;ID=0071;#8003;*49<

  //Obtem o valor da flag atual na "flag store" (que armazena o ultimo estado da flag)
  bool current_flag_status = !(flag_store[FLAG_ID - 1]); //-1 pq o index começa em zero

  char SSH_FLAG[40];
  snprintf(SSH_FLAG, sizeof(SSH_FLAG), ">SSH%02d%d;ID=%04d;#%04X;*", FLAG_ID, current_flag_status, MARTE_ID, MARTE_MSG_NUM);

  int r;
  unsigned char chksum;
  chksum = 0;
  for (r = 0; r < strlen(SSH_FLAG); r++)
  {
    if ((SSH_FLAG[r] == '*') && (SSH_FLAG[r - 1] == ';'))
      break;
    else
      chksum = chksum ^ SSH_FLAG[r];
  }
  snprintf(SSH_FLAG, sizeof(SSH_FLAG), ">SSH%02d%d;ID=%04d;#%04X;*%2X<\r\n", FLAG_ID, current_flag_status, MARTE_ID, MARTE_MSG_NUM, chksum);
  Serial.print(SSH_FLAG);
  Serial2.print(SSH_FLAG);
  update_msg_num();
  flag_store[FLAG_ID - 1] = !flag_store[FLAG_ID - 1];
}

//Função para setar o valor de uma flag
void set_flag(int FLAG_ID, bool valor_flag)
{
  //Exemplo de MSG: TX  >SCT03 1234;ID=0071;#8003;*78<
  //Exemplo de MSG: RX  >RCT03 01234;ID=0071;#8003;*49<

  //Obtem o valor da flag atual na "flag store" (que armazena o ultimo estado da flag)
  char SSH_FLAG[40];
  snprintf(SSH_FLAG, sizeof(SSH_FLAG), ">SSH%02d%d;ID=%04d;#%04X;*", FLAG_ID, valor_flag, MARTE_ID, MARTE_MSG_NUM);

  int r;
  unsigned char chksum;
  chksum = 0;
  for (r = 0; r < strlen(SSH_FLAG); r++)
  {
    if ((SSH_FLAG[r] == '*') && (SSH_FLAG[r - 1] == ';'))
      break;
    else
      chksum = chksum ^ SSH_FLAG[r];
  }
  snprintf(SSH_FLAG, sizeof(SSH_FLAG), ">SSH%02d%d;ID=%04d;#%04X;*%2X<\r\n", FLAG_ID, valor_flag, MARTE_ID, MARTE_MSG_NUM, chksum);
  Serial.print(SSH_FLAG);
  Serial2.print(SSH_FLAG);
  if (debug_BT) SerialBT.print("TX: SSH_FLAG -> ");
  if (debug_BT) SerialBT.println(SSH_FLAG);
  update_msg_num();
}


//check_flag_QSH(int numero da flag), retorna o valor da flag consultada ou -1 em caso de erro
int check_flag(int FLAG_ID)
{
  //Serial.println("Cheguei no checkflag");
  //Logica:  envia a mensagem de qsh (verificar flag) e depois le a resposta (RSH), procurando pelo valor da FLAG.
  //Flag 01 = Ignicao, Flag 02 = motor ligado
  // Query Ignição e Motor
  // 1/2 10:41:13 TX: >QSH01;ID=0033;#8020;*57<
  // 1/2 10:41:13 RX: >RSH011;ID=0033;#8020;*65<
  // 1/2 10:41:15 TX: >QSH02;ID=0033;#8021;*55<
  // 1/2 10:41:15 RX: >RSH020;ID=0033;#8021;*66<

  int tentativa = 0;
  bool msg_valida = false;
#define max_tentativas 20
#define delay_entre_tentativas 200
//#define debug_QSH

// while (processando_serial2 == true)
// {
//   delay(100);
//   SerialBT.println("Serial em uso, aguardando");
// }
 processando_serial2 = true;

//Limpeza do buffer anterior (joga fora por enquanto)
while (Serial2.available() > 0) //Check if there is something in the serial buffer
    {
      char recieved = Serial2.read();
    }

  while (!((tentativa > max_tentativas) || (msg_valida == true)))
  {
    tentativa++;
    //Serial.println("cheguei no while");
#ifdef debug_QSH
    SerialBT.print("Tentativa #");
    SerialBT.print(tentativa);
    SerialBT.print(" | ");
#endif

    char SSH_FLAG[40];
    snprintf(SSH_FLAG, sizeof(SSH_FLAG), ">QSH%02d;ID=%04d;#%04X;*", FLAG_ID, MARTE_ID, MARTE_MSG_NUM);

    int r;
    unsigned char chksum;
    chksum = 0;
    for (r = 0; r < strlen(SSH_FLAG); r++)
    {
      if ((SSH_FLAG[r] == '*') && (SSH_FLAG[r - 1] == ';'))
        break;
      else
        chksum = chksum ^ SSH_FLAG[r];
    } //>QSH%02d;ID=%04d;#%04X;*
    snprintf(SSH_FLAG, sizeof(SSH_FLAG), ">QSH%02d;ID=%04d;#%04X;*%2X<\r\n", FLAG_ID, MARTE_ID, MARTE_MSG_NUM, chksum);

    Serial2.print(SSH_FLAG);
    update_msg_num();

#ifdef debug_QSH
    SerialBT.print("Debug QSH");
    SerialBT.print(" TX> ");
    //atualiza a SSH_FLAG para remover o /n e nao zoar o print
    char SSH_FLAG_debug[40];
    snprintf(SSH_FLAG_debug, sizeof(SSH_FLAG_debug), ">QSH%02d;ID=%04d;#%04X;*%2X<", FLAG_ID, MARTE_ID, MARTE_MSG_NUM, chksum);
    SerialBT.print(SSH_FLAG_debug);
#endif

    //delay(100); //aguarda o marte receber
    String inData;
    while (Serial2.available() > 0) //Check if there is something in the serial buffer
    {
      char recieved = Serial2.read();
      inData += recieved;
      if (recieved == '\n') // Add data into the "inData" until a \n is received
      {
#ifdef debug_QSH
        SerialBT.print(" RX: ");
        Serial.println(inData);
        inData.replace("\\r", "");
        inData.replace("\\n", "");
        Serial.println(inData);
        SerialBT.print(inData);
#endif
        int posRSH = inData.indexOf("RSH");
        if (posRSH != -1) //Confere se a mensagem possui RSH  <- precisa melhorar isso
        {
          //msg para referencia >RSH011;ID=0033;#8020;*65<
          //                    01234567
          //a posição do caractar de VALOR da flag fica no caracter 6 a 7
          int retorno = (inData.substring(6, 7).toInt());
          inData = "";
          //Serial.println(retorno);

#ifdef debug_QSH
          //SerialBT.print("  RX2  ");
          //inData.replace("\r\n", ""); //tira os caracteres de fim de linha
          //SerialBT.print(inData);
          SerialBT.print("  VALOR  ");
          SerialBT.println(retorno);
#endif
          msg_valida = true;
          processando_serial2 = false;
          return retorno;
        }
        else
        {
          msg_valida = false;
          SerialBT.println("Recebi uma mensagem inesperada");
          processando_serial2 = false;
        }
        inData = ""; // Clear recieved buffer
      }
    }
    delay(delay_entre_tentativas); //delay entre tentativas de comunicacao
  }
  if (msg_valida == false)
  {
    SerialBT.println("ERRO: Nao foi possivel comunicar com marte");
    return -1;
  }
  return -1;
}

/* #endregion */

/* #region  =====================================  LOGICA DE CORTE FellerMS            =====================================*/

unsigned long last_update_pins = 0;
//unsigned long last_print_info = 0;
void update_inputs(int waittime)
{
  unsigned long call_millis = millis();
  if (call_millis - last_update_pins >= waittime)
  {
    //codigo movido para a TASK
    //screen_update_inputs();
    last_update_pins = call_millis;
  }
}

void logica_feller_MS_v1()
{
  int timeout_01 = 0;
  int timeout_02 = 0;
  int timeout_03 = 0;
#define max_timeout_01 30000
#define max_timeout_02 30000
#define max_timeout_03 30000
  bool cancelar = false; //Se ultrapassar o limite de tempo, seta cancelar para true e sai do while
  
  //current_input_state[1]; marrom //Abre garra de CIMA/
  //current_input_state[2]; verde //Fecha garra de CIMA
  //current_input_state[3]; amarelo //Abre garra de BAIXO
  //current_input_state[4]; laranja //fecha garra de BAIXO
  //current_input_state[5]; vermelho //Não usada
  //Estado de garra ->  0 = fechado, 1 = aberto, -1 = indefinido
//sequencia = laranja, marrom, verde, amarelo
  //STEP01 Garra de baixo fechou? (verifica o pulso)

  if (current_input_state[4] == 1)
  {
    SerialBT.println("STEP0 OK: Garra de baixo fechou");

    //STEP01 Enquanto não abrir a garra de cima trava aqui
    while (!(current_input_state[1] == 1 || cancelar == true))
    {
      delay(10); //espera a garra de cima abrir
      timeout_01 += 10;
      if (timeout_01 > max_timeout_01)
      {
        cancelar = true;
        SerialBT.println("STEP01 CANCELADO: Demorou para abrir garra de cima");
      }
    }
    if (cancelar == false)
      SerialBT.println("STEP01 OK: Garra de cima abriu");

    //STEP02 Enquanto não fechar a garra de cima trava aqui
    while (!(current_input_state[2] == 1 || cancelar == true))
    {
      delay(10); //espera a garra de cima fechar
      timeout_02 += 10;
      if (timeout_02 > max_timeout_02)
      {
        cancelar = true;
        SerialBT.println("STEP02 CANCELADO: Demorou para fechar garra de cima");
      }
    }
    if (cancelar == false)
      SerialBT.println("STEP02 OK: Garra de cima fechou");

    //STEP03 Enquanto não abrir a garra de baixo trava aqui
    while (!(current_input_state[3] == 1 || cancelar == true))
    {
      delay(10); //espera a garra de baixo abrir
      timeout_03 += 10;
      if (timeout_03 > max_timeout_03)
      {
        cancelar = true;
        SerialBT.println("STEP03 CANCELADO: Demorou para abrir garra de baixo");
      }
    }

    if (cancelar == false)
      SerialBT.println("STEP03 OK: Garra de baixo abriu");

    //Final da logica de inputs
    if (cancelar == false) //Verificação de motor ligado via FLAG
    {
      SerialBT.println("Rotina de corte validada, verificando FLAG Marte");
      if (STATUS_Flag_Motor_Ligado == 1) //Flag 2 = flag do motor ligado
      {
        cortes_na_sessao++;
        snprintf(buf, sizeof(buf), "Corte valido. TOTAL = %d", cortes_na_sessao);
        SerialBT.println(buf);
        Serial.println(buf);
        add_CT(3);
        set_flag(Flag_TTS_Eucalipto_Cortado, 1); //usado para gerar evento de voz no celular
      }
      else if (STATUS_Flag_Motor_Ligado == 0)
      {
        SerialBT.println("Corte valido, mas flag de motor desligado, ignorado");
        add_CT(20);
      }
      else if (STATUS_Flag_Motor_Ligado == -1)
      {
        SerialBT.println("CANCELADO: Não foi possivel obter o status da FLAG de motor ligado");
        add_CT(20);
      }
    }
    else //se o cancelar tiver sido ligado
    {
      SerialBT.println("Cancelar por timeout ativado");
    }

    SerialBT.println("Fim da rotina de corte");
  }
}



void logica_feller_MS_v2()
{
  int timeout_01 = 0;
  int timeout_02 = 0;
  int timeout_03 = 0;
#define max_timeout_01 120000
#define max_timeout_02 120000
#define max_timeout_03 120000
  bool cancelar = false; //Se ultrapassar o limite de tempo, seta cancelar para true e sai do while
  
  //current_input_state[1]; marrom //Abre garra de CIMA/
  //current_input_state[2]; verde //Fecha garra de CIMA
  //current_input_state[3]; amarelo //Abre garra de BAIXO
  //current_input_state[4]; laranja //fecha garra de BAIXO
  //current_input_state[5]; vermelho //Não usada
  //Estado de garra ->  0 = fechado, 1 = aberto, -1 = indefinido
//sequencia = laranja, marrom, verde, amarelo
  //STEP01 Garra de baixo fechou? (verifica o pulso)

  if (current_input_state[1] == 1) //abriu a garra de cima?
  {
    if (debug_BT) SerialBT.println("STEP 1 OK: Abriu garra de cima");

    //STEP01 Enquanto não abrir a garra de cima trava aqui
    while (!(current_input_state[2] == 1 || cancelar == true))
    {
      delay(1); //espera a garra de cima abrir
      timeout_01 += 1;
      if (timeout_01 > max_timeout_01)
      {
        cancelar = true;
        if (debug_BT) SerialBT.println("STEP02 CANCELADO: Demorou para fechar a garra de cima");
      }
    }
    if (cancelar == false)
      if (debug_BT) SerialBT.println("STEP02 OK: fechou garra de cima");

    //Final da logica de inputs
    if (cancelar == false) //Verificação de motor ligado via FLAG
    {
      if (debug_BT) SerialBT.println("Rotina de corte validada, verificando FLAG Marte");
      if (STATUS_Flag_Motor_Ligado == 1) //Flag 2 = flag do motor ligado
      {
        cortes_na_sessao++;
        snprintf(buf, sizeof(buf), "OK_%d NOK_%d GSUP_%d", cortes_na_sessao, cortes_invalidos_na_sessao, vezes_pressionado_fecha_garra_menor);
        SerialBT.println(buf);
        //Serial.println(buf);
        add_CT(3);
        set_flag(Flag_TTS_Eucalipto_Cortado, 1); //usado para gerar evento de voz no celular
      }
      else if (STATUS_Flag_Motor_Ligado == 0)
      {
        if (debug_BT) SerialBT.println("Corte valido, mas flag de motor desligado, ignorado");
        cortes_invalidos_na_sessao++;
        //add_CT(20);
      }
      else if (STATUS_Flag_Motor_Ligado == -1)
      {
        if (debug_BT) SerialBT.println("CANCELADO: Não foi possivel obter o status da FLAG de motor ligado");
        cortes_invalidos_na_sessao++;
        //add_CT(20);
      }
    }
    else //se o cancelar tiver sido ligado
    {
      if (debug_BT) SerialBT.println("Cancelar por timeout ativado");
    }

    if (debug_BT) SerialBT.println("Fim da rotina de corte");
  }
}


/* #endregion */

#ifdef usarPCF
void checa_PCF()
{
  //TESTE DO PCF
  char buf_teste[100];
  snprintf(buf_teste, sizeof(buf_teste), "P0-%d  P1-%d  P2-%d  P3-%d  P4-%d  P5-%d  P6-%d P7-%d",
           pcf8574.digitalRead(P0), pcf8574.digitalRead(P1), pcf8574.digitalRead(P2), pcf8574.digitalRead(P3), pcf8574.digitalRead(P4), pcf8574.digitalRead(P5), pcf8574.digitalRead(P6), pcf8574.digitalRead(P7));
  Serial.println(buf_teste);
  delay(1000);
}
#endif

/* #region  =====================================  LOOP                                =====================================*/

int arvores_cortadas;
void loop()
{
  //logica_feller_MS_v1(); //considera garras de cima e de baixo, perdeu arvores
  logica_feller_MS_v2(); //considera so abre e fecha garra de cima
}
/* #endregion */
