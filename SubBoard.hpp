#pragma once

#define MUX_A0 12
#define MUX_A1 14
#define MUX_A2 16
#define MUX_D 10
#define MUX_X 0

#define CHA_A 0
#define CHA_B 1
#define CHA_C 2

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
      unsigned int F = 6;
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
      delay(200); //Thats okay. While this function is delayed ESP serves WiFi clients
      setState(channel,false);
    }

    void pressButton(unsigned int channel){
      DEBUG_ESP_PORT.printf("Pressing button U_74HC4051D channel %d\n", channel);
      channel = channel % 8;
      setState(channel,true);
    }

    void releaseButton(unsigned int channel){
      DEBUG_ESP_PORT.printf("Pressing button U_74HC4051D channel %d\n", channel);
      channel = channel % 8;
      setState(channel,false);
    }
};
