/** @file beacon.h
 *
 * @brief Lib para criar um beacon BLE utilizando o protocolo iBeacon,
 * Futuramente vou implementar algo para ficar mais facil setar o nome a ser usado
 * e mensagem transmitida
 * @par
 * COPYRIGHT NOTICE: (c) 2021 Italo C J Soares / SIMOVA.  All rights reserved.
 */

#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include "BLEBeacon.h"
#include "Arduino.h"

/* Para gerar uma UUID, uma sugestão é utilizar o site a seguir:
https://www.uuidgenerator.net/
*/

/* UUID do Beacon BLE (128 bits) */
/*
 * Importante: o UUID tem seu endian invertido quando lido em um app qualquer para scan BLE.
 * Logo, esse UUID será lido como: 0ffcd8742d3d-75a3-7f40-e78a-7035c02f
 */

#define BEACON_UUID "2fc03570-8ae7-407f-a375-3d2d74d8fc0f"

/* IMPORTANTE: o código/ID do fabricante é um identificador único, registrado no
Bluetooth SIG (Special Interest Group). Logo, trata-se de um identificador
globalmente conhecido. Para fins de teste utilize um ID qualquer mas,
em caso de fazer um produto comercial, NUNCA UTILIZE UM ID DE UMA EMPRESA
OU FABRICANTE QUE NÃO SEJA A SUA. Isso poderá levar a problemas legais.
Aqui, neste exemplo, a finalidade é de aprendizado somente.
*/
/* Lista com fabricantes registrados na SIG:
<blockquote class="wp-embedded-content" data-secret="mrqdGynoT2"><a href="https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers/">Company Identifiers</a></blockquote><iframe loading="lazy" class="wp-embedded-content" sandbox="allow-scripts" security="restricted" style="position: absolute; clip: rect(1px, 1px, 1px, 1px);" title="“Company Identifiers” — Bluetooth® Technology Website" src="about:blank" data-secret="mrqdGynoT2" width="600" height="338" frameborder="0" marginwidth="0" marginheight="0" scrolling="no" data-rocket-lazyload="fitvidscompatible" data-lazy-src="https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers/embed/#?secret=mrqdGynoT2"></iframe><noscript><iframe class="wp-embedded-content" sandbox="allow-scripts" security="restricted" style="position: absolute; clip: rect(1px, 1px, 1px, 1px);" title="“Company Identifiers” — Bluetooth® Technology Website" src="https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers/embed/#?secret=mrqdGynoT2" data-secret="mrqdGynoT2" width="600" height="338" frameborder="0" marginwidth="0" marginheight="0" scrolling="no"></iframe></noscript>
Importante: o código deve ser usado nesse programa com endian diferente do oferecido na lista
Por exemplo, se na lista o fabricante está como 0x1234, deve ser usado aqui como
0x3412.
*/
#define ID_FABRICANTE_BEACON 0x3434

/* Major e Minor do Beacon (assumindo 1 grupo (major) com 1 beacon somente (minor) */
#define MAJOR_BEACON 7
#define MINOR_BEACON 1

/* Dados do beacon (tipo, fabricante, uuid, major, minor e tx power) */
#define BEACON_DATA ""
#define BEACON_DATA_SIZE 26
#define BEACON_DATA_TYPE 0xFF /* Ver: https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile/ */

/* Objeto global para gerar o advertising do BLE */
BLEAdvertising *pAdvertising; // BLE Advertisement type

/**
 * Inicia o BLE e começa a propaganda na forma de um iBeacon, utiliza uma char array para setar o nome
 * @param BeaconSSID Deve ser uma char array contendo o nome
 * @return void.
 */
void ble_setup(char *BeaconSSID)
{
    char BLE_MPA_SSID[30];
    snprintf(BLE_MPA_SSID, sizeof(BLE_MPA_SSID), "MPA_%s", BeaconSSID);
    //Serial.begin(115200);
    Serial.println("Fazendo inicializacao do beacon...");

    /* Cria e configura um device e server BLE */
    BLEDevice::init(BLE_MPA_SSID);
    BLEServer *pServer = BLEDevice::createServer();

    /* Inicializa e configura advertising */
    pAdvertising = BLEDevice::getAdvertising();
    BLEDevice::startAdvertising();

    // configura_beacon();
    /* Cria um objeto para gerenciar o Beacon */
    BLEBeacon ble_beacon = BLEBeacon();

    /* Código/ID do fabricante */
    /* IMPORTANTE: o código/ID do fabricante é um identificador único, registrado no
       Bluetooth SIG (Special Interest Group). Logo, trata-se de um identificador
       globalmente conhecido. Para fins de teste utilize um ID qualquer mas,
       em caso de fazer um produto comercial, NUNCA UTILIZE UM ID DE UMA EMPRESA
       OU FABRICANTE QUE NÃO SEJA A SUA. Isso poderá levar a problemas legais.
       Aqui, neste exemplo, a finalidade é de aprendizado somente.
     */
    ble_beacon.setManufacturerId(ID_FABRICANTE_BEACON);

    /* Configura proximityu, major e minor do Beacon */
    ble_beacon.setProximityUUID(BLEUUID(BEACON_UUID));
    ble_beacon.setMajor(MAJOR_BEACON);
    ble_beacon.setMinor(MINOR_BEACON);

    /* Configura advertiser BLE */
    BLEAdvertisementData advertisement_data = BLEAdvertisementData();
    BLEAdvertisementData scan_response_data = BLEAdvertisementData();

    /* Indica que Bluetooth clássico não deve ser suportado */
    advertisement_data.setFlags(0x04);

    /* Informando os dados do beacon */
    std::string strServiceData = "";
    strServiceData += (char)BEACON_DATA_SIZE;
    strServiceData += (char)BEACON_DATA_TYPE;
    strServiceData += ble_beacon.getData();

    advertisement_data.addData(strServiceData);

    /* configura informações dos dados a serem enviados pelo beacon e informações de scan
       no advertiser */
    pAdvertising->setAdvertisementData(advertisement_data);
    pAdvertising->setScanResponseData(scan_response_data);

    /* Começa a funcionar como beacon (advertiser entra em ação) */
    pAdvertising->start();

    Serial.println("O beacon foi inicializado e ja esta operando.");
}