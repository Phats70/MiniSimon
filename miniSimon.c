//********************************************************
//** Project        : miniSimon
//** Date           : 2019/06/14
//********************************************************																				
//** Author         : TheoS
//********************************************************
//** Chip type		: PIC16F15323
//** Clock Frequency: 4.0000MHz (Internal OSC)
//** PCB Name       : N/A
//********************************************************
//========================================================
//** Revision History
//========================================================
//** V1.00	: Original
//**
//********************************************************
//**	 		Compiler Header Files					**
//********************************************************
#include <xc.h>
#include "miniSimon.h"
#include <stdint.h>

//**====================================================**
//**	 		PIC16F15323 Configuration Bits			**
//**====================================================**
// CONFIG1
#pragma config FEXTOSC = OFF        // External Oscillator mode selection bits->Oscillator not enabled
#pragma config RSTOSC = HFINT1      // Power-up default value for COSC bits->HFINTOSC (1MHz)
#pragma config CLKOUTEN = OFF       // Clock Out Enable bit->CLKOUT function is disabled; i/o or oscillator function on OSC2
#pragma config CSWEN = ON           // Clock Switch Enable bit->Writing to NOSC and NDIV is allowed
#pragma config FCMEN = ON           // Fail-Safe Clock Monitor Enable bit->FSCM timer enabled

// CONFIG2
#pragma config MCLRE = ON           // Master Clear Enable bit->MCLR pin is Master Clear function
#pragma config PWRTE = OFF          // Power-up Timer Enable bit->PWRT disabled
#pragma config LPBOREN = OFF        // Low-Power BOR enable bit->ULPBOR disabled
#pragma config BOREN = ON           // Brown-out reset enable bits->Brown-out Reset Enabled, SBOREN bit is ignored
#pragma config BORV = LO            // Brown-out Reset Voltage Selection->Brown-out Reset Voltage (VBOR) set to 1.9V on LF, and 2.45V on F Devices
#pragma config ZCD = OFF            // Zero-cross detect disable->Zero-cross detect circuit is disabled at POR.
#pragma config PPS1WAY = ON         // Peripheral Pin Select one-way control->The PPSLOCK bit can be cleared and set only once in software
#pragma config STVREN = ON          // Stack Overflow/Underflow Reset Enable bit->Stack Overflow or Underflow will cause a reset

// CONFIG3
#pragma config WDTCPS = WDTCPS_31   // WDT Period Select bits->Divider ratio 1:65536; software control of WDTPS
#pragma config WDTE = OFF           // WDT operating mode->WDT Disabled, SWDTEN is ignored
#pragma config WDTCWS = WDTCWS_7    // WDT Window Select bits->window always open (100%); software control; keyed access not required
#pragma config WDTCCS = SC          // WDT input clock selector->Software Control

// CONFIG4
#pragma config BBSIZE = BB512       // Boot Block Size Selection bits->512 words boot block size
#pragma config BBEN = OFF           // Boot Block Enable bit->Boot Block disabled
#pragma config SAFEN = OFF          // SAF Enable bit->SAF disabled
#pragma config WRTAPP = OFF         // Application Block Write Protection bit->Application Block not write protected
#pragma config WRTB = OFF           // Boot Block Write Protection bit->Boot Block not write protected
#pragma config WRTC = OFF           // Configuration Register Write Protection bit->Configuration Register not write protected
#pragma config WRTSAF = OFF         // Storage Area Flash Write Protection bit->SAF not write protected
#pragma config LVP = OFF            // Low Voltage Programming Enable bit->High Voltage on MCLR/Vpp must be used for programming

// CONFIG5
#pragma config CP = OFF             // UserNVM Program memory code protection bit->UserNVM code protection disabled


//********************************************************
//**	 	Global Variables                            **
//********************************************************
uint8_t LevelColors, RandomColorCount, ColorHistory[64], ColorPressed;
uint8_t LastSuccessColors[64], SuccessiveGamesWon;
uint8_t UserInputCount, CurrentLevel, LevelDelay;

volatile uint8_t RND;


TMR_t TMR;
Flags_t Flags;
smSimon_t Simon;



//********************************************************
//**	 	Main Software Routine						**
//********************************************************
void main(void)
{
    uint8_t x;

    Init_PIC();                                                                 // Configure The PIC

    TMR.ShowColour = 0;
    Simon = 0;
    Flags.Bits = 0;
    Flags.CheckInputs = 1;
    SuccessiveGamesWon = 0;
    CurrentLevel = StartLevel;

    TMR.Delay = 25;
    while(TMR.Delay);
	
    while(1)
    {
        Flags.CheckInputs = 1;
        
        switch(Simon)
        {
            case stStartUp:
                
                Buzzer_PWR_On();
                LevelColors = StartUp_Select();

                if(LevelColors != 255)
                {
                    LevelColors = CurrentLevel;
                    Simon++;
                }
                else
                    Simon = stSleep;
            break;

            case stStartGame:

                for(x = 0; x < sizeof(ColorHistory); x++)                       // Clear History
                    ColorHistory[x] = 255;

                RandomColorCount = 0;                                           // Start From Beginning
                Simon++;
            break;
            
            case stAddRandomColor:

                if(RandomColorCount == LevelColors)
                    Simon = stSuccess;
                else
                {
                    LevelDelay = dlyLevel_1;

                    if(RandomColorCount)                                        // Any Previous Colors?
                    {                                                           // Yes....
                        if(RandomColorCount > 20)
                            LevelDelay = dlyLevel_3;
                        
                        else if(RandomColorCount > 10)
                            LevelDelay = dlyLevel_2;

                        for(x = 0; x < RandomColorCount; x++)                   // Display All Previous Colors
                            ShowColors(ColorHistory[x], LevelDelay);
                    }

                    ColorHistory[RandomColorCount] = RND;                       // Add New Color To History
                    ShowColors(ColorHistory[RandomColorCount++], LevelDelay);   // Display New Color
                    TMR.Response = 200;                                         // Set Response Time From Player
                    UserInputCount = 0;
                    Simon++;
                }
            break;

            case stUserKeyInput:

                if(TMR.Response)                                                // Time Left For Player To Respond?
                {                                                               // Yes....
                    if(Flags.Bits & 0x0F)                                       // Any Buttons Pressed?
                    {                                                           // Yes...
                        ColorPressed = Flags.Bits & 0x0F;                       // Keep Key Presses Only

                        Flags.KeyPressed = 0;                                   // Assume No Errors
                        Flags.KeyError = 0;

                        if(ColorPressed & 0x01)                                 // Blue Pressed?
                            Flags.KeyPressed = 1;                               // Yes....

                        if(Flags.KeyPressed && (ColorPressed & 0x02))           // More That One Key Pressed?
                            Flags.KeyError = 1;                                 // Yes....

                        if(Flags.KeyPressed && (ColorPressed & 0x04))           // More Than One Key Pressed?
                            Flags.KeyError = 1;                                 // Yes....

                        if(Flags.KeyPressed && (ColorPressed & 0x08))           // More Than One Key Pressed?
                            Flags.KeyError = 1;                                 // Yes....

                        if(!Flags.KeyError)                                     // Double Key Presses Detected?
                        {                                                       // No....
                            if(Flags.ipBlue)
                            {
                                Sound_On(Tone_Blue);
                                BlueLED = LEDon;
								ColorPressed = Color_Blue;
                            }
                            else if(Flags.ipAmber)
                            {
                                Sound_On(Tone_Amber);
                                AmberLED = LEDon;
								ColorPressed = Color_Amber;
                            }
                            else if(Flags.ipRed)
                            {
                                Sound_On(Tone_Red);
								RedLED = LEDon;
                                ColorPressed = Color_Red;
                            }
                            else
                            {
                                Sound_On(Tone_Green);
								GreenLED = LEDon;
                                ColorPressed = Color_Green;
                            }
                            TMR.Response = 100;                                 // Set Time For Player To Release Key
                            Simon++;
                        }
                        else
                            Simon = stError;
                    }
                }
                else
                    Simon = stError;
            break;

            case stUserKeyRelease:

                if(Flags.Bits & 0x0F)                                           // Key Pressed Released?
                {                                                               // No....
                    if(TMR.Response == 0)                                       // Time Left For Player To Release Key?
                        Simon = stError;                                        // No, Error....
                }
                else
                {
                    Sound_Off();
					LEDs_All_Off();
                    Simon++;                                                    // Key Pressed Released!
                }
            break;

            case stUserInputSequence:

                if(ColorHistory[UserInputCount++] != ColorPressed)              // Correct Color Pressed?
                    Simon = stError;                                            // No....
                else
                {
                    if(RandomColorCount <= LevelColors)                         // End Of Game?
                    {                                                           // No....
                        if(UserInputCount < RandomColorCount)                   // More Colors To Be Input By Player?
                        {                                                       // Yes....
                            TMR.Response = 200;
                            Simon = stUserKeyInput;
                        }
                        else
                        {
                            TMR.Response = 100;
                            Simon = stWait;
                        }
                    }
                    else
                        Simon = stSuccess;
                }
            break;

            case stWait:

                if(TMR.Response == 0)
                    Simon = stAddRandomColor;
            break;

            case stSuccess:

                for(x = 0; x < 5; x++)
                {
                   ShowColors(Color_Blue, dlySuccess);
                   ShowColors(Color_Amber, dlySuccess);
                   ShowColors(Color_Red, dlySuccess);
                   ShowColors(Color_Green, dlySuccess);
                }

                for(x = 0; x < sizeof(LastSuccessColors); x++)
                    LastSuccessColors[x] = ColorHistory[x];

                if(SuccessiveGamesWon < 20)
                    SuccessiveGamesWon++;

                if(CurrentLevel < 10)
                    CurrentLevel++;

                else if(CurrentLevel < 16)
                    CurrentLevel += 2;

                else if(CurrentLevel < 32)
                    CurrentLevel += 4;
                else    
                    CurrentLevel = 32;

                Flags.LastSuccess = 1;
                Simon = stSleep;
            break;

            case stError:
               
                for(x = 0; x < 4; x++)
                    ShowColors(ColorPressed, dlyError);

                Simon = stSleep;

            case stSleep:

                Buzzer_PWR_Off();
                LEDs_All_Off();
                
                INTCONbits.GIE = 0;									// Global Interrupts Disabled
                INTCONbits.PEIE = 0;									// All Peripheral Interrupts Disabled

                T0CON0bits.T0EN = 0;
                T2CONbits.ON = 0;
              
                SLEEP();
                   
                T0CON0bits.T0EN = 1;
                T2CONbits.ON = 1;
                
                INTCONbits.GIE = 1;										// Global Interrputs Enabled
                INTCONbits.PEIE = 1;									// All Peripheral Interrupts Enabled
                Simon = stStartUp;
            break;

            default:
                Simon = stError;
            break;
        }
    }
}


//********************************************************
//**	 	Display The Random Colors  				**
//********************************************************
uint8_t StartUp_Select(void)
{
    uint8_t result = 0;

    if(Flags.ipBlue)                                                            // Level 1?
        result = 6;                                                             // Yes.....

    else if(Flags.ipRed)                                                        // Display Previous Routine?
        result = 255;                                                           // Yes....
    else
        return(255);

    TMR.Delay = 50;

    while(TMR.Delay)
    {
        if(Flags.Bits & 0x0F)
            TMR.Delay = 50;
    }
    
    if(result == 255)
        Show_LastSequence();

    return(result);                                                             // Error, Go Sleep
}


//********************************************************
//**	 	Enable PWM Sound Generation     			**
//********************************************************
void Sound_On(uint8_t SoundType)
{
    uint8_t Period;
	uint16_t dutyValue;
    
    switch(SoundType)
    {
        case Tone_Blue:
            T2CON = 0xA0;
			Period = Tone_Blue_415Hz;
        break;

        case Tone_Amber:
			T2CON = 0xA0;
            Period = Tone_Amber_350Hz;
        break;

        case Tone_Red:
			T2CON = 0xA0;
            Period = Tone_Red_300Hz;
        break;

        case Tone_Green:
			T2CON = 0xA0;
            Period = Tone_Green_250Hz;
        break;

        case Tone_Error:
			T2CON = 0xD0;
            Period = Tone_Error_45Hz;
        break;
    }

    T2PR = Period;
    
    dutyValue = ((uint16_t)Period / 2) & 0x03FF;                // 12.5% Duty Cycle. (Period * 2 = 50%, Period * 1 = 25%)
    
    if(CCP1CONbits.CCP1FMT)
    {
        CCPR1H = dutyValue >> 2;
        CCPR1L = (dutyValue << 6) & 0xC0;
    }
    else
    {
        CCPR1H = (dutyValue >> 8) & 0x03;
        CCPR1L = dutyValue;
    }
      
}


//********************************************************
//**	 	Disable PWM Sound Generation     			**
//********************************************************
void Sound_Off(void)
{
    CCPR1H = 0;
    CCPR1L = 0;
}


//********************************************************
//**	 	Display The Last Level Completed			**
//********************************************************
void Show_LastSequence(void)
{
    uint8_t x;
    
    if(SuccessiveGamesWon)
    {
        for(x = 0; x < SuccessiveGamesWon; x++)
        {
            ShowColors(Color_GamesWon, dlyLevel_1);
            
            TMR.Delay = 25;
            while(TMR.Delay > 0);
        }
        TMR.Delay = 100;
        while(TMR.Delay > 0);
    }

    if(!Flags.LastSuccess)                                                      // Any Games Won?
        return;                                                                 // No....

    for(x = 0; x < sizeof(LastSuccessColors); x++)                              // Display Colors Of Last Game Won
        ShowColors(LastSuccessColors[x], dlyLevel_1);
}


//********************************************************
//**	 	Display The Random Colors					**
//********************************************************
uint8_t ShowColors(Color_t color, uint8_t delay)
{
    if(color > Color_End)
        return(255);

    Flags.CheckInputs = 0;

    switch(color)
    {
        case Color_Blue:

            BlueLED = LEDon;
            Sound_On(Tone_Blue);
        break;

        case Color_Amber:

            AmberLED = LEDon;
            Sound_On(Tone_Amber);
        break;

        case Color_Red:

            RedLED = LEDon;
            Sound_On(Tone_Red);
        break;

        case Color_Green:

            GreenLED = LEDon;
            Sound_On(Tone_Green);
        break;

        case Color_Error:

            BlueLED = LEDon;
            AmberLED = LEDon;
            RedLED = LEDon;
            GreenLED = LEDon;

            Sound_On(Tone_Error);
        break;

        case Color_GamesWon:
		case Color_End:

            BlueLED = LEDon;
            AmberLED = LEDon;
        break;
    }

    TMR.ShowColour = delay;
    while(TMR.ShowColour);
    Sound_Off();

	BlueLED = LEDoff;
    AmberLED = LEDoff;
    RedLED = LEDoff;
    GreenLED = LEDoff;
    
    TMR.ShowColour = 6;
    while(TMR.ShowColour);

    Flags.CheckInputs = 1;
    return(0);
}


//********************************************************
//**	 	uProcessor Initialize Routine				**
//********************************************************
void Init_PIC(void)
{
	    // CLKRMD CLKR enabled; SYSCMD SYSCLK enabled; FVRMD FVR enabled; IOCMD IOC enabled; NVMMD NVM enabled; 
    PMD0 = 0x00;
    // TMR0MD TMR0 enabled; TMR1MD TMR1 enabled; TMR2MD TMR2 enabled; NCOMD DDS(NCO) enabled; 
    PMD1 = 0x00;
    // ZCDMD ZCD enabled; CMP1MD CMP1 enabled; ADCMD ADC enabled; CMP2MD CMP2 enabled; DAC1MD DAC1 enabled; 
    PMD2 = 0x00;
    // CCP2MD CCP2 enabled; CCP1MD CCP1 enabled; PWM4MD PWM4 enabled; PWM3MD PWM3 enabled; PWM6MD PWM6 enabled; PWM5MD PWM5 enabled; 
    PMD3 = 0x00;
    // CWG1MD CWG1 enabled; MSSP1MD MSSP1 enabled; UART1MD EUSART enabled; 
    PMD4 = 0x00;
    // CLC3MD CLC3 enabled; CLC4MD CLC4 enabled; CLC1MD CLC1 enabled; CLC2MD CLC2 enabled; 
    PMD5 = 0x00;
	
    LATA = 0x00;
    LATC = 0x00;

    TRISA = PORTAconfig;
    TRISC = PORTCconfig;

    ANSELC = 0x00;
    ANSELA = 0x00;

    WPUA = 0x17;
    WPUC = 0x00;

    ODCONA = 0x00;
    ODCONC = 0x00;

    SLRCONA = 0x3F;
    SLRCONC = 0x3F;

    IOCAFbits.IOCAF0 = 0;                   //interrupt on change for group IOCAF - flag
    IOCAFbits.IOCAF1 = 0;
    IOCAFbits.IOCAF2 = 0;
    IOCAFbits.IOCAF4 = 0;
    
    IOCANbits.IOCAN0 = 1;                  //interrupt on change for group IOCAN - negative
    IOCANbits.IOCAN1 = 1;
    IOCANbits.IOCAN2 = 1;
    IOCANbits.IOCAN4 = 1;

    IOCAPbits.IOCAP0 = 1;                   //interrupt on change for group IOCAP - positive
    IOCAPbits.IOCAP1 = 1;
    IOCAPbits.IOCAP2 = 1;
    IOCAPbits.IOCAP4 = 1;   
     
    PIE0bits.IOCIE = 1;                     // Enable IOCI interrupt    
	
    RC5PPS = 0x09;							//RC5->CCP1:CCP1;   		
    

    // NOSC HFINTOSC; NDIV 4; 
    OSCCON1 = 0x62;
    // CSWHOLD may proceed; 
    OSCCON3 = 0x00;
    // MFOEN disabled; LFOEN disabled; ADOEN disabled; EXTOEN disabled; HFOEN disabled; 
    OSCEN = 0x00;
    // HFFRQ 4_MHz; 
    OSCFRQ = 0x02;
    // MFOR not ready; 
    OSCSTAT = 0x00;
    // HFTUN 0; 
    OSCTUNE = 0x00;	
	
	
	// MODE PWM; EN enabled; CCP1FMT right_aligned; 
	CCP1CON = 0x8F;    	     
	CCPR1H = 0x00;    
	CCPR1L = 0x00;    

    T2CLKCON = 0x01;					// T2CS FOSC/4; 
    T2HLT = 0x00;						// T2PSYNC Not Synchronized; T2MODE Software control; T2CKPOL Rising Edge; T2CKSYNC Not Synchronized; 
    T2RST = 0x00;						// T2RSEL T2INPPS pin; 
    T2PR = 171;							// T2PR 171; 
    T2TMR = 0x00;						// TMR2 0; 
    
	PIR4bits.TMR2IF = 0;				// Clearing IF flag.
    T2CON = 0xD0;						// T2CKPS 1:32; T2OUTPS 1:1; TMR2ON On; 	
	

    // T0CS FOSC/4; T0CKPS 1:1; T0ASYNC synchronised; 
    T0CON1 = 0x40;
    // TMR0H 246; 
    TMR0H = 0xF6;
    // TMR0L 60; 
    TMR0L = 0x3C;

    // Clear Interrupt flag before enabling the interrupt
    PIR0bits.TMR0IF = 0;

    // Enabling TMR0 interrupt.
    PIE0bits.TMR0IE = 1;

    // T0OUTPS 1:1; T0EN enabled; T016BIT 16-bit; 
    T0CON0 = 0x90;	

	INTCONbits.PEIE = 1;									// All Peripheral Interrupts Enabled
	INTCONbits.GIE = 1;										// Global Interrupts Enabled
}



//********************************************************
//**	 	Interrupt Service Routine					**
//********************************************************
void __interrupt() InterruptHandler(void)
{
    if(PIE0bits.TMR0IE == 1 && PIR0bits.TMR0IF == 1)
	{
		TMR0H = TMR_10ms_H;                                                     // Reload TMR1 For 10msec
		TMR0L = TMR_10ms_L;

        if(TMR.Delay)
            TMR.Delay--;

        if(TMR.ShowColour)
            TMR.ShowColour--;

        if(TMR.Response)
            TMR.Response--;

        if(RND < 3)
            RND++;
        else
            RND = 0;

        if(Flags.CheckInputs)
        {

        }
		PIR0bits.TMR0IF = 0;
	}
    else if(PIE0bits.IOCIE == 1 && PIR0bits.IOCIF == 1)
    {
        if(IOCAFbits.IOCAF0 == 1)
        {
            IOCAFbits.IOCAF0 = 0;
			
			if (RedIP_Pin)
				Flags.ipRed = 0;
			else
				Flags.ipRed = 1;
        }	
        if(IOCAFbits.IOCAF1 == 1)
        {
            IOCAFbits.IOCAF1 = 0;
			
			if (AmberIP_Pin)
				Flags.ipAmber = 0;
			else
				Flags.ipAmber = 1;
        }	
        if(IOCAFbits.IOCAF2 == 1)
        {
            IOCAFbits.IOCAF2 = 0;
			
			if (BlueIP_Pin)
				Flags.ipBlue = 0;
			else
				Flags.ipBlue = 1;
        }	
        if(IOCAFbits.IOCAF4 == 1)
        {
			IOCAFbits.IOCAF4 = 0;  
            
			if (GreenIP_Pin)
				Flags.ipGreen = 0;
			else
				Flags.ipGreen = 1;
        }	
    }
}


