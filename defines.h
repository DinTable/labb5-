#define GPIO_D 			0x40020C00
#define GPIO_D_MODER 	((volatile unsigned int *) (GPIO_D)) 
#define GPIO_D_OTYPER 	((volatile unsigned int *) (GPIO_D+0x4))
#define GPIO_D_OSPEEDR 	((volatile unsigned int *) (GPIO_D+0x8))
#define GPIO_D_PUPDR 	((volatile unsigned int *) (GPIO_D+0xC))
#define GPIO_D_IDR_LOW 	((volatile unsigned char *) (GPIO_D+0x10))
#define GPIO_D_IDR_HIGH	((volatile unsigned char *) (GPIO_D+0x11))
#define GPIO_D_ODR_LOW 	((volatile unsigned char *) (GPIO_D+0x14))
#define GPIO_D_ODR_HIGH 	((volatile unsigned char *) (GPIO_D+0x15))

#define GPIO_E 0x40021000 
#define GPIO_E_MODER ((volatile unsigned int *) (GPIO_E)) 
#define GPIO_E_OTYPER ((volatile unsigned int *) (GPIO_E+0x4))
#define GPIO_E_OSPEEDR ((volatile unsigned int *) (GPIO_E+0x8))
#define GPIO_E_PUPDR ((volatile unsigned int *) (GPIO_E+0xC))
#define GPIO_E_IDR_LOW ((volatile unsigned char *) (GPIO_E+0x10))
#define GPIO_E_IDR_HIGH ((volatile unsigned char *) (GPIO_E+0x11))
#define GPIO_E_ODR_LOW ((volatile unsigned char *) (GPIO_E+0x14))
#define GPIO_E_ODR_HIGH ((volatile unsigned char *) (GPIO_E+0x15))

#define STK 			0xE000E010
#define STK_CTRL		((volatile unsigned int *) (STK))
#define STK_LOAD		((volatile unsigned int *) (STK+0x4))
#define STK_VAL 		((volatile unsigned int *) (STK+0x8))
#define CALIB 			((volatile unsigned int *) (STK+0xC))

#define B_E 			0x40 // 
#define B_SELECT		4 //Select sätts alltid till 1 vid använding av ascii displayen
#define B_NSELECT		0 // grafisk display
#define B_RW			2 //Read/Write, sätts till 1 vid läsning
#define B_RS			1 //Denna bit ska vara 0 om kommando skrivs till eller status läses från displayen. 1 om data skrivs till eller läses från displayen.

#define FUNCTION_SET 	0x30 //Dessa definitioner är enligt kommandotabellen i Rogers support klipp
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