#include <xc.h>	//Para __delay_us()
#include <string.h>
#include "lcd.h"

#define _XTAL_FREQ 4000000
#define CON_BUSY_FLAG

#define E	LATA0
#define RS	LATA1
#define RW	LATA2

#define tr_bus	TRISB
#define BUS	LATB

#ifdef CON_BUSY_FLAG
#define BF	RB7
#endif

volatile bool lcd_init=false;

//Inicializacion por instruccion - cuando las condiciones
//no se cumplen para que funcione el circuito de reset
void iniciaLCD(){
    tr_bus=0;	//Todos los pines son salidas

    __delay_ms(42); //Espera que se estabilize la fuente
    lcdcon(0x30);	//Function Set
    __delay_ms(5);
    lcdcon(0x30);	//Function Set
    __delay_us(120);
    lcdcon(0x30);	//Function Set
    __delay_ms(2);
    lcd_init=true;
    lcdcon(0x38);	//Function Set - 8 bits, 2 lineas, 5x8
    lcdcon(0x08);	//Display Off
    lcdcon(1);	//Display Clear
    lcdcon(0x06);	//Mode Set - Incrementa en escritura, sin mover display
}

void esperalibre(void) {
#ifdef CON_BUSY_FLAG
	if (lcd_init) { //Leer el busy flag
		RS=0;
		tr_bus=0xff; //todos como entradas
		RW=1;	//Ahora los pines de datos de la LCD son salidas!
		do {
			E=0;
			__delay_us(1);//Descomentar si la LCD no responde
			E=1;
			__delay_us(1);//Descomentar si la LCD no responde
		} while (BF); //Espera que este libere el display
		E=0;
		RW=0;
		tr_bus=0;	//todos como salidas (estado inicial)
	}
#endif
}

void envia (unsigned char cosa) {
	BUS=cosa;
	E=1;
	__delay_us(1);
	E=0;
}

void lcdcon( unsigned char comando) {
	esperalibre();
	RS=0;
	envia(comando);
}

void lcddat (unsigned char dato) {
	esperalibre();
	RS=1;
	envia(dato);
}

void escribe (unsigned char mensaje[],signed char largo) {
	if (largo==-1) largo=strlen(mensaje);
	for (unsigned char i=0;i<largo;i++) {
		lcddat(mensaje[i]);
	}
}
