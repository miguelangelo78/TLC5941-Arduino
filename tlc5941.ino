
class TLC5941 {
  public:
  
  /* Channel sizes and counts: */
  #define TLC5941_COUNT 1
  #define GS_CHANNEL_LEN 12 /* Each GS channel is 12 bits */
  #define DC_CHANNEL_LEN 6 /* Each DC channel is 6 bits */
  #define GS_SIZE 192 /* Total size in bits of Grayscale PWM control */
  #define DC_SIZE 96 /* Total size in bits of Dot Correction */

  /* Pin definitions: */
  #define SIN     0 /* Serial input */
  #define SCLK    1 /* Serial clock */
  #define XLAT    2 /* Serial data latch into the registers */
  #define BLANK   3 /* Turns off LEDS and signals the start of the Grayscale cycle */
  #define GSCLK   4 /* Grayscale clock */
  #define MODE    5 /* 0 = In Dot Correction Mode | 1 = In Grayscale Mode */
  #define XERR    6 /* XERR signals LED overheating */
  
  /* Convenience macros: */
  #define outputState(port, pin) ((port) & (1 << (pin))) /* Reads state of pin */
  #define PULSE(pin) { digitalWrite(pin, HIGH); digitalWrite(pin, LOW); } /* Send a pulse to a pin */
  #define SET_BLANK(blank) digitalWrite(BLANK, blank);
  #define SET_MODE(md) digitalWrite(MODE, md);

  enum MD {
    MD_GS, /* Grayscale mode */
    MD_DC  /* Dot Correction Mode */
  };
    
  TLC5941() {}

  void init(void) {
    for(int i = 0; i <= 5; i++) { 
      pinMode(i, OUTPUT);
      digitalWrite(i, LOW);
    }
    
    pinMode(XERR, INPUT);

    SET_MODE(MD_DC);
    SET_BLANK(1);

    /* Send default dot correction data: */
    sendDot();
  }

  inline uint8_t getXERR() {
    return digitalRead(XERR);  
  }

  /* Sets dot correction value (from 0 to 64) for a certain channel */
  inline void setDot(uint8_t channel, uint8_t dot_val) {
    int startOff = DC_SIZE - (channel * DC_CHANNEL_LEN) - DC_CHANNEL_LEN;
    uint8_t * new_dc_dat = byte_to_binary(dot_val, DC_CHANNEL_LEN);
    memcpy(gsData + startOff, new_dc_dat, DC_CHANNEL_LEN);
  }

  /* Sets brightness value (from 0 to 4096) for a certain channel */
  inline void setChannel(uint8_t channel, uint16_t brightness) {
    int startOff = GS_SIZE - (channel * GS_CHANNEL_LEN) - GS_CHANNEL_LEN;
    uint8_t * new_gs_dat = byte_to_binary(brightness, GS_CHANNEL_LEN);
    memcpy(gsData + startOff, new_gs_dat, GS_CHANNEL_LEN);
  }

  /* Updates the chip by outputting the DC and GS data */
  inline void update() {
    sendDot(); 
    sendGS(); 
  }
  
  private:
  /* There is no need for hardcoding all the 1's and 0's here. This is here for reference purpose */

  /* Dot correction data: */
  uint8_t dcData[TLC5941_COUNT * DC_SIZE] = {
    // MSB         LSB
    1, 1, 1, 1, 1, 1,  // Channel 15
    1, 1, 1, 1, 1, 1,  // Channel 14
    1, 1, 1, 1, 1, 1,  // Channel 13
    1, 1, 1, 1, 1, 1,  // Channel 12
    1, 1, 1, 1, 1, 1,  // Channel 11
    1, 1, 1, 1, 1, 1,  // Channel 10
    1, 1, 1, 1, 1, 1,  // Channel 9
    1, 1, 1, 1, 1, 1,  // Channel 8
    1, 1, 1, 1, 1, 1,  // Channel 7
    1, 1, 1, 1, 1, 1,  // Channel 6
    1, 1, 1, 1, 1, 1,  // Channel 5
    1, 1, 1, 1, 1, 1,  // Channel 4
    1, 1, 1, 1, 1, 1,  // Channel 3
    1, 1, 1, 1, 1, 1,  // Channel 2
    1, 1, 1, 1, 1, 1,  // Channel 1
    1, 1, 1, 1, 1, 1,  // Channel 0
  };
  
  /* Grayscale data */
  uint8_t gsData[TLC5941_COUNT * GS_SIZE] = {
    // MSB                           LSB
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 15
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 14
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 13
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 12
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 11
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 10
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 7
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 1
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Channel 0
  };

  /* Send dot correction data: */
  inline void sendDot(void) {
    SET_MODE(MD_DC);
    
    for(uint8_t ctr = 0;;) {
      if(ctr > TLC5941_COUNT * DC_SIZE - 1) {
        PULSE(XLAT);
        break;
      } else {
        digitalWrite(SIN, dcData[ctr++] ? HIGH : LOW);
        PULSE(SCLK);
      }
    }
  }

  /* Send grayscale data: */
  inline void sendGS(void) {
    uint8_t first_cycle_fl = 0;
    uint8_t data_ctr = 0;

    if(outputState(PORTD, MODE)) {
      SET_MODE(MD_GS);
      first_cycle_fl = 1;  
    }
    
    SET_BLANK(LOW);
    for(uint16_t gsclk_ctr = 0; gsclk_ctr <= 4095; gsclk_ctr++) {
      if(!(data_ctr > TLC5941_COUNT * GS_SIZE - 1)) {
        digitalWrite(SIN, gsData[data_ctr++] ? HIGH : LOW);
        PULSE(SCLK);  
      }      
      PULSE(GSCLK);
    }
    /* End of GS cycle */
    SET_BLANK(HIGH);
    PULSE(XLAT);
    if(first_cycle_fl)
      PULSE(SCLK);  
  }
  
  inline uint8_t * byte_to_binary(int x, int bitcount) {
    static uint8_t b[GS_CHANNEL_LEN]; /* I picked the biggest channel */
    for (int z = (1 << bitcount), i = 0; z > 0; z >>= 1)
      b[i++] = (x & z) == z;
    return b;
  }
};

TLC5941 tlc;

void setup() {
  tlc.init();
  tlc.setChannel(0, 0x20);
  tlc.setChannel(2, 0x80);
}
  
void loop() {
  tlc.update();
}
