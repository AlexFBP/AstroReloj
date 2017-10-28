#include <xc.h>
#include <string.h>
#include <stdio.h>  //sprintf
#include <math.h>
#include <stdbool.h>
#include "lcd.h"

#define _XTAL_FREQ 4000000
#pragma config CPUDIV = OSC4_PLL6, PLLDIV = 12, USBDIV = 1	//1L
#pragma config FCMEN = ON, IESO = OFF, FOSC = INTOSCIO_EC	//1H
#pragma config PWRT = ON, VREGEN = OFF, BORV = 3, BOR = OFF	//2L
#pragma config WDTPS = 32768, WDT = OFF	//2H
#pragma config CCP2MX =	ON, PBADEN = OFF, MCLRE = ON, LPT1OSC = ON	//3H
#pragma config STVREN =	OFF, DEBUG = OFF, LVP = OFF//4L
#define true	1
#define false	0

///////////////////////////////////////////////////////
//DEFINICIONES, VARIABLES Y RUTINAS PARA LA LCD
#define BL      LATC2
#define BLOUT   true	//DEPURACION LOCAL: En true el backlight parpadea a 1 Hz (una vez configurado el reloj)
unsigned char bufprint[40];

/////////////////////////////////////////////////
// PROGRAMA PRINCIPAL
#define paramsX 14

volatile unsigned char siguiente; //Variable para mover el cursor de la LCD
volatile unsigned char x[paramsX]={1,2,0,0,0,0,0,1,0,1,2,0,1,6}; //fecha incial
//volatile unsigned char z[14]; //
volatile unsigned char cuenta;
volatile unsigned char cambio=0;
volatile unsigned char TU=0; 

volatile unsigned long FJu=0;

volatile unsigned char horaUTC[3]; //0:H,1:M,2:S
unsigned char horaSL[3]; //0:H,1:M,2:S
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

double ajustatiempo(double hora) {
	if (hora >=24.0 ) {
		hora -= 24.0*floor(hora / 24.0);
	}
	return hora;
}
void tiempo(unsigned char pos, unsigned char hora[]){
    unsigned char largo;
	lcdcon(pos);
    /*
	lcddat(hora[0]/10+48);
	lcddat(hora[0]%10+48);
	escribe((char *)"h",1);
	lcddat(hora[1]/10+48);
	lcddat(hora[1]%10+48);
	escribe((char *)"m",1);
	lcddat(hora[2]/10+48);
	lcddat(hora[2]%10+48);
	escribe((char *)"s",1);
    //*/    //Todo este bloque comentado lo hace la siguiente linea:
    //Se imprimen minimo 2 caracteres, pero si se excede, muestra tambien el numero excedido
    largo=sprintf(bufprint,"%02uh%02um%02us",hora[0],hora[1],hora[2]);
    escribe(bufprint,largo);
	//FJ(hora);	
}
/**
 * Supuestamente escribe la fecha juliana, recibiendo
 * un arreglo con la fecha gregoriana.
 */
void FJ (){
	int largo;
	double longr,lonm, lons, slon, lon;
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

	//largo=sprintf(bufprint,"FJ: %1u", FJu);
	//escribe(bufprint,largo);

	//2. Fraccion de Siglo Juliano
	double T=((double)(FJu-2451545))/36525.0;
	/*lcdcon(0x94);
	largo=sprintf(bufprint,"T:%g", T);
	escribe(bufprint,largo);*/
	double TSGO=(6.69737455833333);
	/*lcdcon(0x94);
	largo=sprintf(bufprint,"TSG0:%g", TSGO);
	escribe(bufprint,largo);*/
	TSGO += 2400.05133690556*T;
	/*lcdcon(0x94);
	largo=sprintf(bufprint,"TSG0:%g", TSGO);
	escribe(bufprint,largo);*/
	T*=T;
	/*lcdcon(0x94);
	largo=sprintf(bufprint,"T^2:%g", T);
	escribe(bufprint,largo);*/
	TSGO += 0.0000258611111111111*T;
	/*
	lcdcon(0x94);
	largo=sprintf(bufprint,"TSG0:%g", TSGO);
	escribe(bufprint,largo);*/
	T*=T;
	/*
	lcdcon(0x94);
	largo=sprintf(bufprint,"T^3:%g", T);
	escribe(bufprint,largo);*/
	TSGO -= 0.00000000172222222222222*T;
	/*
	lcdcon(0x94);
	largo=sprintf(bufprint,"TSG0:%g", TSGO);
	escribe(bufprint,largo);
	//lcdcon(0x94);
	*/
	//*
	//largo=sprintf(bufprint,"TSG0:%g", TSGO);
	//escribe(bufprint,largo);
	//*/
	/*unsigned long var2;
	var2 = (unsigned long) TSGO;
	lcdcon(0xC0);
	//*/

	//LINEA 89 JP
	double TSGOfloor=ajustatiempo(TSGO);
	//lcdcon(0x94);
	//largo=sprintf(bufprint,"TSG0:%g", TSGOfloor);
	//escribe(bufprint,largo);
	double horaUTCfloor = ((double) horaUTC[2]/3600.0);
	horaUTCfloor += ((double)horaUTC[0]+ (double)horaUTC[1]/60.0);
	//lcdcon(0x94);
	//largo=sprintf(bufprint,"TSG0:%g", horaUTCfloor);
	//escribe(bufprint,largo);
	double TSGt=ajustatiempo(TSGOfloor+(horaUTCfloor*1.00273790935079));
	//lcdcon(0x94);
	//largo=sprintf(bufprint,"TSG0:%g", TSGt);
	//escribe(bufprint,largo);
	longr = 74;
	lonm = 3.0;
	lons= 55.0;
	slon = -1;
	double lon = slon*(longr+(double)(lonm/60.0)+(double)(lons/3600.0));
	//lcdcon(0x94);
	//largo=sprintf(bufprint,"TSG0:%g", lon);
	//escribe(bufprint,largo);
	// Calcula finalmente tiempo sideral local
	double TSideralL=ajustatiempo(TSGt+(lon/15.0));
	horaSL[0]=(int)TSideralL;
	horaSL[1]=(int)((TSideralL-horaSL[0])*60);
	horaSL[2]=((((int)TSideralL-horaSL[0])*60)-horaSL[1])*60;
	tiempo(0x9A,horaSL);
}

/**
 * Demora de cant*100 ms
 */
void ms100 (unsigned char cant) {
	while (cant--) {
		__delay_ms(50);
		__delay_ms(50);
	}
}

unsigned char leeBCD (unsigned char buffer[],unsigned char digito) {
    unsigned char fila=buffer[digito/2];
    return ((digito%2)? (fila>>4) : fila)&0xF ;
}

void escribeBCD (unsigned char buffer[],unsigned char digito,unsigned char valor){
    if (valor>0xF) return;
    unsigned char fila = buffer[digito/2];
    buffer[digito/2]= digito%2 ? valor<<4 | fila&0xF : fila&0xF0 | valor ;
}

#define SIG     RA3
#define OK      RA4
#define CHANGE  RA6

/**
 * Interfaz de lectura de datos, y visualizacion en la LCD
 */
unsigned short dato (unsigned short minimo,unsigned short inicial,unsigned short maximo) {
    if (minimo>maximo || inicial<minimo || inicial>maximo) return 0;

    unsigned short resultado=maximo;    //Esta variable se va a reciclar
    unsigned char digitos=0,bcd_min[2],bcd_res[2],bcd_max[2],digito=0,aux,amax,amin;

    //CALCULO DE LOS DIGITOS REQUERIDOS E IMPRESION DEL VALOR INICIAL
    for (;resultado;resultado/=10,digitos++)
        lcdcon(CDSHIFT|CDRIGHT); //Calcula digitos, mueve para imprimir al reves, y al final del bucle, resultado=0;
    lcdcon(MODESET|DECR);//Ahora al escribir decrementa
    for (;digito<digitos;digito++) {    //Inicializacion de comparadores por digito
        escribeBCD(bcd_min,digito,minimo%10);   minimo  /= 10;
        aux=inicial%10;
        escribeBCD(bcd_res,digito,aux);  inicial /= 10; lcddat(aux+'0');//y se imprime el valor inicial, de paso
        escribeBCD(bcd_max,digito,maximo%10);   maximo  /= 10;
    }
    lcdcon(MODESET|INCR);//Al escribir incrementa.
    lcdcon(CDSHIFT|CDRIGHT);//Incrementa 1 para dejar en la posicion del primer digito (de mas peso)
    //INGRESO DEL USUARIO DIGITO POR DIGITO
    for (digito=digitos;digito;digito--) {
        resultado *= 10;
        amax=leeBCD(bcd_max,digito-1); //Carga valor maximo para el digito
        amin=leeBCD(bcd_min,digito-1); //Carga valor minimo para el digito

        //Impresion del valor actual del digito en modificacion
        if (amax==amin) {
            aux=amax;
        } else {
            aux=leeBCD(bcd_res,digito-1); //Carga valor inicial para el digito
            if (aux > amax)
                aux = amax;
            else if (aux < amin)
                aux = amin;
        }
        //Se queda adentro mientras el usuario no termine con OK
        while(true){
            lcddat(aux+'0');//1. Imprimir valor del digito actual, aux, en pantalla (adelanta 1 pos en LCD)
            lcdcon(CDSHIFT|CDLEFT);//2. Devolverse una posicion en LCD
            lcdcon(DISPCTL|DON|CON|BON);//3. Dejar cursor con parpadeo
            while (!(SIG||OK))
                ; //4. Esperar mientras no se presione pulsador
            lcdcon(DISPCTL|DON);//Apagar parpadeo.
            if (SIG) {//4.1 - SIG:
                while(SIG)
                    ;//Esperar que se destrabe
                aux++;  //Incrementar,
                if (aux>amax)   //decidir con respecto al incremento
                    aux=amin;   //para actualizar aux
            } else if (OK) {//4.2 - OK: Guardar valor de aux, romper este while
                while(OK)
                    ;//Esperar que se destrabe

                if ((digito > 1) && (amin != amax)) //Si son distintos, se puede hacer "cortocircuito"
                    if (aux < amax) {
                        escribeBCD(bcd_max,digito-2,9);
                    }
                    if (aux > amin) {
                        escribeBCD(bcd_min,digito-2,0);
                    }
                //escribeBCD(bcd_res,digito-1,aux);
                break;
            }
        }
        //Una vez no queden mas digitos, restablecer la LCD y colocar el cursor justo despues del ultimo digito
        lcdcon(CDSHIFT|CDRIGHT);
        resultado+=aux;
    }

    return resultado;
}

unsigned char limite; 	//Variable para limitar el valor de la fecha en cada digito
unsigned char num;	//Variable para contar el OK

double c;
int terminar;
unsigned char i;

void main(void) {
    OSCCON=0x63;    //INTOSC @ 4 MHz;

    //INICIALIZACION DE PUERTOS
	ADCON1=0x7|0xF; //Pines Digitales, ver datasheet.
	CMCON=0x7;      //Deshabilita los comparadores y deja el puerto A como proposito general

    LATA=0;
    LATB=0;
    //PUERTOS A y B se inicializan dentro de iniciaLCD()
	TRISC=0xFB; //Pin de backlight

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
	iniciaLCD();
	lcdcon(DISPCTL|DON);	//Display Control - Enciende display, sin cursor ni parpadeo
	cuenta=0;
	//////////////////////////////////////////////////////////
    // CODIGO NUEVO A PROBAR: INTERFACES DE ENTRADA Y SALIDA
    //* //NOTA: Quitar un / al comienzo para dejar el codigo original.
    cursor(0,0);//lcdcon(0x80); //primer linea
    escribe((char *)"HO",0);
    cursor(1,0);//lcdcon(0xC0); //segunda linea
    //lcdcon(CDSHIFT|CDRIGHT);
    //lcdcon(CDSHIFT|CDRIGHT);
    escribe((char *)"LA",0);
    cursor(2,0);//lcdcon(0x94); //tercera linea
    escribe((char *)"KE",0);
    cursor(3,0);//lcdcon(0xD4); //cuarta linea
    escribe((char *)"ASE?",0);
    unsigned short pruebita=(unsigned short)dato(2000,2016,2100);
    unsigned char buffer[10],largo;
    largo=sprintf(buffer,"%u",pruebita);
    escribe(buffer,largo);
    SLEEP();
    ////////////////////////////////////////////////////////////*/
    cursor(0,0);//primer linea
	escribe((char *)"Fecha:   01-01-2016",0);
	cursor(1,0); //segunda linea
	escribe((char *)"Hora:    12:00:00",0);
	cursor(2,0); //tercera linea
	escribe ((char *)"TU: ",0);
	cursor(3,0); //cuarta linea
	escribe((char *)"TSL: ",0);
	while (1){ //bucle infinito :D
		if (terminar==0){ //Revisa si no se ha dado el último Ok
			if (SIG){
				while (SIG)
					; //espera que se destrabe el boton SIG para cambiar el numero de la fecha

				lcdcon(siguiente);//Segunda linea
				if (x[i]==limite){ //Escribir en la LCD el numero de la fecha, si se llega el limite vuelve a cero
					x[i]=0;
					lcddat(x[i]+48);
				} else {  //Si no, suma y muestra en pantalla
					x[i]++;
					lcddat(x[i]+48);
				}
			} else if (OK){
				while (OK)
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
						siguiente+=2;
						num=num+1;
						break;
					case 13:
						terminar=1; // terminar de editar datos de entrada
						unsigned char main_hora = x[0]*10 + x[1]+5; //Ajusta a UTC
						if(main_hora>=24) //Si se excede:
							main_hora %= 24 ; //Dejar el residuo a 24
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
		if(cambio){
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
	while (1) ; //por si las pulgas :'v
}

/**
 * Servicio de Procesamiento de Interrupciones
 */
interrupt void ISR (void){
    static bool medseg = false;

	if (CCP1IF && CCP1IE) {
		CCP1IF=0;
		if (medseg) {
			//x[5]++;
			cambio = 1;
			#if BLOUT
			BL = 1;
			#endif
		}
		#if BLOUT
		else {
			BL = 0;
		}
		#endif
		medseg=!medseg;
	}
}
