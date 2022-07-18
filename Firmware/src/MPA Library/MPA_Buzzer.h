/** @file MPA_buzzer.h
 *
 * @brief Lib para ativar e tocar um som no buzzer interno do modulo
 * @par
 * COPYRIGHT NOTICE: (c) 2021 Italo C J Soares / SIMOVA.  All rights reserved.
 * 
 * Todas as funções estão preparadas para rodar usando o FreeRTOS, utilizando o vtaskdelay.
 * Indico que sejam chamadas usando o comando: 
 * esp_ipc_call(0, &play_valido,(void*) a); 
 * 0 = core a ser executado
 * assim ele não vai atrapalhar a execução do programa primario no core1; 
 * 
 */

/* #region  - Define de notas sonoras */

#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978
#define REST 0

/* #endregion */

int error_beep_sound[] = {NOTE_C7, 0, NOTE_C7, 0, NOTE_C7, 0};
int error_beep_duration[] = {80, 20, 80, 20, 80, 20};
int grave_error_beep_sound[] = {NOTE_C7, 0, NOTE_C7, 0, NOTE_C7, 0};
int grave_error_beep_duration[] = {500, 100, 500, 100, 500, 100};

int bootOK_beep_sound[] = {NOTE_G6, 0, NOTE_G6, 0, NOTE_C7, 0};
int bootOK_beep_duration[] = {80, 40, 80, 40, 100, 50};
int Valido_beep_sound[] = {NOTE_G6, 0, NOTE_C7, 0};
int Valido_beep_duration[] = {80, 40, 100, 50};
int invalido_beep_sound[] = {NOTE_G6, 0, NOTE_G6, 0};
int invalido_beep_duration[] = {80, 40, 100, 50};

unsigned int last_call_error = 0;
void buzzer_erro(void *arg)
{
  unsigned long currentMillis = millis();
  if (currentMillis - last_call_error >= 5000)
  {
    // save the last time you blinked the LED
    last_call_error = currentMillis;

    for (int i = 0; i < (sizeof(error_beep_sound) / sizeof(error_beep_sound[0]));
         i++)
    {
      ledcWriteTone(channel_buzzer, error_beep_sound[i]);
      vTaskDelay(error_beep_duration[i] / portTICK_PERIOD_MS);
    }
  }
}

unsigned int last_call_error_grave = 0;
void buzzer_erro_grave(void *arg)
{
  unsigned long currentMillis = millis();
  if (currentMillis - last_call_error_grave >= 5000)
  {
    // save the last time you blinked the LED
    last_call_error_grave = currentMillis;

    for (int i = 0;
         i < (sizeof(grave_error_beep_sound) / sizeof(grave_error_beep_sound[0]));
         i++)
    {
      ledcWriteTone(channel_buzzer, grave_error_beep_sound[i]);
      vTaskDelay(grave_error_beep_duration[i] / portTICK_PERIOD_MS);
    }
  }
}

void buzzer_ok(void* arg) {
  for (int i = 0;
       i < (sizeof(Valido_beep_sound) / sizeof(Valido_beep_sound[0])); i++) {
    ledcWriteTone(channel_buzzer, Valido_beep_sound[i]);;
        vTaskDelay(Valido_beep_duration[i] / portTICK_PERIOD_MS);
  }
}

void buzzer_nok(void* arg) {
  for (int i = 0;
       i < (sizeof(invalido_beep_sound) / sizeof(invalido_beep_sound[0]));
       i++) {
    ledcWriteTone(channel_buzzer, invalido_beep_sound[i]);
    vTaskDelay(invalido_beep_duration[i] / portTICK_PERIOD_MS);
  }
}

void buzzer_boot(void* arg) {
  for (int i = 0;
       i < (sizeof(bootOK_beep_sound) / sizeof(bootOK_beep_sound[0])); i++) {
    ledcWriteTone(channel_buzzer, bootOK_beep_sound[i]);
    vTaskDelay(bootOK_beep_duration[i] / portTICK_PERIOD_MS);
  }
}



/* #region  Musica do superMario */
//Toca a musica do Super Mario no buzzer
void buzzer_supermario() {
  // change this to make the song slower or faster
  int tempo = 200;
 
int melody[] = {

  // Super Mario Bros theme
  // Score available at https://musescore.com/user/2123/scores/2145
  // Theme by Koji Kondo
  
  
  NOTE_E5,8, NOTE_E5,8, REST,8, NOTE_E5,8, REST,8, NOTE_C5,8, NOTE_E5,8, //1
  NOTE_G5,4, REST,4, NOTE_G4,8, REST,4, 
  NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, // 3
  NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
  NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
  REST,8, NOTE_E5,4,NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,
  NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, // repeats from 3
  NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
  NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
  REST,8, NOTE_E5,4,NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,

  
  REST,4, NOTE_G5,8, NOTE_FS5,8, NOTE_F5,8, NOTE_DS5,4, NOTE_E5,8,//7
  REST,8, NOTE_GS4,8, NOTE_A4,8, NOTE_C4,8, REST,8, NOTE_A4,8, NOTE_C5,8, NOTE_D5,8,
  REST,4, NOTE_DS5,4, REST,8, NOTE_D5,-4,
  NOTE_C5,2, REST,2,

  REST,4, NOTE_G5,8, NOTE_FS5,8, NOTE_F5,8, NOTE_DS5,4, NOTE_E5,8,//repeats from 7
  REST,8, NOTE_GS4,8, NOTE_A4,8, NOTE_C4,8, REST,8, NOTE_A4,8, NOTE_C5,8, NOTE_D5,8,
  REST,4, NOTE_DS5,4, REST,8, NOTE_D5,-4,
  NOTE_C5,2, REST,2,

  NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,4,//11
  NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2,

  NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,8, NOTE_E5,8,//13
  REST,1, 
  NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,4,
  NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2,
  NOTE_E5,8, NOTE_E5,8, REST,8, NOTE_E5,8, REST,8, NOTE_C5,8, NOTE_E5,4,
  NOTE_G5,4, REST,4, NOTE_G4,4, REST,4, 
  NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, // 19
  
  NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
  NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
  REST,8, NOTE_E5,4, NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,

  NOTE_C5,-4, NOTE_G4,8, REST,4, NOTE_E4,-4, // repeats from 19
  NOTE_A4,4, NOTE_B4,4, NOTE_AS4,8, NOTE_A4,4,
  NOTE_G4,-8, NOTE_E5,-8, NOTE_G5,-8, NOTE_A5,4, NOTE_F5,8, NOTE_G5,8,
  REST,8, NOTE_E5,4, NOTE_C5,8, NOTE_D5,8, NOTE_B4,-4,

  NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,//23
  NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
  NOTE_D5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_G5,-8, NOTE_F5,-8,
  
  NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2, //26
  NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,
  NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
  NOTE_B4,8, NOTE_F5,4, NOTE_F5,8, NOTE_F5,-8, NOTE_E5,-8, NOTE_D5,-8,
  NOTE_C5,8, NOTE_E4,4, NOTE_E4,8, NOTE_C4,2,

  NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,//repeats from 23
  NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
  NOTE_D5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_G5,-8, NOTE_F5,-8,
  
  NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2, //26
  NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,
  NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
  NOTE_B4,8, NOTE_F5,4, NOTE_F5,8, NOTE_F5,-8, NOTE_E5,-8, NOTE_D5,-8,
  NOTE_C5,8, NOTE_E4,4, NOTE_E4,8, NOTE_C4,2,
  NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,8, NOTE_E5,8,
  REST,1,

  NOTE_C5,8, NOTE_C5,4, NOTE_C5,8, REST,8, NOTE_C5,8, NOTE_D5,4, //33
  NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2,
  NOTE_E5,8, NOTE_E5,8, REST,8, NOTE_E5,8, REST,8, NOTE_C5,8, NOTE_E5,4,
  NOTE_G5,4, REST,4, NOTE_G4,4, REST,4, 
  NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,
  NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
  NOTE_D5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_A5,-8, NOTE_G5,-8, NOTE_F5,-8,
  
  NOTE_E5,8, NOTE_C5,4, NOTE_A4,8, NOTE_G4,2, //40
  NOTE_E5,8, NOTE_C5,4, NOTE_G4,8, REST,4, NOTE_GS4,4,
  NOTE_A4,8, NOTE_F5,4, NOTE_F5,8, NOTE_A4,2,
  NOTE_B4,8, NOTE_F5,4, NOTE_F5,8, NOTE_F5,-8, NOTE_E5,-8, NOTE_D5,-8,
  NOTE_C5,8, NOTE_E4,4, NOTE_E4,8, NOTE_C4,2,
  
  //game over sound
  NOTE_C5,-4, NOTE_G4,-4, NOTE_E4,4, //45
  NOTE_A4,-8, NOTE_B4,-8, NOTE_A4,-8, NOTE_GS4,-8, NOTE_AS4,-8, NOTE_GS4,-8,
  NOTE_G4,8, NOTE_D4,8, NOTE_E4,-2,  

};

  // sizeof gives the number of bytes, each int value is composed of two bytes
  // (16 bits) there are two values per note (pitch and duration), so for each
  // note there are four bytes
  int notes = sizeof(melody) / sizeof(melody[0]) / 2;

  // this calculates the duration of a whole note in ms
  int wholenote = (60000 * 4) / tempo;

  int divider = 0, noteDuration = 0;
  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    // tone(buzzer, melody[thisNote], noteDuration * 0.9);
    ledcWriteTone(channel_buzzer, melody[thisNote]);

    // Wait for the specief duration before playing the next note.
    //delay(noteDuration);
    vTaskDelay(noteDuration / portTICK_PERIOD_MS);
    // stop the waveform generation before the next note.
    // noTone(buzzer);
    ledcWriteTone(channel_buzzer, 0);
  }
}
/* #endregion */

/* #region  Musica do Starwars  */
//Toca a musica do StarWars no buzzer
void buzzer_starwars(void* arg) {
  int tempo = 108;
  int melody[] = {

      // Dart Vader theme (Imperial March) - Star wars
      // Score available at https://musescore.com/user/202909/scores/1141521
      // The tenor saxophone part was used

      NOTE_AS4, 8,  NOTE_AS4, 8,  NOTE_AS4, 8, // 1
      NOTE_F5,  2,  NOTE_C6,  2,  NOTE_AS5, 8,  NOTE_A5,  8,  NOTE_G5,  8,
      NOTE_F6,  2,  NOTE_C6,  4,  NOTE_AS5, 8,  NOTE_A5,  8,  NOTE_G5,  8,
      NOTE_F6,  2,  NOTE_C6,  4,  NOTE_AS5, 8,  NOTE_A5,  8,  NOTE_AS5, 8,
      NOTE_G5,  2,  NOTE_C5,  8,  NOTE_C5,  8,  NOTE_C5,  8,  NOTE_F5,  2,
      NOTE_C6,  2,  NOTE_AS5, 8,  NOTE_A5,  8,  NOTE_G5,  8,  NOTE_F6,  2,
      NOTE_C6,  4,

      NOTE_AS5, 8,  NOTE_A5,  8,  NOTE_G5,  8,  NOTE_F6,  2,  NOTE_C6,  4, // 8
      NOTE_AS5, 8,  NOTE_A5,  8,  NOTE_AS5, 8,  NOTE_G5,  2,  NOTE_C5,  -8,
      NOTE_C5,  16, NOTE_D5,  -4, NOTE_D5,  8,  NOTE_AS5, 8,  NOTE_A5,  8,
      NOTE_G5,  8,  NOTE_F5,  8,  NOTE_F5,  8,  NOTE_G5,  8,  NOTE_A5,  8,
      NOTE_G5,  4,  NOTE_D5,  8,  NOTE_E5,  4,  NOTE_C5,  -8, NOTE_C5,  16,
      NOTE_D5,  -4, NOTE_D5,  8,  NOTE_AS5, 8,  NOTE_A5,  8,  NOTE_G5,  8,
      NOTE_F5,  8,

      NOTE_C6,  -8, NOTE_G5,  16, NOTE_G5,  2,  REST,     8,  NOTE_C5,  8, // 13
      NOTE_D5,  -4, NOTE_D5,  8,  NOTE_AS5, 8,  NOTE_A5,  8,  NOTE_G5,  8,
      NOTE_F5,  8,  NOTE_F5,  8,  NOTE_G5,  8,  NOTE_A5,  8,  NOTE_G5,  4,
      NOTE_D5,  8,  NOTE_E5,  4,  NOTE_C6,  -8, NOTE_C6,  16, NOTE_F6,  4,
      NOTE_DS6, 8,  NOTE_CS6, 4,  NOTE_C6,  8,  NOTE_AS5, 4,  NOTE_GS5, 8,
      NOTE_G5,  4,  NOTE_F5,  8,  NOTE_C6,  1

  };
  // sizeof gives the number of bytes, each int value is composed of two bytes
  // (16 bits) there are two values per note (pitch and duration), so for each
  // note there are four bytes
  int notes = sizeof(melody) / sizeof(melody[0]) / 2;

  // this calculates the duration of a whole note in ms
  int wholenote = (60000 * 4) / tempo;

  int divider = 0, noteDuration = 0;

  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    // tone(buzzer, melody[thisNote], noteDuration * 0.9);
    ledcWriteTone(channel_buzzer, melody[thisNote]);

    // Wait for the specief duration before playing the next note.
    //delay(noteDuration);
    vTaskDelay(noteDuration / portTICK_PERIOD_MS);
    // stop the waveform generation before the next note.
    // noTone(buzzer);
    ledcWriteTone(channel_buzzer, 0);
  }
}
/* #endregion */

/* #region  Musica do Pacman */
//Toca a musica do Pacman no buzzer
void buzzer_pacman(void* arg) {
  int tempo = 105;
  int melody[] = {

      // Pacman
      // Score available at https://musescore.com/user/85429/scores/107109
      NOTE_B4,  16,  NOTE_B5,  16,  NOTE_FS5, 16, NOTE_DS5, 16, // 1
      NOTE_B5,  32,  NOTE_FS5, -16, NOTE_DS5, 8,  NOTE_C5,  16,
      NOTE_C6,  16,  NOTE_G6,  16,  NOTE_E6,  16, NOTE_C6,  32,
      NOTE_G6,  -16, NOTE_E6,  8,

      NOTE_B4,  16,  NOTE_B5,  16,  NOTE_FS5, 16, NOTE_DS5, 16,
      NOTE_B5,  32, // 2
      NOTE_FS5, -16, NOTE_DS5, 8,   NOTE_DS5, 32, NOTE_E5,  32,
      NOTE_F5,  32,  NOTE_F5,  32,  NOTE_FS5, 32, NOTE_G5,  32,
      NOTE_G5,  32,  NOTE_GS5, 32,  NOTE_A5,  16, NOTE_B5,  8};

  // sizeof gives the number of bytes, each int value is composed of two bytes
  // (16 bits) there are two values per note (pitch and duration), so for each
  // note there are four bytes
  int notes = sizeof(melody) / sizeof(melody[0]) / 2;

  // this calculates the duration of a whole note in ms
  int wholenote = (60000 * 4) / tempo;

  int divider = 0, noteDuration = 0;

  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    // tone(buzzer, melody[thisNote], noteDuration * 0.9);
    ledcWriteTone(channel_buzzer, melody[thisNote]);

    // Wait for the specief duration before playing the next note.
    //delay(noteDuration);
    vTaskDelay(noteDuration / portTICK_PERIOD_MS);

    // stop the waveform generation before the next note.
    // noTone(buzzer);
    ledcWriteTone(channel_buzzer, 0);
  }
}
/* #endregion */

/* #region  Musica do Tetris */
//Toca a musica do Tetris no buzzer
void buzzer_tetris(void* arg) {
  int tempo = 144;
  int melody[] = {

      // Based on the arrangement at https://www.flutetunes.com/tunes.php?id=192

      NOTE_E5, 4,  NOTE_B4,  8, NOTE_C5, 8,  NOTE_D5, 4, NOTE_C5, 8,
      NOTE_B4, 8,  NOTE_A4,  4, NOTE_A4, 8,  NOTE_C5, 8, NOTE_E5, 4,
      NOTE_D5, 8,  NOTE_C5,  8, NOTE_B4, -4, NOTE_C5, 8, NOTE_D5, 4,
      NOTE_E5, 4,  NOTE_C5,  4, NOTE_A4, 4,  NOTE_A4, 8, NOTE_A4, 4,
      NOTE_B4, 8,  NOTE_C5,  8,

      NOTE_D5, -4, NOTE_F5,  8, NOTE_A5, 4,  NOTE_G5, 8, NOTE_F5, 8,
      NOTE_E5, -4, NOTE_C5,  8, NOTE_E5, 4,  NOTE_D5, 8, NOTE_C5, 8,
      NOTE_B4, 4,  NOTE_B4,  8, NOTE_C5, 8,  NOTE_D5, 4, NOTE_E5, 4,
      NOTE_C5, 4,  NOTE_A4,  4, NOTE_A4, 4,  REST,    4,

      NOTE_E5, 4,  NOTE_B4,  8, NOTE_C5, 8,  NOTE_D5, 4, NOTE_C5, 8,
      NOTE_B4, 8,  NOTE_A4,  4, NOTE_A4, 8,  NOTE_C5, 8, NOTE_E5, 4,
      NOTE_D5, 8,  NOTE_C5,  8, NOTE_B4, -4, NOTE_C5, 8, NOTE_D5, 4,
      NOTE_E5, 4,  NOTE_C5,  4, NOTE_A4, 4,  NOTE_A4, 8, NOTE_A4, 4,
      NOTE_B4, 8,  NOTE_C5,  8,

      NOTE_D5, -4, NOTE_F5,  8, NOTE_A5, 4,  NOTE_G5, 8, NOTE_F5, 8,
      NOTE_E5, -4, NOTE_C5,  8, NOTE_E5, 4,  NOTE_D5, 8, NOTE_C5, 8,
      NOTE_B4, 4,  NOTE_B4,  8, NOTE_C5, 8,  NOTE_D5, 4, NOTE_E5, 4,
      NOTE_C5, 4,  NOTE_A4,  4, NOTE_A4, 4,  REST,    4,

      NOTE_E5, 2,  NOTE_C5,  2, NOTE_D5, 2,  NOTE_B4, 2, NOTE_C5, 2,
      NOTE_A4, 2,  NOTE_GS4, 2, NOTE_B4, 4,  REST,    8, NOTE_E5, 2,
      NOTE_C5, 2,  NOTE_D5,  2, NOTE_B4, 2,  NOTE_C5, 4, NOTE_E5, 4,
      NOTE_A5, 2,  NOTE_GS5, 2,

  };

  // sizeof gives the number of bytes, each int value is composed of two bytes
  // (16 bits) there are two values per note (pitch and duration), so for each
  // note there are four bytes
  int notes = sizeof(melody) / sizeof(melody[0]) / 2;

  // this calculates the duration of a whole note in ms
  int wholenote = (60000 * 4) / tempo;

  int divider = 0, noteDuration = 0;

  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    // tone(buzzer, melody[thisNote], noteDuration * 0.9);
    ledcWriteTone(channel_buzzer, melody[thisNote]);

    // Wait for the specief duration before playing the next note.
    //delay(noteDuration);
    vTaskDelay(noteDuration / portTICK_PERIOD_MS);

    // stop the waveform generation before the next note.
    // noTone(buzzer);
    ledcWriteTone(channel_buzzer, 0);
  }
}
/* #endregion */


//https://stackoverflow.com/questions/9410/how-do-you-pass-a-function-as-a-parameter-in-c

// #include <stdio.h>

// void print();
// void execute(void());

// int main()
// {
//     execute(print); // sends address of print
//     return 0;
// }

// void print()
// {
//     printf("Hello!");
// }

// void execute(void f()) // receive address of print
// {
//     f();
// }