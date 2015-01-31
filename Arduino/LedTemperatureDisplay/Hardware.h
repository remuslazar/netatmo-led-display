/* Setup you hardware mapping (ports/pins) here

   for Leonardo/Yun see also:
   http://arduino.cc/en/pmwiki.php?n=Hacking/PinMapping32u4 and
   ArduinoPinMapping32u4.md
*/

// setup the mapping used in the hardware schematic:
// A, B,C,D, L,S, EN,R1,R2,G1,G2
// 9,10,8,7, 1,5,  6, 0, 3, 4, 2

/*
  Very important notice
  =====================

  The code (ISR) is heavily optimized for speed, so there are 3
  constraints:

  1) The port used for the S(CLK) signal cannot be used for the
  R1,R2,G1,G2 signals! Make sure that you use different ports.

  2) All Data signals have to be on the same data port (e.g. PORTD)

  3) EN pin has to be PD7 (Arduino digital pin 6 on Leonardo)
*/

#define LED_HUB08_A_PORT PORTB
#define LED_HUB08_A_BIT 5
#define LED_HUB08_A_DDR DDRB

#define LED_HUB08_B_PORT PORTB
#define LED_HUB08_B_BIT 6
#define LED_HUB08_B_DDR DDRB

#define LED_HUB08_C_PORT PORTB
#define LED_HUB08_C_BIT 4
#define LED_HUB08_C_DDR DDRB

#define LED_HUB08_D_PORT PORTE
#define LED_HUB08_D_BIT 6
#define LED_HUB08_D_DDR DDRE


#define LED_HUB08_L_PORT PORTD
#define LED_HUB08_L_BIT 3
#define LED_HUB08_L_DDR DDRD

#define LED_HUB08_S_PORT PORTC
#define LED_HUB08_S_BIT 6
#define LED_HUB08_S_DDR DDRC

#define LED_HUB08_DATA_PORT PORTD
#define LED_HUB08_DATA_DDR DDRD

#define LED_HUB08_R1_BIT 2
#define LED_HUB08_R2_BIT 0
#define LED_HUB08_G1_BIT 4
#define LED_HUB08_G2_BIT 1

// don't change
#define LED_HUB08_EN_PORT PORTD
#define LED_HUB08_EN_BIT 7
#define LED_HUB08_EN_DDR DDRD

// macros for the bitmasks (don't change)
#define LED_HUB08_A_MASK _BV(LED_HUB08_A_BIT)
#define LED_HUB08_B_MASK _BV(LED_HUB08_B_BIT)
#define LED_HUB08_C_MASK _BV(LED_HUB08_C_BIT)
#define LED_HUB08_D_MASK _BV(LED_HUB08_D_BIT)
#define LED_HUB08_L_MASK _BV(LED_HUB08_L_BIT)
#define LED_HUB08_S_MASK _BV(LED_HUB08_S_BIT)
#define LED_HUB08_EN_MASK _BV(LED_HUB08_EN_BIT)
#define LED_HUB08_R1_MASK _BV(LED_HUB08_R1_BIT)
#define LED_HUB08_R2_MASK _BV(LED_HUB08_R2_BIT)
#define LED_HUB08_G1_MASK _BV(LED_HUB08_G1_BIT)
#define LED_HUB08_G2_MASK _BV(LED_HUB08_G2_BIT)
