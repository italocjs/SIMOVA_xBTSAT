# Simova_MPA_LIBRARY
## Biblioteca base para as placas MPA (Módulo de processamento auxiliar)

### Versão da biblioteca:  V1.0.0
Versões de hardware compativeis:  
- SimovaMPA V1.0


Biblioteca utilizada para manter as funções e lógicas necessárias para o funcionamento do MPA separado da logica de execução / rotina de operação.

Exemplo de como deve ficar o programa:
-> MPA Library -> Biblioteca MPA contém funções serial, leitura de portas, bluetooth, setups, LCD, etc
-> main.h      -> Logica especifica da aplicação

## Funções suportadas
### Virloc
- void add_CT(int CT_NUMBER)                        -> Adicionar contador
- void sub_CT(int CT_NUMBER)                        -> Subtrair contador
- void set_CT(int CT_NUMBER)                        -> Definir contador
- void set_flag(int FLAG_ID, bool valor_flag)       -> Definir flag
- int check_flag(int FLAG_ID)                       -> Retorna valor da flag
- void toggle_flag(int FLAG_ID)                     -> Inverter flag
- bool MPA_find_id(char *out_ID)                    -> Encontra e retorna a ID

### Audio
- void play_error()                                 -> Som de erro 
- void play_valido()                                -> Som de OK
- void play_bootOK()                                -> Som de Boot ok

### Debug via Serial/BTSerial
- void Debug(const char str[], int level, const char func[], int line)
- void Debugln(const char str[], int level, const char func[], int line)

Verifica o nivel de debug, se atender printa Arquivo + Linha + mensagem,  se houver cliente conectado no bluetooth printa lá também. 

DEBUG LEVEL: 1 = Erro | 2 = Erro, Debug_Source | 3 = Erro, Debug_Source, Debug_MPA

### Funções SERIAL/BTSERIAL em desenvolvimento
- "restart"         -> Função para reiniciar o ESP quando invocada
- "clean-restart"   -> Função para limpar todas as variaveis salvas na memoria e reiniciar
- "debug-inputs"    -> Ativa o debug das entradas do módulo (responde para quem pediu)
- "report"          -> Responde as variaveis salvas na memória
- "errors"          -> Responde log dos erros que ocorreram (ex watchdog ou comunicação) 





## Copyright
COPYRIGHT: (c) 2022 Italo C J Soares / SIMOVA TECNOLOGIA E SERVIÇOS DE INFORMATICA LTDA
All rights reserved. You may NOT distribute or reproduce this code (raw or compiled) without written authorization from either the author or the company. Open source code will be disclosed upon request, as required by its license.

Contact: italocjs@live.com / fabio.calegari@simova.com.br