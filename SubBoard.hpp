#pragma once

#define MUX_A0 12
#define MUX_A1 14
#define MUX_A2 16
#define MUX_D 0

#define CHA_A 0
#define CHA_B 1
#define CHA_C 2
#define MUX_X 9

#define SPEAKER 13

#define PRESS_DELAY 200
#define BLINK_DELAY 500

#define FOG 3
#define STROBES 4
#define BL_L 5
#define BL_R 6
#define HBEAM 7

#define BTN_B 3
#define BTN_C 2
#define BTN_D 5
#define BTN_E 1
#define BTN_F 4

class SubBoard{
  private:
    bool memory[8] = {false,false,false,false,false,false,false,false};
    bool stateInUse = false;
    unsigned int curChannel = -1;
    
    unsigned long previousMillis = 0;
    bool leftBlinkerIsOn = false;
    bool leftBlinkerState = false;
    bool rightBlinkerIsOn = false;
    bool rightBlinkerState = false;
    
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
      for(int i=0; i<8; i++){
        bool curBits[3] = {(i & 1)!=0, (i & 2) != 0, (i & 4) != 0};
        digitalWrite(MUX_A0, curBits[0] ? HIGH : LOW);
        digitalWrite(MUX_A1, curBits[1] ? HIGH : LOW);
        digitalWrite(MUX_A2, curBits[2] ? HIGH : LOW);
        digitalWrite(MUX_D, false);
        memory[i] = false;
      }
      DEBUG_ESP_PORT.println("Cleared U_74HC259D memory");
    }
  public:
    SubBoard(){
      DEBUG_ESP_PORT.println("Called SubBoard constructor");
      
      pinMode(MUX_X, OUTPUT);
      DEBUG_ESP_PORT.println("Set U_74HC4051D output");
      digitalWrite(MUX_X, LOW);
      
      pinMode(MUX_A0, OUTPUT);
      pinMode(MUX_A1, OUTPUT);
      pinMode(MUX_A2, OUTPUT);
      pinMode(MUX_D, OUTPUT);
      DEBUG_ESP_PORT.println("Set U_74HC259D outputs");
      clearMemory();
      parkState();
      
    }
    ~SubBoard() {
      DEBUG_ESP_PORT.println("Called SubBoard destructor");
    };

    void blinkingLoop(){
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= BLINK_DELAY) {
        previousMillis = currentMillis;
        
        leftBlinkerState = leftBlinkerIsOn ? !leftBlinkerState : false;
        
        rightBlinkerState = rightBlinkerIsOn ? !rightBlinkerState : false;
        if (leftBlinkerIsOn && rightBlinkerIsOn) leftBlinkerState = rightBlinkerState;

        leftBlinkerState ? enableMod(BL_L) : disableMod(BL_L);
        rightBlinkerState ? enableMod(BL_R) : disableMod(BL_R);
      }
    }

    void toggleBlinker(bool left, bool turnOn){
      if(left) leftBlinkerIsOn = turnOn;
      else rightBlinkerIsOn = turnOn;
    }
    
    void enableMod(unsigned int channel){
      setMemory(channel, true);
    }

    void disableMod(unsigned int channel){
      setMemory(channel, false);
    }

    void pressButton(unsigned int channel){
      channel = channel % 8;
      bool newBits[3] = {(channel    & 1)!=0, (channel    & 2) != 0, (channel    & 4) != 0};

      while(stateInUse){
        DEBUG_ESP_PORT.printf("Cannot change state. Already in use");
        delay(100);
      }

      stateInUse = true;
      setMemory(CHA_A, newBits[0]);
      setMemory(CHA_B, newBits[1]);
      setMemory(CHA_C, newBits[2]);
      digitalWrite(MUX_X, HIGH);
      static unsigned long timeBefore = millis();
      DEBUG_ESP_PORT.printf("Pressed button U_74HC4051D channel %d(%d)\n", channel, timeBefore);

      delay(PRESS_DELAY);
      
      digitalWrite(MUX_X, LOW);
      parkState();
      stateInUse = false;
      static unsigned long timeAfter = millis();
      DEBUG_ESP_PORT.printf("Released button U_74HC4051D channel %d(%d, elapsed:%d)\n", channel ,timeAfter, timeAfter-timeBefore);
    }

    void parkState(){
      unsigned int channel = 7;
      bool newBits[3] = {(channel    & 1)!=0, (channel    & 2) != 0, (channel    & 4) != 0};
      digitalWrite(MUX_X, LOW);
      setMemory(CHA_A, newBits[0]);
      setMemory(CHA_B, newBits[1]);
      setMemory(CHA_C, newBits[2]);
    }

    void melodyBeep(JsonArray melody, JsonArray durations, JsonArray pauses){
      int melody_size = melody.size();
      if(melody_size!=durations.size() ||melody_size!=pauses.size()) return;
      
      for (int thisNote = 0; thisNote < melody_size; thisNote++) {
        int note = melody[thisNote];
        int duration = durations[thisNote];
        int pause = pauses[thisNote];
        tone(SPEAKER, note, duration);
        delay(pause);
        noTone(SPEAKER);
      }
    }
    void beep(){
      String json = "{\"melody\":[262,196,196,220,196,0,247,262],\"durations\":[250,125,125,250,250,250,250,250],\"pauses\":[325,162,162,325,325,325,325,325]}";
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, json);
      doc.shrinkToFit();
      melodyBeep(doc["melody"], doc["durations"], doc["pauses"]);
    }
};
