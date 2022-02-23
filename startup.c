/*
 *     startup.c
 */
#include "defines.h"
#include "keypad.h"
#include "ascii"
#include "delays.h"
#include "geometries.h"
#include "objects.h"

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
    *GPIO_D_MODER = 0x55005555;
	*GPIO_E_MODER = 0x55555555;
}

void timer6_init(){
	*TIM6_CR1 &= ~CEN;
	*TIM6_ARR = 0xFFFF;
	*TIM6_CR1 |= (CEN | UDIS);
	
}

int objects_overlap( POBJECT birb, POBJECT obst_1_t, POBJECT obst_1_b, POBJECT obst_2_t, POBJECT obst_2_b){

	if(obst_1_t->posx <= birb->posx + birb->geo->sizex - 1 && !(obst_1_t->posx + obst_1_t->geo->sizex < birb->posx)){
		if(birb->posy <= obst_1_t->posy + obst_1_t->geo->sizey) return 1;
		else if(birb->posy + birb->geo->sizey - 3 >= obst_1_b->posy) return 1;
		else return 0;
	}
	
	else if(obst_2_t->posx <= birb->posx + birb->geo->sizex - 1 && !(obst_2_t->posx + obst_2_t->geo->sizex < birb->posx)){
		if(birb->posy <= obst_2_t->posy + obst_2_t->geo->sizey) return 1;
		else if(birb->posy + birb->geo->sizey - 3 >= obst_2_b->posy) return 1;  
		else return 0;
	} else return 0; 
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

void display_ctr(char pt_array[]){
	char *ptext;
	char text[] = "Points: ";
	ptext = text;
	
	ascii_gotoxy(1,1);
	while(*ptext){
		ascii_write_char(*ptext++);
	}

	ascii_gotoxy(9,1);
	while(*pt_array){
		ascii_write_char(*pt_array++);
	}
}

void point_ctr(POBJECT bird, POBJECT ob1, POBJECT ob2, _Bool *between_obj1, char pt_array[]){
	
	
	if( bird->posx > ob1->posx && bird->posx < ob1->posx+ob1->geo->sizex && *between_obj1 ){
		
		if(pt_array[1] == '9'){
			pt_array[1] = '0'-1;
			pt_array[0]++;
		}
		pt_array[1]++;
		*between_obj1 = 0;
		
	}
	else if( bird->posx > ob2->posx && bird->posx < ob2->posx+ob2->geo->sizex && !(*between_obj1) ){
		
		if(pt_array[1] == '9'){
			pt_array[1] = '0'-1;
			pt_array[0]++;
		}
		pt_array[1]++;
		*between_obj1 = 1;
		
	}
}

void randomize_obstacles(POBJECT obstacle_top_1, POBJECT obstacle_bottom_1, POBJECT obstacle_top_2, POBJECT obstacle_bottom_2, char plus){
	int random;
	if(plus){
		random = (char) *TIM6_CNT;
		random %= 14;
		obstacle_top_1->posx = 128;
		obstacle_bottom_1->posx = 128;
		obstacle_top_1->posy = -29 - random;
		obstacle_bottom_1->posy = 43 - random;
	}
	else{
		random = (char) *TIM6_CNT;
		random %= 14;
		obstacle_top_2->posx = 128;
		obstacle_bottom_2->posx = 128;
		obstacle_top_2->posy = -29 + random;
		obstacle_bottom_2->posy = 43 + random;
	}
}

void main(void){
	init_app();
	graphic_initialize(); //initierar port E enligt boken
	graphic_clear_screen();
	timer6_init();
	ascii_init();
	
	char pt_array[3] = {'0','0', '\0'};
	short random = 0;
	_Bool between_obj1 = 1;
	
	static POBJECT obstacle_top_1 = &obstacle_top_obj_1;
	static POBJECT obstacle_bottom_1 = &obstacle_bottom_obj_1;
	static POBJECT obstacle_top_2 = &obstacle_top_obj_2;
	static POBJECT obstacle_bottom_2 = &obstacle_bottom_obj_2;
	static POBJECT bird = &bird_soar_obj;
	static POBJECT start_screen = &starting_screen_obj;
	
	short the_y_pos_bird = bird->posy; 
	short the_y_speed_bird = bird->diry;
	
	set_obstacle_speed(-12, 0, obstacle_top_1, obstacle_bottom_1, obstacle_top_2, obstacle_bottom_2);

	while(1){
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
			point_ctr(bird, obstacle_bottom_1, obstacle_bottom_2, &between_obj1, pt_array);
			display_ctr(pt_array);
			
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
				bird->diry += 3;
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
				pt_array[0] = '0'; 
				pt_array[1] = '0';
				between_obj1 = 1;
				break;
			}
			
			if(obstacle_top_1->posx + obstacle_top_1->geo->sizex <= 0){
				randomize_obstacles(obstacle_top_1, obstacle_bottom_1, obstacle_top_2, obstacle_bottom_2, 1);
				
			}
		
			if(obstacle_top_2->posx + obstacle_top_2->geo->sizex <= 0){
				randomize_obstacles(obstacle_top_1, obstacle_bottom_1, obstacle_top_2, obstacle_bottom_2, 0);
				
			}		
		}
	}	
}