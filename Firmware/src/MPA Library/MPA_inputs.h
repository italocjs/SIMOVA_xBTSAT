#ifndef _MPA_INPUTS_H // Para evitar multipla inclusão do mesmo codigo
#define _MPA_INPUTS_H

// Array de ints para importar os pinos.
int input_pin_index[6] = {0, input1_pin, input2_pin, input3_pin, input4_pin, input5_pin};
int output_pin_index[4] = {0, output1_pin, output2_pin, output3_pin};

// Array de ints para guardar o valor dos pinos
int current_input_state[6];
int last_input_state[6];
int output_state[4];


void print_pin_states()
{
  //\033[106mSTEP2 - Nao estava na horizontal, cancelando\033[0m
  Serial.print("\033[96mINPUTS: ");
  for (int i = 1; i < 6; i++)
  {
    char buf_0[20];
    char letra[20];
    if (current_input_state[i] == 0)
    {
      //letra = 'L';
      snprintf(letra, sizeof(letra), "\033[91mL\033[0m ");
    }
    else
    {
      //letra = 'H';
      snprintf(letra, sizeof(letra), "\033[92mH\033[0m ");
    }
    snprintf(buf_0, sizeof(buf_0), "%d%s ", i, letra);
    Serial.print(buf_0);
  }
  // Atualizar estado da saida (só para informacao)
  // Serial.print("| ");
  // for (int i = 1; i < 4; i++)
  // {
  //   char buf_0[20];
  //   snprintf(buf_0, sizeof(buf_0), "O%d=%d ", i, output_state[i]);
  //   Serial.print(buf_0);
  // }
   Serial.println("\033[0m");
}

unsigned long last_print_pins = 0;
int navertical;
#define minimum_print_time 500
bool MPA_Inputs_UPDATE_firstRUN = true;
void MPA_inputs_UPDATE() // Vai ser chamado por uma task
{
  // unsigned long call_millis = millis();

  bool mudoualgo = false;

  // Atualizar o LAST STATE
  for (int i = 1; i < (sizeof(last_input_state) / sizeof(last_input_state[0])); i++)
  {
    last_input_state[i] = current_input_state[i];
  }

  // Atualizar o CURRENT STATE
  for (int i = 1; i < (sizeof(current_input_state) / sizeof(current_input_state[0])); i++)
  {
    current_input_state[i] = digitalRead(input_pin_index[i]);
  }

  // Atualizar estado da saida (só para informacao)
  for (int i = 1; i < (sizeof(output_state) / sizeof(output_state[0])); i++)
  {
    output_state[i] = digitalRead(output_pin_index[i]);
  }

  // printa os trem,  precisa verificar denovo pq ele ainda nao sabia o novo status do pino
  for (int i = 1; i < (sizeof(last_input_state) / sizeof(last_input_state[0])); i++)
  {
    if (last_input_state[i] != current_input_state[i])
    {
      mudoualgo = true;
    }
  }



  if (mudoualgo == true && MPA_Inputs_UPDATE_firstRUN == false)
  {
    print_pin_states();
  }
  if (MPA_Inputs_UPDATE_firstRUN == true)
{
  MPA_Inputs_UPDATE_firstRUN = false; //Essa gambiarra é só para não printar toda vez que liga, pq o estado das arrays está indefinido
}

  // ESPECIFICO DO PROGRAMA DO USUARIO <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

  if (current_input_state[3] == 0 && current_input_state[4] == 1) // cabecote subiu
  {
    navertical = 1;
  }
  else if (current_input_state[3] == 1 && current_input_state[4] == 0) // cabecote desceu
  {
    navertical = 0;
  }
}

#endif