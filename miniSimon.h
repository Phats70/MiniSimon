#ifndef MINI_SIMON_H
#define MINI_SIMON_H
//********************************************************
//** Project        : miniSimon
//** Date           : 2019/06/14
//********************************************************
//** Author         : TheoS
//********************************************************
#include <stdint.h>

//********************************************************
//**	 	User Program Constants						**
//********************************************************
#define dlyLevel_1				42
#define dlyLevel_2				32
#define dlyLevel_3				22
#define dlyError				10
#define dlySuccess				5


#define StartLevel				5

//********************************************************
//**	 	Program Constants							**
//********************************************************
#define DbTreset				5

#define _XTAL_FREQ				4000000											// Required By __delay_us & __delay_ms
#define TMR_10ms_H				0xF6
#define TMR_10ms_L				0x3C

#define LEDon					1
#define LEDoff					0

#define PINoutput				0
#define PINinput				1

#define Tone_Blue				0
#define Tone_Amber				1
#define Tone_Red				2
#define Tone_Green				3
#define Tone_Error				4

#define Tone_Blue_415Hz			0x96		
#define Tone_Amber_350Hz		0xB1
#define Tone_Red_300Hz			0xCF
#define Tone_Green_250Hz		0xF9
#define Tone_Error_45Hz			0xAB

#define Buzzer_PWR_Off()		LATAbits.LATA5 = 0
#define Buzzer_PWR_On()         LATAbits.LATA5 = 1

#define LEDs_All_Off()          LATC &= 0xF0

typedef volatile struct
{
    uint8_t  ShowColour;
    uint16_t Delay;
    uint16_t Response;

}TMR_t;


typedef volatile union
{
    uint8_t Bits;

    struct
    {
        unsigned ipBlue:1;
        unsigned ipAmber:1;
        unsigned ipRed:1;
        unsigned ipGreen:1;
        unsigned CheckInputs:1;
        unsigned KeyPressed:1;
        unsigned KeyError:1;
        unsigned LastSuccess:1;
    };
}Flags_t;


typedef enum
{
	stStartUp,
	stStartGame,
	stAddRandomColor,
	stUserKeyInput,
	stUserKeyRelease,
    stUserInputSequence,
	stWait,
	stSuccess,
	stError,
	stSleep

}smSimon_t;


typedef enum
{
	Color_Blue,
	Color_Amber,
	Color_Red,
	Color_Green,
	Color_Error,
	Color_GamesWon,
	Color_End

}Color_t;


//********************************************************
//**	 	PIC Hardware Definitions					**
//********************************************************
#define RedIP_Pin			PORTAbits.RA0
#define RedIP_WPU			WPUAbits.WPUA0
#define RedIP_IOC			IOCAbits.IOCA0

#define AmberIP_Pin			PORTAbits.RA1
#define AmberIP_WPU			WPUAbits.WPUA1
#define AmberIP_IOC			IOCAbits.IOCA1

#define BlueIP_Pin			PORTAbits.RA2
#define BlueIP_WPU			WPUAbits.WPUA2
#define BlueIP_IOC			IOCAbits.IOCA2

// MCLR Pin					RA3

#define GreenIP_Pin			PORTAbits.RA4
#define GreenIP_WPU			WPUAbits.WPUA4
#define GreenIP_IOC			IOCAbits.IOCA4

// Buzzer_Pwr               RA5

#define PORTAconfig			0b11011111


#define RedLED				PORTCbits.RC0
#define AmberLED			PORTCbits.RC1
#define GreenLED			PORTCbits.RC2
#define BlueLED				PORTCbits.RC3
// Not Used					PORTCbits.RC4

#define PWMout_Pin			TRISCbits.TRISC5

#define PORTCconfig			0b11010000



//********************************************************
//**	 	Function Prototypes							**
//********************************************************
void Init_PIC(void);

uint8_t ShowColors(Color_t color, uint8_t delay);

uint8_t StartUp_Select(void);
void Show_LastSequence(void);

void Sound_On(uint8_t Hz);
void Sound_Off(void);

#endif

