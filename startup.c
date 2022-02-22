/*
 *     startup.c
 */
#define GPIO_D 			0x40020C00
#define GPIO_MODER 		((volatile unsigned int *) (GPIO_D)) 
#define GPIO_OTYPER 	((volatile unsigned int *) (GPIO_D+0x4))
#define GPIO_OSPEEDR 	((volatile unsigned int *) (GPIO_D+0x8))
#define GPIO_PUPDR 		((volatile unsigned int *) (GPIO_D+0xC))
#define GPIO_IDR_LOW 	((volatile unsigned char *) (GPIO_D+0x10))
#define GPIO_IDR_HIGH	((volatile unsigned char *) (GPIO_D+0x11))
#define GPIO_ODR_LOW 	((volatile unsigned char *) (GPIO_D+0x14))
#define GPIO_ODR_HIGH 	((volatile unsigned char *) (GPIO_D+0x15))

#define STK 			0xE000E010
#define STK_CTRL		((volatile unsigned int *) (STK))
#define STK_LOAD		((volatile unsigned int *) (STK+0x4))
#define STK_VAL 		((volatile unsigned int *) (STK+0x8))
#define CALIB 			((volatile unsigned int *) (STK+0xC))

#define B_E 			0x40 // 
#define B_SELECT		4 //Select sätts alltid till 1 vid använding av ascii displayen
#define B_RW			2 //Read/Write, sätts till 1 vid läsning
#define B_RS			1 //Denna bit ska vara 0 om kommando skrivs till eller status läses från displayen. 1 om data skrivs till eller läses från displayen.

#define FUNCTION_SET 	0x38 //Dessa definitioner är enligt kommandotabellen i Rogers support klipp
#define DISPLAY_CONTROL 0xF
#define CLEAR_DISPLAY 	1
#define ENTRY_MODE_SET	4
#define MAX_POINTS		350
#define SIMULATOR		1

#define TIM6_CR1		((volatile unsigned short*) 0x40001000)
#define TIM6_CNT		((volatile unsigned short*) 0x40001024)
#define TIM6_ARR		((volatile unsigned short*) 0x4000102C)
#define UDIS			(1<<1)
#define CEN				(1<<0)
#define STK_CTRL 		((volatile unsigned int *)(0xE000E010))  
#define STK_LOAD 		((volatile unsigned int *)(0xE000E014))  
#define STK_VAL 		((volatile unsigned int *)(0xE000E018)) 

#define SCB_VTOR 		((volatile unsigned int *) (0x2001C000))

#define SYSCFG_EXTICR1 	((short *) (0x40013808))
#define EXTI_IRM 		((int *)(0x40013C00))
#define EXTI_RTSR 		((int *)(0x40013C08))
#define EXTI_PR 		((int *)(0x40013C14))
#define NVIC_ISER0 		((int *)(0xE000E100))
#define EXTI3_IRQ 		(0x2001C000 + 0x00000064)

int ts_state = 0; 
static char keyb_state = 0xFF;

__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");        /* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");                    /* call main */
__asm__ volatile(".L1: B .L1\n");                /* never return */
}

__attribute__((naked))
void graphic_initialize(void){
    __asm volatile (" .HWORD 0xDFF0\n");
    __asm volatile (" BX LR\n");
}

__attribute__((naked))
void graphic_clear_screen(void){
    __asm volatile (" .HWORD 0xDFF1\n");
    __asm volatile (" BX LR\n");
}

__attribute__((naked))
void graphic_pixel_set(int x, int y){
    __asm volatile (" .HWORD 0xDFF2\n");
    __asm volatile (" BX LR\n");
}

__attribute__((naked))
void graphic_pixel_clear(int x, int y){
    __asm volatile (" .HWORD 0xDFF3\n");
    __asm volatile (" BX LR\n");
}

void init_app(void) {
    *GPIO_MODER = 0x55005555;
}

void ascii_ctrl_bit_set(unsigned char x){
	char c;
	c = *GPIO_ODR_LOW;
	*GPIO_ODR_LOW = B_SELECT | x | c; // Select alltid 1 i vårt fall och sätt x biten till 1.
}

void ascii_ctrl_bit_clear(unsigned char x){
	char c;
	c = *GPIO_ODR_LOW;
	c = c & ~x; // detta gör att x biten nollställs medan alla andra bitar behåller sina värden.
	*GPIO_ODR_LOW = B_SELECT | c; // Select alltid 1
}

void ascii_write_controller(unsigned char byte){	
	ascii_ctrl_bit_set(B_E); // E = 1 betyder att arbetscyklen startas
	*GPIO_ODR_HIGH = byte;
	ascii_ctrl_bit_clear(B_E); // Efter att uppgiften utförts så avslutar vi arbetscykeln.
	delay_250ns();
}
	

unsigned char ascii_read_controller(void){
	unsigned char c;
	ascii_ctrl_bit_set(B_E); // Starta arbetscyklen
	
	delay_250ns(); // Vänta minst 360 ns innan datan är förberedd av ascii displayen för att läsas
	delay_250ns();
	
	c = *GPIO_IDR_HIGH;
	
	ascii_ctrl_bit_clear(B_E);
	
	return c;
}

void ascii_write_cmd(unsigned char command){
	ascii_ctrl_bit_clear(B_RS);
	ascii_ctrl_bit_clear(B_RW);
	ascii_write_controller(command);
}

void ascii_write_data(unsigned char data){
	ascii_ctrl_bit_set(B_RS);
	ascii_ctrl_bit_clear(B_RW);
	ascii_write_controller(data);
}

unsigned char ascii_read_status(void){
	char c;
	*GPIO_MODER = 0x00005555; //sätter bit 8-15 i porten (dataregistret för ascii displayen) till ingångar som förberedelse för att ascii_read_controller ska läsa från dem senare.
	ascii_ctrl_bit_set(B_RW);
	ascii_ctrl_bit_clear(B_RS);
	c = ascii_read_controller();
	
	*GPIO_MODER = 0x55555555; //återställer dataregistret till utgång
	
	return c;
}

unsigned char ascii_read_data(void){
	char c;
	*GPIO_MODER = 0x00005555;  
	ascii_ctrl_bit_set(B_RW);
	ascii_ctrl_bit_set(B_RS); // som ovan men nu är RS = 1 för att vi läser data istället för status
	c = ascii_read_controller();
	
	*GPIO_MODER = 0x55555555;
	
	return c;
}

void ascii_command(command){
	while( ascii_read_status() & 0x80 ); // Vänta så länge ascii displayen är upptagen
	delay_micro(8);
	
	ascii_write_cmd(command);
	
	if(command == CLEAR_DISPLAY){
		delay_milli(2);
	}
	else if(command == FUNCTION_SET || command == DISPLAY_CONTROL || ENTRY_MODE_SET){
		delay_micro(39);
	}
}

void ascii_init(void){
	ascii_ctrl_bit_clear(B_RS); // För dessa kommandon skall RS och RW = 0
	ascii_ctrl_bit_clear(B_RW);
	
	ascii_command(FUNCTION_SET);
	ascii_command(DISPLAY_CONTROL);
	ascii_command(CLEAR_DISPLAY);
}

void ascii_write_char(unsigned char c){
	while( ascii_read_status() & 0x80 ); // Vänta så länge ascii displayen är upptagen
	delay_micro(8);
	
	ascii_write_data(c);
}

void ascii_gotoxy(int x, int y){	
	int address = x-1;
	if (y == 2){
		address = address + 0x40; // Teckenminnet har plats för 64 tecken per rad (20 visas), därför blir addressen för rad 2 lika med 0x40 (64 i decimal)
	}
	ascii_write_cmd(0x80 | address);
}

int kbdGetCol(void) { /* Om någon tangent (i aktiverad rad)
* är nedtryckt, returnera dess kolumnnummer,
* annars, returnera 0 */
    unsigned char c;
    c = *GPIO_IDR_HIGH;
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
            *GPIO_ODR_HIGH = 0x10;
            break;
        case 2:
            *GPIO_ODR_HIGH = 0x20;
            break;
        case 3:
            *GPIO_ODR_HIGH = 0x40;
            break;
        case 4:
            *GPIO_ODR_HIGH = 0x80;
            break;
        case 5:
            *GPIO_ODR_HIGH = 0xF0;
            break;
        case 0:
            *GPIO_ODR_HIGH = 0x00;
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
    *GPIO_ODR_LOW = segCodes[c];

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

void delay_250ns(void){
    *STK_CTRL = 0;
    *STK_LOAD = ((168/4) - 1);
    *STK_VAL = 0;
    *STK_CTRL = 5;
    while( (*STK_CTRL & 0x10000) == 0); // Väntar tills statusbiten är lika med 0 innan den fortsätter då det innebär att nedräkningen är färdig.
    *STK_CTRL = 0;
}

void delay_micro(unsigned int us){
#ifdef SIMULATOR
    us = us / 1000;
    us++;
#endif 
    while( us > 0 ){
        delay_250ns();
        delay_250ns();
        delay_250ns();
        delay_250ns();
        us--;
    }
}

void delay_milli(unsigned int ms){
#ifdef SIMULATOR
    ms /= 1000;
    ms++;
#endif
    while(ms > 0){
        delay_micro(1000);
        ms--;
    }
}

typedef struct{
	char x,y;
} POINT, *PPOINT;

typedef struct{
	int numpoints;
	int sizex;
	int sizey;
	POINT px[MAX_POINTS];
} GEOMETRY, *PGEOMETRY;

typedef struct tObj{
	PGEOMETRY geo;
	int dirx, diry;
	int posx, posy;
	void (*draw) (struct tObj *);
	void (*clear) (struct tObj *);
	int (* move) (struct tObj *);
	void (* set_speed) (struct tObj *, int, int);
} OBJECT, *POBJECT;

int objects_overlap( POBJECT fucking_bird, POBJECT obst_1_t, POBJECT obst_1_b, POBJECT obst_2_t, POBJECT obst_2_b){

	if(obst_1_t->posx <= fucking_bird->posx + fucking_bird->geo->sizex - 1 && !(obst_1_t->posx + obst_1_t->geo->sizex < fucking_bird->posx)){
		if(fucking_bird->posy <= obst_1_t->posy + obst_1_t->geo->sizey) return 1;
		else if(fucking_bird->posy + fucking_bird->geo->sizey - 3 >= obst_1_b->posy) return 1;
		else return 0;
	}
	
	else if(obst_2_t->posx <= fucking_bird->posx + fucking_bird->geo->sizex - 1 && !(obst_2_t->posx + obst_2_t->geo->sizex < fucking_bird->posx)){
		if(fucking_bird->posy <= obst_2_t->posy + obst_2_t->geo->sizey) return 1;
		else if(fucking_bird->posy + fucking_bird->geo->sizey - 3 >= obst_2_b->posy) return 1;  
		else return 0;
	} else return 0; 
}

void draw_object(POBJECT obj){
	for(int i = 0; i < obj->geo->numpoints; i++){
		graphic_pixel_set(obj->geo->px[i].x + obj->posx, obj->geo->px[i].y + obj->posy);
	}
}

void clear_object(POBJECT obj){
	for(int i = 0; i < obj->geo->numpoints; i++){
		graphic_pixel_clear(obj->geo->px[i].x + obj->posx, obj->geo->px[i].y + obj->posy);
	}
}

void move_object(POBJECT obj){
	clear_object(obj);
	obj->posx += obj->dirx;
	obj->posy += obj->diry;
	draw_object(obj);
}

void set_object_speed(POBJECT obj, int x, int y){
	obj->dirx = x;
	obj->diry = y;
}

void timer6_init(){
	*TIM6_CR1 &= ~CEN;
	*TIM6_ARR = 0xFFFF;
	*TIM6_CR1 |= (CEN | UDIS);
	
}

void move_obstacles(POBJECT o1t, POBJECT o1b, POBJECT o2t, POBJECT o2b){
	o1t->move(o1t);
	o1b->move(o1b);
	o2t->move(o2t);
	o1b->move(o2b);
}

void set_obstacle_speed(int x, int y, POBJECT o1t, POBJECT o1b, POBJECT o2t, POBJECT o2b){
	o1t->set_speed(o1t, x,y);
	o1b->set_speed(o1b, x,y);
	o2t->set_speed(o2t, x,y);
	o2b->set_speed(o2b, x,y);
}

GEOMETRY obstacle_top_geometry = {

	126,
	14, 50,
	{
			{1, 0}, {12, 0},
			{1, 1}, {12, 1},
			{1, 2}, {12, 2},
			{1, 3}, {12, 3},
			{1, 4}, {12, 4},
			{1, 5}, {12, 5},
			{1, 6}, {12, 6},
			{1, 7}, {12, 7},
			{1, 8}, {12, 8},
			{1, 9}, {12, 9},
			{1, 10}, {12, 10},
			{1, 11}, {12, 11},
			{1, 12}, {12, 12},
			{1, 13}, {12, 13},
			{1, 14}, {12, 14},
			{1, 15}, {12, 15},
			{1, 16}, {12, 16},
			{1, 17}, {12, 17},
			{1, 18}, {12, 18},
			{1, 19}, {12, 19},
			{1, 20}, {12, 20},
			{1, 21}, {12, 21},
			{1, 22}, {12, 22},
			{1, 23}, {12, 23},
			{1, 24}, {12, 24},
			{1, 25}, {12, 25},
			{1, 26}, {12, 26},
			{1, 27}, {12, 27},
			{1, 28}, {12, 28},
			{1, 29}, {12, 29},
			{1, 30}, {12, 30},
			{1, 31}, {12, 31},
			{1, 32}, {12, 32},
			{1, 33}, {12, 33},
			{1, 34}, {12, 34},
			{1, 35}, {12, 35},
			{1, 36}, {12, 36},
			{1, 37}, {12, 37},
			{1, 38}, {12, 38},
			{1, 39}, {12, 39},
			{1, 40}, {12, 40},
			{1, 41}, {12, 41},
			{1, 42}, {12, 42},
			{1, 43}, {12, 43},
			{1, 44}, {12, 44},
			{1, 45}, {12, 45},
			{0, 46}, {1, 46}, {2, 46}, {3, 46}, {4, 46},{5, 46},{6, 46},{7, 46},{8, 46},{9, 46},{10, 46},{11, 46},{12, 46},{13, 46},
			{0, 47}, {13, 47},
			{0, 48}, {13, 48},
			{0, 49}, {13, 49},
			{0, 50}, {1, 50}, {2, 50}, {3, 50}, {4, 50},{5, 50},{6, 50},{7, 50},{8, 50},{9, 50},{10, 50},{11, 50}, {12, 50},{13, 50},
	}
};
	
GEOMETRY obstacle_bottom_geometry = {

	126,
	14, 50,
	{
			{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0},{5, 0},{6, 0},{7, 0},{8, 0},{9, 0},{10, 0},{11, 0},{12, 0},{13, 0},
			{0, 1}, {13, 1},
			{0, 2}, {13,2},
			{0, 3}, {13, 3},
			{0, 4}, {1, 4}, {2, 4}, {3, 4}, {4, 4},{5, 4},{6, 4},{7, 4},{8, 4},{9, 4},{10, 4},{11, 4}, {12, 4},{13, 4},
			{1, 5}, {12, 5},
			{1, 6}, {12, 6},
			{1, 7}, {12, 7},
			{1, 8}, {12, 8},
			{1, 9}, {12, 9},
			{1, 10}, {12, 10},
			{1, 11}, {12, 11},
			{1, 12}, {12, 12},
			{1, 13}, {12, 13},
			{1, 14}, {12, 14},
			{1, 15}, {12, 15},
			{1, 16}, {12, 16},
			{1, 17}, {12, 17},
			{1, 18}, {12, 18},
			{1, 19}, {12, 19},
			{1, 20}, {12, 20},
			{1, 21}, {12, 21},
			{1, 22}, {12, 22},
			{1, 23}, {12, 23},
			{1, 24}, {12, 24},
			{1, 25}, {12, 25},
			{1, 26}, {12, 26},
			{1, 27}, {12, 27},
			{1, 28}, {12, 28},
			{1, 29}, {12, 29},
			{1, 30}, {12, 30},
			{1, 31}, {12, 31},
			{1, 32}, {12, 32},
			{1, 33}, {12, 33},
			{1, 34}, {12, 34},
			{1, 35}, {12, 35},
			{1, 36}, {12, 36},
			{1, 37}, {12, 37},
			{1, 38}, {12, 38},
			{1, 39}, {12, 39},
			{1, 40}, {12, 40},
			{1, 41}, {12, 41},
			{1, 42}, {12, 42},
			{1, 43}, {12, 43},
			{1, 44}, {12, 44},
			{1, 45}, {12, 45},
			{1, 46}, {12, 46},
			{1, 47}, {12, 47},
			{1, 48}, {12, 48},
			{1, 49}, {12, 49},
			{1, 50}, {12, 50},
	}
};
	
GEOMETRY bird_flap_geometry = {
	44,
	13,12,
	{
	{0,7},{0,8},
	{1,6},{1,8},
	{2,4},{2,5},{2,8},
	{3,3},{3,6},{3,8},{3,9},{3,10},{3,11},
	{4,2},{4,6},{4,8},
	{5,1},{5,3},{5,6},{5,8},
	{6,0},{6,4},{6,5},{6,6},{6,8},{6,9},{6,10},{6,11},
	{7,0},{7,8},
	{8,0},{8,2},{8,8},
	{9,0},{9,8},
	{10,1},{10,3},{10,4},{10,5},{10,6},{10,7},
	{11,2},{11,3},
	{12,3}	
	}
};	

GEOMETRY bird_soar_geometry = {
	47,
	14,12,
	{
	{-1,0},{-1,0},
	{0,0},{0,2},{0,7},{0,8},
	{1,0},{1,3},{1,6},{1,8},
	{2,0},{2,4},{2,5},{2,8},
	{3,1},{3,2},{3,3},{3,8},{3,9},{3,10},{3,11},
	{4,2},{4,3},{4,8},
	{5,1},{5,8},
	{6,0},{6,8},{6,9},{6,10},{6,11},
	{7,0},{7,8},
	{8,0},{8,2},{8,8},
	{9,0},{9,8},
	{10,1},{10,3},{10,4},{10,5},{10,6},{10,7},
	{11,2},{11,3},
	{12,3}	
	}
};

GEOMETRY starting_screen_geometry = {

	317,
	67,37,
	{
		{2,0},{3,0},{4,0}, // 8: 50
		{1,1},{2,1},{4,1},{5,1},
		{0,2},{1,2},{5,2},{6,2},
		{0,3},{6,3},
		{0,4},{6,4},
		{0,5},{6,5},
		{0,6},{6,6},
		{0,7},{6,7},
		{1,8},{5,8},
		{2,9},{3,9},{4,9},
		{2,10},{3,10},{4,10},
		{1,11},{5,11},
		{0,12},{6,12},
		{-1,13},{7,13},
		{-1,14},{7,14},
		{-1,15},{7,15},
		{-1,16},{7,16},
		{-1,17},{7,17},
		{0,18},{6,18},
		{1,19},{2,19},{3,19},{4,19},{5,19},
		
		{10,12},{11,12},{12,12},{13,12}, // -: 4
		
		{16,6}, // b: 22
		{16,7},
		{16,8},
		{16,9},
		{16,10},
		{16,11},
		{16,12},{17,12},{18,12},{19,12},
		{16,13},{20,13},
		{16,14},{21,14},
		{16,15},{21,15},
		{16,16},{20,16},
		{16,17},{17,17},{18,17},{19,17},
		
		{23,9}, // i: 8
		{23,11},
		{23,12},
		{23,13},
		{23,14},
		{23,15},
		{23,16},
		{23,17},
		
		{26,9}, // t: 13
		{26,10}, 
		{26,11},
		{26,12},
		{25,13},{26,13},{27,13},
		{26,14},
		{26,15},
		{26,16},
		{26,17},{27,17},{28,17},
		
		{33,6}, // b: 22
		{33,7},
		{33,8},
		{33,9},
		{33,10},
		{33,11},
		{33,12},{34,12},{35,12},{36,12},
		{33,13},{37,13},
		{33,14},{38,14},
		{33,15},{38,15},
		{33,16},{37,16},
		{33,17},{34,17},{35,17},{36,17},
		
		{40,9}, // i: 8
		{40,11},
		{40,12},
		{40,13},
		{40,14},
		{40,15},
		{40,16},
		{40,17},
		
		{43,11},{45,11},{46,11}, // r: 10
		{43,12},{44,12},
		{43,13},
		{43,14},
		{43,15},
		{43,16},
		{43,17},
		
		{51,6}, // b: 22
		{51,7},
		{51,8},
		{51,9},
		{51,10},
		{51,11},
		{51,12},{50,12},{49,12},{48,12},
		{51,13},{47,13},
		{51,14},{46,14},
		{51,15},{46,15},
		{51,16},{47,16},
		{51,17},{50,17},{49,17},{48,17},
		
		{53,0},{54,0},{55,0},{56,0},{57,0}, // TM: 25
		{55,1},
		{55,2},
		{55,3},
		{55,4},
		{55,5},
		{60,0},{64,0},
		{59,1},{61,1},{63,1},{65,1},
		{59,2},{62,2},{65,2},
		{59,3},{65,3},
		{59,4},{65,4},
		{59,5},{65,5},
		
		{16,22},{17,22},{18,22},{19,22}, // p: 18
		{16,23},{20,23},
		{16,24},{21,24},
		{16,25},{21,25},
		{16,26},{17,26},{18,26},{19,26},{20,26},
		{16,27},
		{16,28},
		{16,29},
		
		{23,25},{25,25},{26,25}, // r: 8
		{23,26},{24,26},
		{23,27},
		{23,28},
		{23,29},
		
		{29,25},{30,25}, // e: 13
		{28,26},{31,26},
		{28,27},{29,27},{30,27},{31,27},
		{28,28},
		{28,29},{29,29},{30,29},{31,29},
		
		{34,25},{35,25},{36,25}, // s: 9
		{34,26},
		{35,27},
		{36,28},
		{34,29},{35,29},{36,29},
		
		{39,25},{40,25},{41,25}, // s: 9
		{39,26},
		{40,27},
		{41,28},
		{39,29},{40,29},{41,29},
		
		{45,23}, // '5': 16
		{45,24},
		{47,25},{48,25},{49,25},{50,25},
		{47,26},
		{47,27},{48,27},{49,27},
		{50,28},
		{47,29},{48,29},{49,29},
		{52,23},
		{52,24},
		
		
		{16,31}, // t: 10
		{16,32}, 
		{15,33},{16,33},{17,33},
		{16,34},
		{16,35},
		{16,36},{17,36},{18,36},
		
		{20,33},{21,33}, // o: 8
		{19,34},{22,34},
		{19,35},{22,35},
		{20,36},{21,36},
		
		{26,31},{27,31},{28,31},{29,31}, // p: 14
		{26,32},{30,32},
		{26,33},{30,33},
		{26,34},{27,34},{28,34},{29,34},
		{26,35},
		{26,36},
		
		{32,31}, // l: 7
		{32,32},
		{32,33},
		{32,34},
		{32,35},
		{32,36},{33,36},
		
		{36,32},{37,32},{38,32}, // a: 14
		{35,33},{39,33},
		{35,34},{39,34},
		{35,35},{39,35},
		{36,36},{37,36},{38,36},{40,36},{41,36},
		
		{43,32},{47,32}, // y: 7
		{44,33},{46,33},
		{45,34},
		{44,35},
		{43,36},
	}
};

static OBJECT starting_screen_obj = {
	&starting_screen_geometry,
	0,0,
	32,16,
	draw_object,
	clear_object,
};

static OBJECT obstacle_top_obj_1 = {
	&obstacle_top_geometry,
	0,0, //direction speed
	60,-29, //starting position
	draw_object,
	clear_object,
	move_object,
	set_object_speed
};

static OBJECT obstacle_bottom_obj_1 = {
	&obstacle_bottom_geometry,
	0,0, //direction speed
	60,43, //starting position
	draw_object,
	clear_object,
	move_object,
	set_object_speed
};

static OBJECT obstacle_top_obj_2 = {
	&obstacle_top_geometry,
	0,0, 
	130,-29, 
	draw_object,
	clear_object,
	move_object,
	set_object_speed
};

static OBJECT obstacle_bottom_obj_2 = {
	&obstacle_bottom_geometry,
	0,0, 
	130,43, 
	draw_object,
	clear_object,
	move_object,
	set_object_speed
};

static OBJECT bird_soar_obj = {
	&bird_soar_geometry,
	0,0, 
	5,30, 
	draw_object,
	clear_object,
	move_object,
	set_object_speed
};

static OBJECT bird_flap_obj = {
	&bird_flap_geometry,
	0,0, 
	5,30, 
	draw_object,
	clear_object,
	move_object,
	set_object_speed
};

void reset_obj_position(POBJECT ot1, POBJECT ob1, POBJECT ot2, POBJECT ob2, POBJECT bs, POBJECT bf){
	ot1->posx = 60;
	ot1->posy = -29;
	ob1->posx = 60;
	ob1->posy = 43;
	
	ot2->posx = 130;
	ot2->posy = -29;
	ob2->posx = 130;
	ob2->posy = 43;
	
	bs->posy = 30;
	bf->posy = 30;
}

void main(void){
	init_app();
	
	graphic_initialize();
	graphic_clear_screen();
	timer6_init();
	
	char *s;

	char counter[] = "Points: ";
	
	ascii_gotoxy(1,1);
	s = counter;
	
	while(*s){
		ascii_write_char(*s++);
	}
	
	short random = 0;
	int points;
	
	static POBJECT obstacle_top_1 = &obstacle_top_obj_1;
	static POBJECT obstacle_bottom_1 = &obstacle_bottom_obj_1;
	static POBJECT obstacle_top_2 = &obstacle_top_obj_2;
	static POBJECT obstacle_bottom_2 = &obstacle_bottom_obj_2;
	static POBJECT bird = &bird_soar_obj;
	static POBJECT start_screen = &starting_screen_obj;
	
	short the_y_pos_bird = bird->posy; 
	short the_y_speed_bird = bird->diry;
	
	set_obstacle_speed(-6, 0, obstacle_top_1, obstacle_bottom_1, obstacle_top_2, obstacle_bottom_2);

	while(1){
		points = 0; 
		graphic_clear_screen();
		reset_obj_position(obstacle_top_1, obstacle_bottom_1, obstacle_top_2, obstacle_bottom_2, &bird_soar_obj, &bird_flap_obj);
		start_screen->draw(start_screen);
		while(keyb_enhanced() != 5){
			bird = &bird_soar_obj;
			draw_object(bird);
			delay_milli(50000);
			clear_object(bird);
			bird = &bird_flap_obj;
			draw_object(bird);
			delay_milli(50000);
			clear_object(bird);
		}
		graphic_clear_screen();
		while(1){
			bird->move(bird);
			delay_milli(50000);
			move_obstacles(obstacle_top_1, obstacle_bottom_1, obstacle_top_2, obstacle_bottom_2);

		
			if(keyb_enhanced() == 2){
				the_y_pos_bird = bird->posy;
				the_y_speed_bird = bird->diry;
				clear_object(bird);
				bird = &bird_flap_obj;
				bird->posy = the_y_pos_bird; 
				bird->diry = the_y_speed_bird; 
				bird->diry = -4; 
				bird->move(bird);
				delay_milli(50000);
	
			} 
			if(bird->diry < 0){
				bird->diry += 2;
			}
			else if (bird->diry < 5){
				bird->diry +=3;
			}
		
			if(bird->diry >= 0){
				the_y_pos_bird = bird->posy;
				the_y_speed_bird = bird->diry;
				clear_object(bird);
				bird = &bird_soar_obj;
				bird->posy = the_y_pos_bird;
				bird->diry = the_y_speed_bird;
			}
			
			bird->move(bird);
			delay_milli(50000);
		
			if (objects_overlap(bird, obstacle_top_1, obstacle_bottom_1,obstacle_top_2, obstacle_bottom_2)){
				break;
			}
			
			if(obstacle_top_1->posx + obstacle_top_1->geo->sizex <= 0){
				++points;
				random = (char) *TIM6_CNT;
				random %= 14;
				obstacle_top_1->posx = 128;
				obstacle_bottom_1->posx = 128;
				obstacle_top_1->posy = -29 - random;
				obstacle_bottom_1->posy = 43 - random;
			}
		
			if(obstacle_top_2->posx + obstacle_top_2->geo->sizex <= 0){
				++points;
				random = (char) *TIM6_CNT; 
				random %= 14;
				obstacle_top_2->posx = 128;
				obstacle_bottom_2->posx = 128;
				obstacle_top_2->posy = -29 + random;
				obstacle_bottom_2->posy = 43 + random;
			}		
		}
	}	
}
