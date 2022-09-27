#ifndef _S3C2440_ADC_H_
#define _S3C2440_ADC_H_

#define ADC_WRITE(ch, prescale)	((ch)<<16|(prescale))

#define ADC_WRITE_GETCH(data)	(((data)>>16)&0x7)
#define ADC_WRITE_GETPRE(data)	((data)&0xff)

#endif /* _S3C2440_ADC_H_ */
