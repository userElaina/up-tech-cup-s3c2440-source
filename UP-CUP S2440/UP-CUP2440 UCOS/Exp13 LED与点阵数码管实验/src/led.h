/*          led_test.h    */
// led number
#define lednum1con *(volatile unsigned char *)0x08000110 
#define lednum2con *(volatile unsigned char *)0x08000112
// led array 
#define led1con *(volatile unsigned char *)0x08000100 
#define led2con *(volatile unsigned char *)0x08000102
#define led3con *(volatile unsigned char *)0x08000104
#define led4con *(volatile unsigned char *)0x08000106
#define led5con *(volatile unsigned char *)0x08000108
#define led6con *(volatile unsigned char *)0x0800010a
#define led7con *(volatile unsigned char *)0x0800010c
#define led8con *(volatile unsigned char *)0x0800010e
void set_lednum(void);
void set_1_led(void);
void set_2_led(void);
void set_3_led(void);
void set_4_led(void);
void test_led(void);
	