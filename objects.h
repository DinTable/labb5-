#include "defines.h"

typedef struct tObj{
	PGEOMETRY geo;
	int dirx, diry;
	int posx, posy;
	void (*draw) (struct tObj *);
	void (*clear) (struct tObj *);
	int (* move) (struct tObj *);
	void (* set_speed) (struct tObj *, int, int);
} OBJECT, *POBJECT;

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