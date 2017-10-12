#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

//#include <time.h>
#define _XTAL_FREQ 4000000
#pragma config WDTE = OFF, PWRTE = ON, CP = OFF, BOREN = OFF, LVP = OFF, MCLRE = OFF, CPD = OFF, FOSC = INTOSCIO
#define true	1
#define false	0

//#include "cosa.h"

///////////////////////////////////////////////////////
//DEFINICIONES, VARIABLES Y RUTINAS PARA LA LCD

volatile char lcd_init;
volatile unsigned char bufprint[20];

#define E	RA0
#define RS	RA1
#define RW	RA2

#define tr_bus	TRISB
#define BUS	PORTB
#define BF	RB7

#define conbusyflag	true

#include "lcd.h"

/////////////////////////////////////////////////
// PROGRAMA PRINCIPAL
volatile unsigned char siguiente; //Variable para mover el cursor de la LCD
#define paramsX 14
volatile unsigned char x[paramsX]={1,2,0,0,0,0,0,1,0,1,2,0,1,6}; //fecha incial
//volatile unsigned char z[14]; //
volatile unsigned char cuenta;
volatile unsigned char cambio=0;
volatile unsigned char TU=0; 

volatile unsigned long FJu=0;


volatile unsigned char horaUTC[3]; //0:H,1:M,2:S
volatile unsigned char fecha1UTC[2]; //0:M,1:D
volatile unsigned int fecha2UTC; //Año

/**
 * Cada que se llama incrementa en un día la fecha
 * UTC y ajusta el calendario.
 */
void ajfecha (void) {
	
}

/**
 * Cada que se llama incrementa un segundo y ajusta
 * el reloj y fecha UTC almacenadas.
 */
void reloj (void){
	horaUTC[2]++; //Incrementa Segundos
	if (horaUTC[2]==60) { //60 segundos?
		//SI:
		horaUTC[1]++;//Incrementa Minutero
		horaUTC[2]=0;//
		if (horaUTC[1]==60) {
			horaUTC[0]++;
			horaUTC[1]=0;
			if (horaUTC[0]==24) {
				ajfecha();
			}
		}
	}
}

//double HH;	//Variable para calcular TU
//double HT;

/**
 * Supuestamente escribe la fecha juliana, recibiendo
 * un arreglo con la fecha gregoriana.
 */
void FJ (){
	int largo;
	
	//1. Calculo de Fecha Juliana
	int y,m;
	y=fecha2UTC;
	m=fecha1UTC[0];
	
	if (m<=2) {
		m+=12;
		y-=1;
	}
	
	FJu = 365*(unsigned long)(y+4716)+(unsigned long)((y+4716)/4);
	FJu += (unsigned long)(30.6001*(m+1)); //+
	FJu -= (unsigned long)(y/100); //-
	FJu += ((unsigned long)(y/100))/4; //-
	FJu += fecha1UTC[1]-1522; //+
	
	lcdcon(0xC0);
	largo=sprintf(bufprint,"FJ: %1u", FJu);
	//escribe(bufprint,largo);
	
	//2. Fraccion de Siglo Juliano
	double T=((double)FJu-2451545.0)/36525.0;
	double TSGO=(6.69737455833333);
	TSGO += 2400.05133690556*T;
	T*=T;
	TSGO += 0.0000258611111111111*T;
	//T2=T2*T;
	
	//TSGO -= 0.00000000172222222222222*T*T*T;
}

/**
 * 
 */
void tiempo(unsigned char pos, unsigned char hora[]){
	lcdcon(pos);
	lcddat(hora[0]/10+48);
	lcddat(hora[0]%10+48);
	escribe((char *)"h",1);
	lcddat(hora[1]/10+48);
	lcddat(hora[1]%10+48);
	escribe((char *)"m",1);
	lcddat(hora[2]/10+48);
	lcddat(hora[2]%10+48);
	escribe((char *)"s",1);
	//FJ(hora);	
}

#define SIG	RA3
#define	OK	RA4
#define CHANGE  RA6
#define	control	PORTA
#define tr_control	TRISA

unsigned char limite; 	//Variable para limitar el valor de la fecha en cada digito
unsigned char num;	//Variable para contar el OK

double c;
int terminar;
volatile bit medseg;
unsigned char i;

void main(void) {
	//INICIALIZACION DE PUERTOS
	
	control=0;
	CMCON=7;	//Deshabilita los comparadores y deja el puerto A como proposito general
	tr_control=0xf8;	//Solo los tres pines mas bajos son salidas
	tr_bus=0;	//Todos los pines son salidas
	limite=3;	//variable para limites de la fecha
	num=0;		//Variable para variar limites
	//HH=5;
	terminar=0;	// Variable para dejar de modificar datos 
	//HT=0;
	TU=0;
	cambio=0;
	i=6;
	siguiente=0x89; //segunda linea de la LCD
   
	//INICIALIZACION DEL MODULO CCP
	CCP1CON=0; // Apaga el modulo CCP
	TMR1=0;		// Resetear Timmer1
	INTCON=0;	// Deshabilitar interrupciones y borrar T0IF
	PIE1=0;		// Deshabilitar interrupciones por perifericos
	PIR1=0;		// Borrar bandera de interrupcion por perifericos
	T1CON=0x30;	// TMR1 con preescaler de 8
	CCPR1=0xF424; //CMP de 62500Hz
	CCP1CON=0x0B; // Modo de comparacion con evento especial 
	 
	
	//INICIALIZACION DE VARIABLES... y perifericos
	lcd_init=false;
	iniciaLCD();
	lcdcon(0x0C);	//Display Control - Enciende display, sin cursor ni parpadeo
	OK==0;	//Boton OK
	SIG==0;	//Boton siguiente para modificar fecha y hora
	CHANGE==0;
	medseg=0;
	cuenta=0;
	//////////////////////////////////////////////////////////
	escribe((char *)"Fecha:   01-01-2016",-1);
	lcdcon(0xC0); //segunda linea 
	escribe((char *)"Hora:    12:00:00",-1);
	lcdcon(0xD4); //tercera linea
	escribe ((char *)"TU: ",-1);
	lcdcon(0x94); //cuarta linea
	escribe((char*)"TSGO: ",-1);
	while (1){ //bucle infinito :D
		if (terminar==0){ //Reviza si no se ha dado el último Ok 
			if (SIG==1){
				while (SIG==1)
					; //espera que se destrabe el boton SIG para cambiar el numero de la fecha

				lcdcon(siguiente);//Segunda linea
				if (x[i]==limite){ //Escribir en la LCD el numero de la fecha, si se llega el limite vuelve a cero
					x[i]=0; 		
					lcddat(x[i]+48);
				} else {  //Si no, suma y muestra en pantalla
					x[i]++;
					lcddat(x[i]+48);
				}
			} else if (OK==1){
				while (OK==1)
					;// Espera a que Ok se destrabe
				switch (num) {
					case 0:	//si fue el primer OK, entonces cambia el limite 9
						i++;
						//siguiente++;
						limite=9;
						lcdcon(++siguiente); //?
						num++;
						break;
					case 1:
						i++; 
						siguiente+=2;
						num++;
						limite=1; 
						break;
					case 2:
					case 4:
					case 5:
					case 6:
					case 8:
					case 10:
					case 12:
						i++;
						siguiente++;
						limite=9;
						num++;
						break;
					case 3:
						i++; 
						siguiente+=2;
						limite=2;
						num++; 
						break; 
					case 7:
						i=0;
						limite=2;
						siguiente=0xC9;
						num=num+1; 
						break;
					case 9:
					case 11:
						i++;
						limite=5;  
						siguiente=siguiente+2;
						num=num+1;
						break;
					case 13:
						terminar=1; // terminar de editar datos de entrada
						unsigned char main_hora = x[0]*10 + x[1]+5; //Ajusta a UTC
						if(main_hora>=24) //Si se excede:
							main_hora= main_hora % 24 ; //Calcular el residuo a 24
						x[0] = main_hora / 10; //Separa la decena
						x[1] = main_hora % 10; //Separa la unidad
						horaUTC[0]=main_hora;
						horaUTC[1]=x[2]*10+x[3];
						horaUTC[2]=x[4]*10+x[5];
						fecha1UTC[0]=x[6]*10+x[7];
						fecha1UTC[1]=x[8]*10+x[9];
						fecha2UTC=x[10]*1000+x[11]*100+x[12]*10+x[13];
						tiempo(0xDC,(unsigned char *)horaUTC); //Muestra en display
						TMR1ON=1;	//Arranca el TMR
						CCP1IE=1;	//Interrupcion del modulo de captura y comparacion
						PEIE=1;		//Interrupcion por perifericos
						GIE=1;      //Interrupciones globales
						break;					
					}
				}
			}
		if(cambio==1){
			//reloj(x[5],0);	//le falta un argumento, modificaste reloj, cierto?
			reloj();	//Incrementa segundo y actualiza horaUTC
			/*
			for (unsigned char i=0;i<strlen((unsigned char *)x);i++) {
				z[i]=x[i];
			} //*/
			tiempo(0xDC,(unsigned char *)horaUTC); //Refresca Pantalla
			FJ();
			cambio=0;
		}
	} 
	while (1)
		; //por si las pulgas :v
}
interrupt void ISR (void){
	if (CCP1IF && CCP1IE) {
		CCP1IF=0;
		if (medseg) {
			//x[5]++;
			cambio=1;
			
		}
		medseg=!medseg;
	}
}