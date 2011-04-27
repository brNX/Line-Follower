#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include "uart.h"
#include <stdlib.h>

uint16_t lmax,lmin,lmid,rmax,rmin,rmid;

void initPWM(){

	DDRB |= (1<<PB0) | (1<<PB1);
    
	//Clear OC0A/OC0B on Compare Match when up-counting. Set OC0A/OC0B on Compare Match when down-counting.
	TCCR0A |= (1<<COM0A1) | (1<<COM0B1);
	
	//mode 1 (pwm phase correct)
	TCCR0A |= (1<<WGM00);

	//prescaler to 64 
	TCCR0B |=  (1<<CS01) | (1<<CS00);


	OCR0A = 0; // set pwm duty
	OCR0B = 0; // set pwm duty

}

void initADC() {
	ADMUX = 0; //set Vref = Vcc  ,
	//ADMUX =_BV(REFS1); //set to 1.1V internal for temperature (_BV(REFS1))

	ADCSRA = _BV(ADEN) | _BV(ADPS1)| _BV(ADPS2) ; // enable adc and set prescaler to /64  8mhz/64 = 125khz
}

uint16_t analogRead(uint8_t channel) {

	ADMUX = channel;

	//Start Single conversion
	ADCSRA |= _BV(ADSC);

	//Wait for conversion to complete
	while (ADCSRA & _BV(ADSC));

	return (ADCL | (ADCH << 8));
}

void calibrateSensors(){
	uint32_t temp=0;

	//char tempc[7];

	OCR0A=120;
	OCR0B=120;

	/*print_string("Caliration:\n");
	print_string("Middle:\n");*/

	//middle
	//wait for button push
	while (PINB & (1<<PB2)) ;
	_delay_ms(500);

	analogRead(3);
	for (int i = 0 ; i <8 ; i++)
		temp += analogRead(3);
	lmid = temp >>3;

	temp=0;
	analogRead(2);
	for (int i = 0 ; i <8 ; i++)
		temp += analogRead(2);
	rmid = temp >>3;

	/*print_string("MiddleR:");
	itoa(rmid,tempc,10);
	print_string(tempc);
	sendByte('\n');


	print_string("MiddleL:");
	itoa(lmid,tempc,10);
	print_string(tempc);
	sendByte('\n');*/


	OCR0A=0;
	OCR0B=120;

	//print_string("Right:\n");

	//right
	//wait for button push
	while (PINB & (1<<PB2)) ;
	_delay_ms(500);

	temp=0;
	analogRead(3);
	for (int i = 0 ; i <8 ; i++)
		temp += analogRead(3);
	lmax = temp >>3;

	temp=0;
	analogRead(2);
	for (int i = 0 ; i <8 ; i++)
		temp += analogRead(2);
	rmin = temp >>3;

	/*print_string("Rmin:");
	itoa(rmin,tempc,10);
	print_string(tempc);
	sendByte('\n');


	print_string("Lmax:");
	itoa(lmax,tempc,10);
	print_string(tempc);
	sendByte('\n');*/

	OCR0A=120;
	OCR0B=0;

	//print_string("Left:\n");

	//left
	//wait for button push
	while (PINB & (1<<PB2)) ;
	_delay_ms(500);

	temp=0;
	analogRead(3);
	for (int i = 0 ; i <8 ; i++)
		temp += analogRead(3);
	lmin = temp >>3;

	temp=0;
	analogRead(2);
	for (int i = 0 ; i <8 ; i++)
		temp += analogRead(2);
	rmax = temp >>3;

	/*print_string("Rmax:");
	itoa(rmax,tempc,10);
	print_string(tempc);
	sendByte('\n');


	print_string("Lmin:");
	itoa(lmin,tempc,10);
	print_string(tempc);
	sendByte('\n');*/
}

uint8_t rightSpeed(uint16_t value){

	uint16_t step = (rmax-rmid) >>2;

	if (value <= rmid)
		return 200;

	if ((value > rmid) && (value < (rmid + step)))
		return 140;

	if ((value >= (rmid+step)) && (value < (rmid + 2*step)))
		return 80;

	if ((value >= (rmid+2*step)) && (value < (rmid + 3*step)))
		return 50;

	return 0;
}

uint8_t leftSpeed(uint16_t value){
	uint16_t step = (lmax-lmid) >>2;

	if (value <= lmid)
		return 200;

	if ((value > lmid) && (value < (lmid + step)))
		return 140;

	if ((value >= (lmid+step)) && (value < (lmid + 2*step)))
		return 80;

	if ((value >= (lmid+2*step)) && (value < (lmid + 3*step)))
		return 50;

	return 0;

}

int main (){

initADC();
initPWM();

//uartInit();

//pushbutton pin
DDRB &= ~(1<<PB2);
PORTB |= (1<<PB2); //pullup
calibrateSensors();

for (;;){

	char value[7];

	for(int i=0;i<7;i++)
		value[i]=0;

	analogRead(3);
	uint32_t left = analogRead(3);
	for (int i = 0 ; i <7 ; i++)
		left += analogRead(3);
	left = left >>3;

	analogRead(2);
	uint32_t right = analogRead(2);
	for (int i = 0 ; i <7 ; i++)
		right += analogRead(2);
	right = right >>3;




	/*itoa(leftSpeed(left),value,10);
	print_string("Left:");
	print_string(value);
	sendByte('\n');

	itoa(rightSpeed(right),value,10);
	print_string("Right:");
	print_string(value);
	sendByte('\n');*/


	OCR0A=leftSpeed(left);
	OCR0B=rightSpeed(right);

//	if(right <= 330){
//			/*sendByte('L');
//			sendByte('e');
//			sendByte('f');
//			sendByte('t');
//			sendByte('\n');*/
//			OCR0A=120;
//			OCR0B=10;
//	}else if(left <= 520){
//		/*sendByte('R');
//		sendByte('i');
//		sendByte('g');
//		sendByte('h');
//		sendByte('t');
//		sendByte('\n');*/
//		OCR0A=10;
//		OCR0B=120;
//	}else{
//		/*sendByte('S');
//		sendByte('t');
//		sendByte('o');
//		sendByte('p');
//		sendByte('\n');*/
//		OCR0A=120;
//		OCR0B=120;
//	}

	/*itoa(left,value,10);
	sendByte('L');
	sendByte('e');
	sendByte('f');
	sendByte('t');
	sendByte(':');
	for(int i=0;value[i]!='\0';i++){
		sendByte(value[i]);
	}
	sendByte('\n');

	for(int i=0;i<7;i++)
		value[i]=0;

	itoa(right,value,10);
	sendByte('R');
	sendByte('i');
	sendByte('g');
	sendByte('h');
	sendByte('t');
	sendByte(':');
	for(int i=0;value[i]!='\0';i++){
		sendByte(value[i]);
	}
	sendByte('\n');*/
	//_delay_ms(1000);

}

return 0;
}
