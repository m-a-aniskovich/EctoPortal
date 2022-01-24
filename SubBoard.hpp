#pragma once

#define MUX_A0 12
#define MUX_A1 14
#define MUX_A2 16
#define MUX_D 10
#define MUX_X 0

#define CHA_A 0
#define CHA_B 1
#define CHA_C 2

#define SPEAKER 13

#define EMULATE_DELAY 1000

class SubBoard{
  private:
    bool memory[8] = {true,true,true,true,true,true,true,true};
    unsigned int curChannel = -1;
    void setMemory(unsigned int channel, bool value){
      channel = channel % 8;
      if(memory[channel] == value) return;

      bool curBits[3] = {(curChannel & 1)!=0, (curChannel & 2) != 0, (curChannel & 4) != 0};
      bool newBits[3] = {(channel    & 1)!=0, (channel    & 2) != 0, (channel    & 4) != 0};

      if(curBits[0] != newBits[0] || curChannel < 0){
        unsigned int midChannel = newBits[0]*1 + curBits[1]*2 + curBits[2]*4;
        digitalWrite(MUX_A0, newBits[0] ? HIGH : LOW);
        digitalWrite(MUX_D, memory[midChannel] ? HIGH : LOW);
      }
      if(curBits[1] != newBits[1] || curChannel < 0){
        unsigned int midChannel = newBits[0]*1 + newBits[1]*2 + curBits[2]*4;
        digitalWrite(MUX_A1, newBits[1] ? HIGH : LOW);
        digitalWrite(MUX_D, memory[midChannel] ? HIGH : LOW);
      }
      if(curBits[2] != newBits[2] || curChannel < 0){
        digitalWrite(MUX_A2, newBits[2] ? HIGH : LOW);
      }
      digitalWrite(MUX_D, value ? HIGH : LOW);

      curChannel = channel;
      memory[channel] = value;

      DEBUG_ESP_PORT.printf("U_74HC259D channel %d(%d%d%d) is set to %s\n", channel, newBits[0], newBits[1], newBits[2], value ? "HIGH" : "LOW");
    }

    void clearMemory(){
      for(int i=0; i<8; i++) setMemory(i,false);
      DEBUG_ESP_PORT.println("Cleared U_74HC259D memory");
    }
    
    void setState(unsigned int channel, bool value){
      channel = channel % 8;
      bool newBits[3] = {(channel    & 1)!=0, (channel    & 2) != 0, (channel    & 4) != 0};

      digitalWrite(MUX_X, LOW);
      setMemory(CHA_A, newBits[0]);
      setMemory(CHA_B, newBits[1]);
      setMemory(CHA_C, newBits[2]);
      digitalWrite(MUX_X, value ? HIGH : LOW);
      
      DEBUG_ESP_PORT.printf("U_74HC4051D channel %d(%d%d%d) is set to %s\n", channel, newBits[0], newBits[1], newBits[2], value ? "HIGH" : "LOW");
    }

    void parkState(){
      digitalWrite(MUX_X, LOW);
      setMemory(CHA_A,true);
      setMemory(CHA_B,true);
      setMemory(CHA_C,true);
    }
  public:
    struct Mods{
      unsigned int FOG     = 3;
      unsigned int STROBES = 4;
      unsigned int BL_L    = 5;
      unsigned int BL_R    = 6;
      unsigned int HBEAM   = 7;
    };
    struct Buttons{
      unsigned int B = 2;
      unsigned int C = 1;
      unsigned int D = 0;
      unsigned int E = 3;
      unsigned int F = 4;
    };
    SubBoard(){
      DEBUG_ESP_PORT.println("Called SubBoard constructor");
      pinMode(MUX_A0, OUTPUT);
      pinMode(MUX_A1, OUTPUT);
      pinMode(MUX_A2, OUTPUT);
      pinMode(MUX_D, OUTPUT);
      DEBUG_ESP_PORT.println("Set U_74HC259D outputs");
      clearMemory();

      pinMode(MUX_X, OUTPUT);
      DEBUG_ESP_PORT.println("Set U_74HC4051D output");
      
    }
    ~SubBoard() {
      DEBUG_ESP_PORT.println("Called SubBoard destructor");
    };

    void enableMod(unsigned int channel){
      setMemory(channel, true);
    }

    void disableMod(unsigned int channel){
      setMemory(channel, false);
    }

    void emulateButton(unsigned int channel){
      DEBUG_ESP_PORT.printf("Emulating button press U_74HC4051D channel %d\n", channel);
      channel = channel % 8;
      setState(channel,true);
      delay(EMULATE_DELAY); //Thats okay. While this function is delayed ESP serves WiFi clients
      parkState();
    }

    void pressButton(unsigned int channel){
      DEBUG_ESP_PORT.printf("Pressing button U_74HC4051D channel %d\n", channel);
      channel = channel % 8;
      setState(channel,true);
    }

    void releaseButton(unsigned int channel){
      DEBUG_ESP_PORT.printf("Releasing button U_74HC4051D channel %d\n", channel);
      channel = channel % 8;
      parkState();
    }

    #define NOTE_B0  31
    #define NOTE_C1  33
    #define NOTE_CS1 35
    #define NOTE_D1  37
    #define NOTE_DS1 39
    #define NOTE_E1  41
    #define NOTE_F1  44
    #define NOTE_FS1 46
    #define NOTE_G1  49
    #define NOTE_GS1 52
    #define NOTE_A1  55
    #define NOTE_AS1 58
    #define NOTE_B1  62
    #define NOTE_C2  65
    #define NOTE_CS2 69
    #define NOTE_D2  73
    #define NOTE_DS2 78
    #define NOTE_E2  82
    #define NOTE_F2  87
    #define NOTE_FS2 93
    #define NOTE_G2  98
    #define NOTE_GS2 104
    #define NOTE_A2  110
    #define NOTE_AS2 117
    #define NOTE_B2  123
    #define NOTE_C3  131
    #define NOTE_CS3 139
    #define NOTE_D3  147
    #define NOTE_DS3 156
    #define NOTE_E3  165
    #define NOTE_F3  175
    #define NOTE_FS3 185
    #define NOTE_G3  196
    #define NOTE_GS3 208
    #define NOTE_A3  220
    #define NOTE_AS3 233
    #define NOTE_B3  247
    #define NOTE_C4  262
    #define NOTE_CS4 277
    #define NOTE_D4  294
    #define NOTE_DS4 311
    #define NOTE_E4  330
    #define NOTE_F4  349
    #define NOTE_FS4 370
    #define NOTE_G4  392
    #define NOTE_GS4 415
    #define NOTE_A4  440
    #define NOTE_AS4 466
    #define NOTE_B4  494
    #define NOTE_C5  523
    #define NOTE_CS5 554
    #define NOTE_D5  587
    #define NOTE_DS5 622
    #define NOTE_E5  659
    #define NOTE_F5  698
    #define NOTE_FS5 740
    #define NOTE_G5  784
    #define NOTE_GS5 831
    #define NOTE_A5  880
    #define NOTE_AS5 932
    #define NOTE_B5  988
    #define NOTE_C6  1047
    #define NOTE_CS6 1109
    #define NOTE_D6  1175
    #define NOTE_DS6 1245
    #define NOTE_E6  1319
    #define NOTE_F6  1397
    #define NOTE_FS6 1480
    #define NOTE_G6  1568
    #define NOTE_GS6 1661
    #define NOTE_A6  1760
    #define NOTE_AS6 1865
    #define NOTE_B6  1976
    #define NOTE_C7  2093
    #define NOTE_CS7 2217
    #define NOTE_D7  2349
    #define NOTE_DS7 2489
    #define NOTE_E7  2637
    #define NOTE_F7  2794
    #define NOTE_FS7 2960
    #define NOTE_G7  3136
    #define NOTE_GS7 3322
    #define NOTE_A7  3520
    #define NOTE_AS7 3729
    #define NOTE_B7  3951
    #define NOTE_C8  4186
    #define NOTE_CS8 4435
    #define NOTE_D8  4699
    #define NOTE_DS8 4978
    void beep(){
      int melody[] = {
        NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
      };
      
      // note durations: 4 = quarter note, 8 = eighth note, etc.:
      int noteDurations[] = {
        4, 8, 8, 4, 4, 4, 4, 4
      };
      
      for (int thisNote = 0; thisNote < 8; thisNote++) {
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(SPEAKER, melody[thisNote], noteDuration);
    
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        // stop the tone playing:
        noTone(SPEAKER);
      }
    }
};
