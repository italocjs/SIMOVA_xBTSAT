/** @file MPA_CONFIG.h
 *
 * @brief Agrupamento de configuraçoes (via #define) para as funções do virloc e também uma camada
 * de abstração de hardware para diferentes versões de placas a serem utilizadas
 * @par
 * COPYRIGHT NOTICE: (c) 2022 Italo C J Soares / SIMOVA.  All rights reserved.
 */

#ifndef _MPA_CONFIG_H // Para evitar multipla inclusão do mesmo codigo
#define _MPA_CONFIG_H


/* ========================== CONFIGURAÇÕES PRINCIPAIS =========================== */

#define MARTE_BAUD 19200   //VL6 19200 
//#define MARTE_BAUD 57600 //VL8 57600




/* ========================== CONFIGURAÇÕES DA PLACA  =========================== */

#define SIMOVA_MPA_ESP32

#ifdef SIMOVA_MPA_ESP32
// ========================================== PINOUT ESP WROOM ============================================
//                O01    O02    O03 || I01 || I02 || I03 || I04 || I05 || I06 || I07 || I08 ||  NC ||  NC          <- Pino no programa
// 3V3  || GND || D13 || D12 || D14 || D27 || D26 || D25 || D33 || D32 || D35 || D34 || _VN || _VP || _EN ||       <- Pino real
//      ||     ||     ||     ||     ||     ||     ||     ||     ||     ||     ||     || D39 || D36 ||     ||       <- Aliases
// USB  ||     ||     ||     ||     ||     ||     ||     ||     ||     ||     ||     ||     ||     ||     ||
//      ||     ||     ||     ||     || D16 || D17 ||     ||     ||     || SDA || D01 || D01 || SCL ||     ||       <- Aliases
// 3V3  || GND || D15 || _D2 || _D4 || RX2 || TX2 || _D5 || D18 || D19 || D21 || RX0 || TX0 || D22 || D33 ||       <- Pino real
//      ||     ||     || LED ||BUZER|| RX2 || TX2 ||     ||     ||     || SDA ||     ||     || SCL ||     ||       <- Pino no programa
// ========================================== PINOUT ESP WROOM ============================================

// ========> CONFIGURAÇÃO DE ENTRADAS E SAIDAS <========
#define output1_pin 13
#define output2_pin 12
#define output3_pin 14

#define input1_pin 27
#define input2_pin 26
#define input3_pin 25
#define input4_pin 33
#define input5_pin 32
#define input6_pin 35
#define input7_pin 34
#define input8_pin 39

#define channel_buzzer 5 //Canal ADC1_CH5 = Usado no pino 5.
#define resolution_buzzer 8
#define buzzer_pin 4

// ========> CONFIGURAÇÃO DE PORTAS DE COMUNICAÇÃO <========

#define pin_SDA 21
#define pin_SCL 22

#define pin_RX0 7
#define pin_TX0 8
#define pin_RX1 -1 // Não usado
#define pin_TX1 -1 // Não usado
#define pin_RX2 16 // Virloc
#define pin_TX2 17 // Virloc

#endif //SIMOVA_MPA_ESP3


#endif //_MPA_PINS_H