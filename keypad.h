#include "defines.h"

int ts_state = 0; 
static char keyb_state = 0xFF;

int kbdGetCol(void) { /* Om någon tangent (i aktiverad rad)
* är nedtryckt, returnera dess kolumnnummer,
* annars, returnera 0 */
    unsigned char c;
    c = *GPIO_D_IDR_HIGH;
    if (c & 0x8) return 4;
    if (c & 0x4) return 3;
    if (c & 0x2) return 2;
    if (c & 0x1) return 1;
    return 0;
}

void kbdActivate(unsigned int row) { /* Aktivera angiven rad hos tangentbordet, eller
* deaktivera samtliga */
    switch (row) {
        case 1:
            *GPIO_D_ODR_HIGH = 0x10;
            break;
        case 2:
            *GPIO_D_ODR_HIGH = 0x20;
            break;
        case 3:
            *GPIO_D_ODR_HIGH = 0x40;
            break;
        case 4:
            *GPIO_D_ODR_HIGH = 0x80;
            break;
        case 5:
            *GPIO_D_ODR_HIGH = 0xF0;
            break;
        case 0:
            *GPIO_D_ODR_HIGH = 0x00;
            break;
    }
}

unsigned char keyb(void) {
    unsigned char key[] = {1, 2, 3, 0xA, 4, 5, 6, 0xB, 7, 8, 9, 0xC, 0xE, 0, 0xF, 0xD};
    int row, col;
    for (row = 1; row <= 4; row++) {
        kbdActivate(row);
        if ((col = kbdGetCol())) {
            kbdActivate(0);
            return key[4 * (row - 1) + (col - 1)];
        }
    }
    kbdActivate(0);
    return 0xFF;
}

void out7seg(unsigned char c) {
    if (c > 16) kbdActivate(0);
    unsigned char segCodes[] = {0x3F, 0x6, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x7, 0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79,
                                0x71};
    *GPIO_D_ODR_LOW = segCodes[c];

}

unsigned char keyb_enhanced(void){

	if ( ts_state ){
		kbdActivate(5);
		if( kbdGetCol() ) ts_state = 1;
		else ts_state = 0;
		return 0xFF;
	}
	
	
	if( !ts_state ){
		char c = keyb();
		if(c != keyb_state){
			ts_state = 1;
			return c; 
		}
	}
} 
