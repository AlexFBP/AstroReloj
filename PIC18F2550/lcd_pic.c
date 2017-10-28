//---------------------------------------------------------------------
// LIBRERIAS
#include <xc.h> //Para __delay_us() y acceso a pines del microcontrolador
#include <string.h>
#include "lcd.h"

//---------------------------------------------------------------------
// PARAMETROS DE LA LCD
#define FILAS_LCD       4
#define COLUMNAS_LCD    20
#define F2HW    0x40    //Posicion en hardware de la fila 2
#define C2HW    20      //Posicion en hardware de la columna 2

#define _XTAL_FREQ 4000000
#define CON_BUSY_FLAG

#define control     PORTA
#define tr_control  TRISA
#define E	LATA0
#define RS	LATA1
#define RW	LATA2

#define tr_bus	TRISB
#define BUS	LATB
#ifdef CON_BUSY_FLAG
#define BF	RB7
#endif

volatile bool lcd_init=false;

/**
 * Inicialización de la LCD.
 * POR HACER: Mirar si los parámetros de inicializacíón se pueden pasar por referencia
 */
void iniciaLCD(){
    control=0;
    tr_control&=0xF8;//Los tres pines mas bajos son salidas
    BUS=0;
    tr_bus=0; //Todos los pines son salidas

    resetLCD();
}

/**
 * Inicializacion por instruccion, cuando no se cumplen las condiciones
 * para que funcione el circuito de reset (ver datasheet HD44780)
*/
void resetLCD (void) {
    __delay_ms(42); //Espera que se estabilize la fuente
    lcdcon(FUNCSET|MODE8);	//Function Set
    __delay_ms(5);
    lcdcon(FUNCSET|MODE8);	//Function Set
    __delay_us(120);
    lcdcon(FUNCSET|MODE8);	//Function Set
    __delay_ms(2);
    lcd_init=true;  //A partir de este momento la LCD ya esta inicializada
                    //y se puede leer la bandera de ocupado
    lcdcon(FUNCSET|MODE8|NLINE);	//Function Set - 8 bits, 2 lineas, 5x8
    lcdcon(DISPCTL);	//Display Off
    lcdcon(CLRDISP);	//Display Clear
    lcdcon(MODESET|INCR);	//Mode Set - Incrementa en escritura, sin mover display
}

/**
 * Detecta si la LCD esta ocupada y espera hasta que se libere
 */
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
#else
    __delay_ms(2);  //Demora maxima, por borrado de la pantalla (datasheet HD44780)
#endif
}

/**
 * Coloca un octeto en el bus de datos de la LCD
 */
void envia (unsigned char cosa) {
	BUS=cosa;
	E=1;
	__delay_us(1);
	E=0;
}

/**
 * Envia un comando al controlador de la LCD
 */
void lcdcon(unsigned char comando) {
	esperalibre();
	RS=0;
	envia(comando);
}

/**
 * Envia un dato al controlador de la LCD
 */
void lcddat (unsigned char dato) {
	esperalibre();
	RS=1;
	envia(dato);
}

/**
 * Imprime en pantalla un string, preferentemente generado con
 * sprintf. Ingresar 0 en largo para que la funcion lo calcule.
 */
void escribe (unsigned char *mensaje,unsigned char largo) {
	if (!largo) largo=strlen(mensaje);
	for (unsigned char i=0;i<largo;i++) {
		lcddat(mensaje[i]);
	}
}

/**
 * Ubica el cursor en una posicion arbitraria de la LCD.
 * Devuelve true solamente si la posicion deseada es valida
 */
bool cursor(unsigned char fila,unsigned char columna) {
    if (fila<FILAS_LCD && columna<COLUMNAS_LCD) {
        lcdcon(SetDDRAM+((fila&1)?F2HW:0)+((fila&2)?C2HW:0)+columna);
        return true;
    } else return false;
}
