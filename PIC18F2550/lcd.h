#ifndef LCD_H
#define LCD_H

#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif
    void iniciaLCD (void);
    void esperalibre (void);
    void envia (unsigned char cosa);
    void lcdcon (unsigned char comando);
    void lcddat (unsigned char dato);
    void escribe (unsigned char mensaje[],signed char largo);
#ifdef	__cplusplus
}
#endif

#endif
