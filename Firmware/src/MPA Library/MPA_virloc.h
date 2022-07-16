#ifndef _MPA_VIRLOC // Para evitar multipla inclusão do mesmo codigo
#define _MPA_VIRLOC

#include "MPA_Buzzer.h"
#include "esp_ipc.h"

#define GREEN_OK "\033[92mOK\033[0m"
#define RED_INVALIDO "\033[91mERRO\033[0m"
#define debug_QSH
#define mensagem_padrao_MPA_find_ID ">QCT03;ID=xxxx;#8000;*5B<"
#define DEFAULT_TIMEOUT_VALIDATION 500
#define DEFAULT_TIMEOUT_CLEANRX2 50
#define max_tentativas 10
#define delay_entre_tentativas 150
int QTDE_DE_ERROS_SERIAL = 0;

#include "MPA_SerialProcessing.h" //usamos o Debug e BTDebug dessa lib.
int MARTE_MSG_NUM = 0x8000;
extern bool Serial2InUse;

/* #region  ===================================== SETUP & TOOLS =====================================*/

// retorna um char contendo o valor do checksum, ATENCAO, precisa converter para HEXA depois
unsigned char MPA_virloc_Calc_CHECKSUM(String input)
{
    char mensagem007[150]; // Essa função PRECISA ser otimizada para alocar o tamanho dinamicamente
    strcpy(mensagem007, input.c_str());
    int r;
    unsigned char chksum;
    chksum = 0;
    for (r = 0; r < strlen(mensagem007); r++)
    {
        if ((mensagem007[r] == '*') && (mensagem007[r - 1] == ';'))
            break;
        else
            chksum = chksum ^ mensagem007[r];
    }
    // if (debug >= Debug_MPA)
    // {
    //     char buf[30];
    //     snprintf(buf, sizeof(buf), "CHKSUM CALC = 0x%x", chksum);
    //     Serial.println(buf);
    // }
    return chksum;
}
// Soma +1 no contador, a partir do 0x8000,  ao chegar no 0xFFFF começa denovo do 0x8000
void marte_msg_num_update()
{
    if (MARTE_MSG_NUM >= 0xFFFF)
    {
        MARTE_MSG_NUM = 0x8000;
    }
    MARTE_MSG_NUM++;
}
// Encontra ID Marte,  retorna char[5]
bool MPA_find_id(char *out_ID)
{
    int tentativa = 0;
    bool msg_valida = false;

    // Limpeza do buffer anterior
    char garbage;
    while (Serial2.available() > 0)
    {
        garbage = Serial2.read();
    }
    if (sizeof(garbage) > 1)
    { // if só pro compilador parar de encher o saco com variavel não usada
    }

    while (!((tentativa >= max_tentativas) || (msg_valida == true)))
    {
        tentativa++;
        Serial2.print(mensagem_padrao_MPA_find_ID);

        String inData;
        while (Serial2.available() > 0) // Check if there is something in the serial buffer
        {
            char recieved = Serial2.read();
            inData += recieved;
            if (recieved == '\n') // Add data into the "inData" until a \n is received
            {
                int posID = inData.indexOf("ID");
                if (posID != -1) // A string possui ID?
                {
                    // msg para referencia >RCT03 00456;ID=4688;#8000;*4D<
                    strcpy(out_ID, inData.substring(posID + 3, posID + 7).c_str());
                    char buf[30];
                    snprintf(buf, sizeof(buf), "MARTE ID ENCONTRADA = %s", out_ID);
                    Serial.println(buf);
                    msg_valida = true;
                    return true;
                }
                else
                {
                    msg_valida = false;
                    Serial.println("MARTE ID NÃO ENCONTRADA");
                }
                inData = ""; // Clear recieved buffer
            }
        }
        delay(delay_entre_tentativas); // delay entre tentativas de comunicacao
    }
    if (msg_valida == false)
    {
        Serial.println("Falha comunicando com marte");
        // char buf[20];
        // snprintf(buf, sizeof(buf), "xxxx");
        strcpy(out_ID, "xxxx");
        return false;
    }
    return false;
}
// Verifica se tem algo no RX2 e descarta, Essa função não é 100% eficiente, pois pode chegar dados poucos instantes depois dela rodar, e deixar lixo na serial
void CLEAN_RX2()
{
    while (Serial2.available() > 0) // Check if there is something in the serial buffer
    {
        Serial2.read();
        // char recieved = Serial2.read();
    }
}

//Recebe e descarta a proxima mensagem recebida no serial, Util em casos que a resposta não nos interessa. retorna true se receber, false se der timeout
bool RECEIVE_AND_DISCART_RX2()
{
    char return_char_array[100];
    bool TIMEOUT_EXPIRED = false;
    bool message_completed = false;
    int timeout = 0;
    Serial2InUse = true;
    String inData;
    // delay(5);
    while (!(message_completed == true || TIMEOUT_EXPIRED == true)) // Check if there is something in the serial buffer
    {
        char recieved;
        if (Serial2.available() > 0)
        {
            recieved = Serial2.read();
            inData += recieved;
            if (recieved == '\n')
            {
                message_completed = true;
            }
        }

        else if (timeout > DEFAULT_TIMEOUT_VALIDATION)
        {
            TIMEOUT_EXPIRED = true;
            Serial.println("\033[91mRECEIVE RX2 ERRO TIMEOUT\033[0m");
            return false;
        }
        else
        {
            timeout++;
            // vTaskDelay(1 / portTICK_PERIOD_MS); // espera 1ms pra fazer a proxima leitura se o buffer não tiver nada ainda
            delay(1); // Nesse caso é melhor usar o delay e travar o processador, evita mensagem corrompida.
        }
    }
    // Serial.print("RX2: ");
    // Serial.print(inData); // Se quiser debugar a entrada crua
    // Serial.print(" ");
    inData.replace("\r", "");
    inData.replace("\n", "");
    strcpy(return_char_array, inData.c_str());
    Serial2InUse = false;
    return true;
}
// Extrai do inData o numero da mensagem e valida se é a mesma
bool VALIDATE_MSG_NUM(String inData, int MSG_NUM)
{
    char msgnumHEX[4];
    snprintf(msgnumHEX, sizeof(msgnumHEX), "%04X", MSG_NUM);
    if (inData.indexOf(msgnumHEX) != -1) // Does the exact match exists? MSG NUM is known
    {
        return true;
    }

    else
    {
        Serial.println("\033[91mERRO MSGNUM\033[0m");
        return false;
    }
}
// Extrai do inData se o tipo de resposta é igual o esperado (EX: RCT, SCT...)
bool VALIDATE_RESPONSE_TYPE(String inData, String FindThis)
{
    if (inData.indexOf(FindThis) == -1)
    { // Existe na string? Retorna TRUE
        Serial.println("\033[91mERRO TIPO N ENCONTRADO\033[0m");
        return false;
    }
    else if (inData.indexOf(FindThis) > 1)
    { // Existe na string? Retorna TRUE
        Serial.println("\033[91mERRO TIPO TRUNCADO\033[0m");
        return false;
    }
    else if (inData.indexOf(FindThis) == 1)
    {
        return true;
    }

    // Situacao de mensagens truncadas
    //  Serial.println(inData.indexOf(FindThis));
    //  if (inData.indexOf(FindThis) > 1)
    //  { //

    //     return false;
    // }
    return false;
}
// Extrai do inData o checksum e valida.
bool VALIDATE_CHECKSUM(String inData)
{
    char CHKSUMHEX[3];
    snprintf(CHKSUMHEX, sizeof(CHKSUMHEX), "%02X", MPA_virloc_Calc_CHECKSUM(inData));
    // Serial.println(CHKSUMHEX);
    if (inData.indexOf(CHKSUMHEX) != -1)
    { // Existe na string? Retorna TRUE
        return true;
    }
    else
    {
        Serial.println("\033[91mERRO CHKSUM\033[0m");
        return false;
    }
}
// Executa em uma unica função as tres validações acima
bool VALIDATE_VIRLOC_MESSAGE(String FindThis, char *Input_data, int backup_msg_num)
{
    //=============== VALIDACAO =============
    if (VALIDATE_RESPONSE_TYPE(Input_data, FindThis) == true)
    {
        if (VALIDATE_MSG_NUM(Input_data, backup_msg_num) == true)
        {
            if (VALIDATE_CHECKSUM(Input_data) == true)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return false;
}

// bool VALIDATE_VIRLOC_MESSAGE(String FindThis, char *Input_data, int backup_msg_num)
// {
//     //=============== VALIDACAO =============
//     if (VALIDATE_CHECKSUM(Input_data) &&
//         VALIDATE_MSG_NUM(Input_data, backup_msg_num) &&
//         VALIDATE_RESPONSE_TYPE(Input_data, FindThis))
//     {
//         return true;
//     }
//     else
//     {
//         return false;
//     }
// }
// Recebe uma string no RX2 e retorna no *char apontado, Possui timeout de 500ms, espera sozinho a mensagem finalizar (espera um \n)
bool RECEIVE_RX2(char *return_char_array)
{
    bool TIMEOUT_EXPIRED = false;
    bool message_completed = false;
    int timeout = 0;
    Serial2InUse = true;
    String inData;
    // delay(5);
    while (!(message_completed == true || TIMEOUT_EXPIRED == true)) // Check if there is something in the serial buffer
    {
        char recieved;
        if (Serial2.available() > 0)
        {
            recieved = Serial2.read();
            inData += recieved;
            if (recieved == '\n')
            {
                message_completed = true;
            }
        }

        else if (timeout > DEFAULT_TIMEOUT_VALIDATION)
        {
            TIMEOUT_EXPIRED = true;
            Serial.println("\033[91mRECEIVE RX2 ERRO TIMEOUT\033[0m");
            return false;
        }
        else
        {
            timeout++;
            // vTaskDelay(1 / portTICK_PERIOD_MS); // espera 1ms pra fazer a proxima leitura se o buffer não tiver nada ainda
            delay(1); // Nesse caso é melhor usar o delay e travar o processador, evita mensagem corrompida.
        }
    }
    // Serial.print("RX2: ");
    // Serial.print(inData); // Se quiser debugar a entrada crua
    // Serial.print(" ");
    inData.replace("\r", "");
    inData.replace("\n", "");
    strcpy(return_char_array, inData.c_str());
    return true;
}

/* #endregion */

/* #region  ===================================== MANIPULACAO DE CONTADORES =====================================*/
/*
  Contadores:
    - Obter o valor de um contador  - GET_CT_VALUE(int CT_NUMBER, char *MARTE_ID)
    - Setar o valor de um contador  - SET_CT_VALUE(int CT_NUMBER, int CT_VALUE, char *MARTE_ID)
*/

// Obtet o valor de um contador, valida a mensagem mas não tenta novamente se der erro
int GET_CT_VALUE(int CT_NUMBER, char *MARTE_ID)
{
    Serial2InUse = true; // pausa tasks que usam serial2
    int backup_msg_num = MARTE_MSG_NUM;
    marte_msg_num_update(); // ja pode atualizar, ja que vamos usar o backup

    /* ===== TRANSMITE QCTxx ===== */
    char QCT_BUFFER[40];
    snprintf(QCT_BUFFER, sizeof(QCT_BUFFER), ">QCT%02d;ID=%s;#%04X;*", CT_NUMBER, MARTE_ID, backup_msg_num);
    snprintf(QCT_BUFFER, sizeof(QCT_BUFFER), ">QCT%02d;ID=%s;#%04X;*%2X<\r\n", CT_NUMBER, MARTE_ID, backup_msg_num, MPA_virloc_Calc_CHECKSUM(QCT_BUFFER));
    Serial.print("TX2 QCT: ");
    Serial.print(QCT_BUFFER);
    CLEAN_RX2(); // chamar no ultimo instante possivel antes da transmissão para evitar "comer" outros pacotes
    Serial2.print(QCT_BUFFER);
    // delay(10);
    vTaskDelay(10 / portTICK_PERIOD_MS);

    /* ===== RECEBE RCTxx yyyyy ===== */
    char RCT_BUFFER[40];
    RECEIVE_RX2(RCT_BUFFER);
    Serial.print("RX2 RCT: ");
    Serial.println(RCT_BUFFER);

    if (VALIDATE_VIRLOC_MESSAGE("RCT", RCT_BUFFER, backup_msg_num) == true) // utiliza o BKP_MSG_NUM pq a resposta é sempre igual
    {
        // msg para referencia >RCT03 00456;ID=4688;#8000;*4D<
        String msg = RCT_BUFFER;
        int CT_VALUE = -1;
        try
        {
            CT_VALUE = (msg.substring(7, 12)).toInt();
        }
        catch (const std::exception &e)
        {
            Serial.println("PEGUEI NO CATCH VIADO");
            delay(2000);
        }

        Serial.print("CT");
        Serial.print(CT_NUMBER);
        Serial.print(" = ");
        Serial.println(CT_VALUE);
        Serial2InUse = false; // pausa tasks que usam serial2
        return CT_VALUE;
    }
    else
    {
        Serial.println("ERRO AO OBTER NUMERO DA CT");
        Serial2InUse = false; // pausa tasks que usam serial2
        return -1;
    }
    return -1;
}

// usado para somar, subtrair ou setar um valor baseado na ultima leitura do CT. Repete transmissão se o numero não bater
bool ADD_CT_VALUE(int CT_NUMBER, int NUMBER_TO_SUM, char *MARTE_ID)
{
    Serial.println("ENTREI NO ADDCT");
    Serial2InUse = true; // pausa tasks que usam serial2
    int backup_msg_num = MARTE_MSG_NUM;
    marte_msg_num_update(); // ja pode atualizar, ja que vamos usar o backup
    bool sucesso = false;
    int tentativas = 0;

    //==================Transmite a primeira vez para conhecer o CT =================
                // Serial.println("VOU PEGAR A CT");
                // int current_CT_value = GET_CT_VALUE(CT_NUMBER, MARTE_ID);
                // if (current_CT_value == -1)
                // {
                //     Serial.println("Erro grave na comunicação do CT, ABORTANDO");
                //     // play_grave_error();
                //     esp_ipc_call(0, &buzzer_erro_grave, (void *)0);
                //     // para evitar gravar o contador como 0, retorna aqui o programa
                //     Serial2InUse = false;
                //     return false;
                // }
                // // Enquanto não validar a transmissão da mensagem, tenta denovo
                // bool CT_VALIDO = false;
                // Serial.println("VOU ENTRAR NO WHILE");
    while (sucesso == false)
    {
        Serial2InUse = true;
        /* ===== OBTEM VALOR DO CT ===== */
        int valor_do_ct_antes = GET_CT_VALUE(CT_NUMBER, MARTE_ID);

        /* ===== TRANSMITE SCTxx ===== */     
        if (valor_do_ct_antes != -1) //se nao der erro, transmite +1 e depois verifica
        {
            char SCT_BUFFER[50];
            snprintf(SCT_BUFFER, sizeof(SCT_BUFFER), ">SCT%02d +1;ID=%s;#%04X;*", CT_NUMBER, MARTE_ID, backup_msg_num);
            snprintf(SCT_BUFFER, sizeof(SCT_BUFFER), ">SCT%02d +1;ID=%s;#%04X;*%2X<\r\n", CT_NUMBER, MARTE_ID, backup_msg_num, MPA_virloc_Calc_CHECKSUM(SCT_BUFFER));
            Serial.print("TX2_SCT: ");
            Serial.print(SCT_BUFFER);
            Serial2.print(SCT_BUFFER);
            delay(10);
            int valor_do_ct_depois = GET_CT_VALUE(CT_NUMBER, MARTE_ID);
            if (valor_do_ct_depois == valor_do_ct_antes + 1) //Transmitiu e somou +1 certinho?
            {
                sucesso = true;
                Serial2InUse = false;
                char buf[40];
                snprintf(buf, sizeof(buf), "\033[93mADDCT OK - CT%02d = %d\033[0m", CT_NUMBER, valor_do_ct_depois);
                Smart_Print(buf);
                return true;
            }
            
        }
        if (tentativas == max_tentativas)
        {
            Serial2InUse = false;
            Serial.println("Muitas tentativas no ADD_CT");
            esp_ipc_call(0, &buzzer_erro_grave, (void *)0);
            return false;
        }
        delay(delay_entre_tentativas);
        tentativas++;
        CLEAN_RX2();
    }
    Serial2InUse = false;
    return false;
}

// essa funcao ta perigosa, pode setar o CT como zero se nao conseguir achar um CT no boot e depois conseguir.
//  usado para somar, subtrair ou setar um valor baseado na ultima leitura do CT. Repete transmissão se o numero não bater
bool SET_CT_VALUE(int CT_NUMBER, int CT_VALUE, char *MARTE_ID)
{
    Serial2InUse = true; // pausa tasks que usam serial2
    int backup_msg_num = MARTE_MSG_NUM;
    marte_msg_num_update(); // ja pode atualizar, ja que vamos usar o backup
    bool sucesso = false;
    int tentativas = 0;
    int current_CT_value = GET_CT_VALUE(CT_NUMBER, MARTE_ID);
    if (current_CT_value == -1)
    {
        Serial.println("Erro grave na comunicação do CT");
        // para evitar gravar o contador como 0, retorna aqui o programa
        return false;
    }
    // Enquanto não validar a transmissão da mensagem, tenta denovo
    while (sucesso == false)
    {
        /* ===== TRANSMITE SCTxx ===== */
        char SCT_BUFFER[50];
        snprintf(SCT_BUFFER, sizeof(SCT_BUFFER), ">SCT%02d %05d;ID=%s;#%04X;*", CT_NUMBER, CT_VALUE, MARTE_ID, backup_msg_num);
        snprintf(SCT_BUFFER, sizeof(SCT_BUFFER), ">SCT%02d %05d;ID=%s;#%04X;*%2X<\r\n", CT_NUMBER, CT_VALUE, MARTE_ID, backup_msg_num, MPA_virloc_Calc_CHECKSUM(SCT_BUFFER));
        Serial.print("TX2_SCT: ");
        Serial.print(SCT_BUFFER);
        CLEAN_RX2(); // chamar no ultimo instante possivel antes da transmissão para evitar "comer" outros pacotes
        delay(10);
        Serial2.print(SCT_BUFFER);
        if (GET_CT_VALUE(CT_NUMBER, MARTE_ID) == CT_VALUE)
        {
            sucesso = true;
            Serial2InUse = false;
            return true;
        }
        else
        {
            tentativas++;
            if (tentativas == max_tentativas)
            {
                Serial2InUse = false;
                return false;
            }
        }
    }
    Serial2InUse = false;
    return false;
}

/* #endregion */

/* #region  ===================================== MANIPULACAO DE FLAGS =====================================*/
/* PRECISA DE MELHORIAS NO TRATAMENTO DA RESPOSTA!
  Flags:
    - Inverter valor de flag    - toggle_flag_SSH(int num_flag)
    - Setar valor de flag       - set_flag(int num_flag, bool valor)
    - Consultar valor de flag   - check_flag_QSH(int num_flag), retorna int -1(erro), 0 ou 1
*/
bool flag_store[100];
// função para alterar de 0 -> 1 ou 1 -> 0 o valor de uma Flag, que será usada para gerar um evento de log GP
void toggle_flag(int FLAG_ID, char *MARTE_ID)
{
    int backup_msg_num = MARTE_MSG_NUM;
    marte_msg_num_update();
    // Exemplo de MSG: TX  >SCT03 1234;ID=0071;#8003;*78<
    // Exemplo de MSG: RX  >RCT03 01234;ID=0071;#8003;*49<

    // Obtem o valor da flag atual na "flag store" (que armazena o ultimo estado da flag)
    bool current_flag_status = !(flag_store[FLAG_ID - 1]); //-1 pq o index começa em zero

    char SSH_FLAG[40];
    snprintf(SSH_FLAG, sizeof(SSH_FLAG), ">SSH%02d%d;ID=%s;#%04X;*", FLAG_ID, current_flag_status, MARTE_ID, backup_msg_num);

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
    snprintf(SSH_FLAG, sizeof(SSH_FLAG), ">SSH%02d%d;ID=%s;#%04X;*%2X<\r\n", FLAG_ID, current_flag_status, MARTE_ID, backup_msg_num, chksum);
    Serial.print(SSH_FLAG);
    Serial2.print(SSH_FLAG);
    marte_msg_num_update();
    flag_store[FLAG_ID - 1] = !flag_store[FLAG_ID - 1];
}

// Função para setar o valor de uma flag
void set_flag(int FLAG_ID, bool valor_flag, char *MARTE_ID)
{
    Serial2InUse = true;
    int backup_msg_num = MARTE_MSG_NUM;
    marte_msg_num_update();

    // Exemplo de MSG: TX  >SCT03 1234;ID=0071;#8003;*78<
    // Exemplo de MSG: RX  >RCT03 01234;ID=0071;#8003;*49<

    // Obtem o valor da flag atual na "flag store" (que armazena o ultimo estado da flag)
    char SSH_FLAG[40];
    snprintf(SSH_FLAG, sizeof(SSH_FLAG), ">SSH%02d%d;ID=%s;#%04X;*", FLAG_ID, valor_flag, MARTE_ID, backup_msg_num);

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
    snprintf(SSH_FLAG, sizeof(SSH_FLAG), ">SSH%02d%d;ID=%s;#%04X;*%2X<\r\n", FLAG_ID, valor_flag, MARTE_ID, backup_msg_num, chksum);
    Serial.print("TX2 SFL: ");
    Serial.print(SSH_FLAG);
    Serial2.print(SSH_FLAG);
    delay(5);
    Serial2InUse = false;
    RECEIVE_AND_DISCART_RX2(); //Adicionado pois estava poluindo o serial e causando confusão em outros comandos
}

// check_flag_QSH(int numero da flag), retorna o valor da flag consultada ou -1 em caso de erro
#define debug_QSH
int check_flag(int FLAG_ID, char *MARTE_ID)
{
    // Logica:  envia a mensagem de qsh (verificar flag) e depois le a resposta (RSH), procurando pelo valor da FLAG.o
    // TX: >QSH01;ID=0033;#8020;*57<
    // RX: >RSH011;ID=0033;#8020;*65<

    int backup_msg_num = MARTE_MSG_NUM;
    marte_msg_num_update();
    int tentativa = 0;
    bool msg_valida = false;
    Serial2InUse = true;

    // Limpeza do buffer anterior (joga fora por enquanto)

    while (!((tentativa > max_tentativas) || (msg_valida == true)))
    {
        tentativa++;

        char QSH_MSG[50];
        snprintf(QSH_MSG, sizeof(QSH_MSG), ">QSH%02d;ID=%s;#%04X;*", FLAG_ID, MARTE_ID, backup_msg_num);
        snprintf(QSH_MSG, sizeof(QSH_MSG), ">QSH%02d;ID=%s;#%04X;*%2X<\r\n", FLAG_ID, MARTE_ID, backup_msg_num, MPA_virloc_Calc_CHECKSUM(QSH_MSG));
        CLEAN_RX2();
        Serial.print("TX2 CFL: ");
        Serial.print(QSH_MSG);
        Serial2.print(QSH_MSG);
        delay(2);
        char RSH_BUFFER[40];
        RECEIVE_RX2(RSH_BUFFER);

        Serial.print("RX2 CFL: ");
        Serial.println(RSH_BUFFER);

        //A função abaixo garante que vai receber apenas a resposta do tipo e numero certo
        //Estavam vindo msgs de numero diferente e interferindo.
        char aa[10];
        snprintf(aa, sizeof(aa), "RSH%02d",FLAG_ID);
        String teste = String(aa);
        //strcpy(aa, teste.c_str()); 
       
       
        // String teste = "RSH";
        // teste += FLAG_ID;
        //Serial.println(teste);

        if (VALIDATE_VIRLOC_MESSAGE(teste, RSH_BUFFER, backup_msg_num) == true)
        {
            /* Msg valida*/
            String msg = RSH_BUFFER;
            int retorno = (msg.substring(6, 7).toInt());
            msg_valida = true;
            CLEAN_RX2(); // EVITA DE OUTRAS FUNCOES LEREM ISSO
            Serial2InUse = false;
            return retorno;
        }
        else
        {
            msg_valida = false;
            // Serial.println("Recebi uma mensagem inesperada");
            // Serial2InUse = false;
            // esp_ipc_call(0, &buzzer_erro, (void *)0);
            // return -1;
        }
    delay(delay_entre_tentativas); // delay entre tentativas de comunicacao
    }
    if (msg_valida == false)
    {
        Serial.println("ERRO CHECK FLAG: Nao foi possivel comunicar com marte");
        RECEIVE_AND_DISCART_RX2(); // EVITA DE OUTRAS FUNCOES LEREM ISSO
        esp_ipc_call(0, &buzzer_erro, (void *)0);
        return -1;
    }
    return -1;
}
 
/* #endregion */

#endif