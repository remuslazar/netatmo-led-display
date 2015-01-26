/* Setup you hardware mapping (ports/pins) here

   for Leonardo/Yun see also:
   http://arduino.cc/en/pmwiki.php?n=Hacking/PinMapping32u4 and
   ArduinoPinMapping32u4.md
*/


// setup the mapping used in the hardware schematic:
// A, B,C,D, L,S, EN,R1,R2,G1,G2
// 9,10,8,7, 1,0,  6, 5, 3, 4, 2


#define LED_HUB08_A_PORT PORTB
#define LED_HUB08_A_MASK _BV(5)
#define LED_HUB08_B_PORT PORTB
#define LED_HUB08_B_MASK _BV(6)
#define LED_HUB08_C_PORT PORTB
#define LED_HUB08_C_MASK _BV(4)
#define LED_HUB08_D_PORT PORTE
#define LED_HUB08_D_MASK _BV(6)

#define LED_HUB08_L_PORT PORTD
#define LED_HUB08_L_MASK _BV(3)
#define LED_HUB08_S_PORT PORTD
#define LED_HUB08_S_MASK _BV(2)
#define LED_HUB08_EN_PORT PORTD
#define LED_HUB08_EN_MASK _BV(7)

#define LED_HUB08_R1_PORT PORTC
#define LED_HUB08_R1_MASK _BV(6)
#define LED_HUB08_R2_PORT PORTD
#define LED_HUB08_R2_MASK _BV(0)

#define LED_HUB08_G1_PORT PORTD
#define LED_HUB08_G1_MASK _BV(4)
#define LED_HUB08_G2_PORT PORTD
#define LED_HUB08_G2_MASK _BV(1)

#define LED_HUB08_EN_PIN 6