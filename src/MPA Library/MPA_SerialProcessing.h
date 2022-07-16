
#ifndef _MPA_SERIAL_PROCESSING // Para evitar multipla inclusão do mesmo codigo
#define _MPA_SERIAL_PROCESSING

#define Debug_Erro 1
#define Debug_Source 2
#define Debug_MPA 3

#include "BluetoothSerial.h" //Inclusão da Lib do bluetooth
BluetoothSerial SerialBT;    // Criando objeto do Serial
extern int debug;
extern bool Serial2InUse;

/* #region  Debug => Printa debug (no serial 0 e BT se tiver conectado) com nome da função e linha onde foi chamado */

void Smart_Print(const char str[])
{
    Serial.println(str);
    if ((SerialBT.hasClient() > 0))
    {
        SerialBT.println(str);
    }
}
void Smart_Print(String str)
{
    Serial.print(str);
    if ((SerialBT.hasClient() > 0))
    {
        SerialBT.print(str);
    }
}

void Debug(const char str[], int level, const char func[], int line)
{
    if (debug >= level)
    {
        char buf[150];
        snprintf(buf, sizeof(buf), "%s():%d  %s", func, line, str);
        Serial.print(buf);
        if ((SerialBT.hasClient() > 0))
        {
            SerialBT.print(buf);
        }
    }
}

void Debugln(const char str[], int level, const char func[], int line)
{
    if (debug >= level)
    {
        char buf[150];
        snprintf(buf, sizeof(buf), "%s():%d  %s\n", func, line, str);
        Serial.print(buf);
        if ((SerialBT.hasClient() > 0))
        {
            SerialBT.print(buf);
        }
    }
}

void Debug(String str, int level, const char func[], int line)
{
    if (debug >= level)
    {
        char buf[150];
        snprintf(buf, sizeof(buf), "%s():%d  %s", func, line, str.c_str()); // c_str() evita erro de char* qdo espera string
        Serial.print(buf);
        if ((SerialBT.hasClient() > 0))
        {
            SerialBT.print(buf);
        }
    }
}

void Debugln(String str, int level, const char func[], int line)
{
    if (debug >= level)
    {
        char buf[150];
        snprintf(buf, sizeof(buf), "%s():%d  %s\n", func, line, str.c_str());
        Serial.print(buf);
        if ((SerialBT.hasClient() > 0))
        {
            SerialBT.print(buf);
        }
    }
}
/* #endregion */

/* #region  Ferramentas diversas */
// Variavel para obter a UUID do chip
int getchipID()
{
    uint32_t chipId = 0;
    for (int i = 0; i < 17; i = i + 8)
    {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    char buf[20];
    snprintf(buf, sizeof(buf), "CHIP_ID=%d", chipId);
    Debug(buf, Debug_MPA, __func__, __LINE__);
    return chipId;
}

/* #endregion */

/* #region  Callback BT */
char bt_serial_temporario[150];
// Processamento da entrada via bluetooth - em desenvolvimento
void callback_ESP_BT(const uint8_t *buffer, size_t size) // Funçao construida mas não desenvolvida nesse programa.  Processa a entrada
{
    String inData;
    for (size_t i = 0; i < size; i++) // loop para colocar todo o buffer numa array
    {
        const char c = buffer[i];
        bt_serial_temporario[i] = c;
        inData += c;
    }
    Debug(inData, Debug_MPA, __func__, __LINE__); // printa a entrada crua
    if (inData == "restart\n")
    {
        Debug("Reiniciando", Debug_MPA, __func__, __LINE__); // printa a entrada crua
        // O ESP.restart misteriosamente roda antes do bluetooth enviar, bizarro demais
        ESP.restart();
    }
    else if (inData == "clean_vars\n")
    {
        SerialBT.println("Not working yet CLEANVARS");
    }
    else if (inData == "report\n")
    {
        SerialBT.println("Not working yet REPORT");
    }
    else if (inData == "setBTname\n")
    {
        SerialBT.println("Not working yet SETBTNAME");
    }
    else
    {
        SerialBT.println("unknown command");
    }

    // inData = ""; // Clear recieved buffer
    //  ELSE
}
/* #endregion */

#endif