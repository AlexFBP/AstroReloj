#ifndef LCD_H
#define LCD_H

#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

    //HD44780 Commands
    #define CLRDISP     0x01
    #define RETHOME     0x02
    #define MODESET     0x04
        #define INCR    2
        #define DECR    0
        #define SHDISP  1       //Accompanies display shift
    #define DISPCTL     0x08
        #define DON     4       //Display ON
        #define CON     2       //Cursor ON
        #define BON     1       //Blink ON
    #define CDSHIFT     0x10
        #define DSHIFT  8       //Display Shift
        #define CMOVE   0       //Cursor Move
        #define CDRIGHT 4
        #define CDLEFT  0
    #define FUNCSET     0x20
        #define MODE8   0x10    //8 Bbit (0 by default)
        #define NLINE   8       //2 Lines (1 by default)
        #define FTYPE   4       //5x10 dots (5x8 dots by default)
    #define SetCGRAM    0x40
    #define SetDDRAM    0x80

    //Inicializacion de puertos del micro y de la LCD en si.
    void iniciaLCD (void);

    //Secuencia de inicializacion de la LCD segun el datasheet HD44780
    void resetLCD (void);

    //Ubica el cursor en la fila y columna especificadas, de la pantalla. No direccionar limites.
    bool cursor(unsigned char fila,unsigned char columna);

    //Espera hasta que la LCD pueda recibir comandos
    void esperalibre (void);

    //Envia un comando a la LCD
    void lcdcon (unsigned char comando);

    //Envia un dato a la LCD
    void lcddat (unsigned char dato);

    //Coloca un octeto en el bus de datos
    void envia (unsigned char cosa);

    //Imprime una frase en la LCD. Si el largo es 0 la calcula internamente
    void escribe (unsigned char *mensaje,unsigned char largo);
#ifdef	__cplusplus
}
#endif

#endif
