// ADC Channel Command Options
#define ADC_MODE_MSK  0x0F
#define ADC_MODE_A0   0x00
#define ADC_MODE_A1   0x01
#define ADC_MODE_A2   0x02
#define ADC_MODE_A3   0x03
#define ADC_MODE_A4   0x04
#define ADC_MODE_A5   0x05
#define ADC_MODE_A6   0x06
#define ADC_MODE_A7   0x07
#define ADC_MODE_A8   0x08
#define ADC_MODE_A9   0x09
#define ADC_MODE_A10  0x0A
#define ADC_MODE_A11  0x0B

// ADC channel Configurations
#define ADC_A0_CH   (0x01<<7)
#define ADC_A1_CH   (0x01<<6)
#define ADC_A2_CH   (0x01<<5)
#define ADC_A3_CH   (0x01<<4)
#define ADC_A4_CH   (0x01<<3)
#define ADC_A5_CH   (0x01<<2)
#define ADC_A6_CH   (0x01<<1)
#define ADC_A7_CH   (0x01<<0)
#define ADC_A8_CH   (0x01<<10)
#define ADC_A9_CH   (0x01<<11)
#define ADC_A10_CH  (0x01<<12)
#define ADC_A11_CH  (0x01<<13)

//Sampling Rate Command Options
#define _CLEAR_PRESCALER(X) ( ((X)&(0xffff00ff)) | (0x00000100))
#define ADC_MODE_666K   0x0C
#define ADC_MODE_280K   0x0D
#define ADC_MODE_125K   0x0E
#define ADC_MODE_60K    0x0F