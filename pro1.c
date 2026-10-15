
#include <xc.h>

#define _XTAL_FREQ 20000000

#define RS RC0
#define EN RC1

//================================================
// LCD FUNCTIONS - ORIGINAL CODE
//================================================

void LCD_Command(unsigned char cmd)
{
    RS = 0;
    PORTD = cmd;
    EN = 1;
    __delay_ms(1);
    EN = 0;
}

void LCD_Data(unsigned char data)
{
    RS = 1;
    PORTD = data;
    EN = 1;
    __delay_ms(1);
    EN = 0;
}

void LCD_String(char *str)
{
    while(*str)
    {
        LCD_Data(*str);
        str++;
    }
}

void LCD_Init()
{
    TRISD = 0x00;
    __delay_ms(20);

    LCD_Command(0x38);
    LCD_Command(0x0C);
    LCD_Command(0x01);
    LCD_Command(0x06);
}

//================================================
// ADC INITIALIZATION - ORIGINAL CODE
//================================================

void ADC_Init()
{
    TRISAbits.TRISA1 = 1;
    ADCON1 = 0x8E;
    ADCON0 = 0x49;
}

//================================================
// ADC READING - ORIGINAL CODE
//================================================

unsigned int ADC_Read()
{
    __delay_us(40);

    GO_nDONE = 1;

    while(GO_nDONE);

    return ((unsigned int)ADRESH << 8) | ADRESL;
}

//================================================
// PWM INITIALIZATION - ADDED
// RC2/CCP1 (PIN 17)
//================================================

void PWM_Init()
{
    TRISCbits.TRISC2 = 0;

    PR2 = 249;

    CCP1CON = 0x0C;
    CCPR1L = 0;

    T2CON = 0x05;
}

//================================================
// SET PWM DUTY CYCLE - ADDED
//================================================

void PWM_SetDuty(unsigned int percent)
{
    unsigned int duty;

    if(percent > 100)
        percent = 100;

    duty = (percent * 1000UL) / 100;

    CCPR1L = duty >> 2;

    CCP1CON = 0x0C | ((duty & 0x03) << 4);
}

//================================================
// TEMPERATURE TO PWM CONVERSION - ADDED
// Temperature is multiplied by 10
// Example: 30.0 C = 300
//================================================

unsigned int Temperature_To_Duty(unsigned int temp10)
{
    if(temp10 <= 250)
        return 0;

    if(temp10 >= 500)
        return 100;

    return ((unsigned long)(temp10 - 250) * 4UL) / 10UL;
}

//================================================
// MAIN PROGRAM
//================================================

void main()
{
    unsigned int adcValue;
    unsigned int currentTemp10;
    unsigned int oldTemp10 = 9999;
    unsigned int dutyPercent;

    TRISC = 0x00;

    LCD_Init();
    ADC_Init();
    PWM_Init();

    LCD_Command(0x01);
    __delay_ms(2);

    LCD_String("Temperature:");

    while(1)
    {
        // Read temperature sensor
        adcValue = ADC_Read();

        // Convert ADC value to temperature x 10
        currentTemp10 =
            ((unsigned long)adcValue * 5000UL + 511UL)
            / 1023UL;

        // Calculate fan PWM duty cycle
        dutyPercent = Temperature_To_Duty(currentTemp10);

        // Update PWM output
        PWM_SetDuty(dutyPercent);

        // Display temperature on LCD
        if(currentTemp10 != oldTemp10)
        {
            LCD_Command(0xC0);

            LCD_String("Temp: ");

            LCD_Data('0' + (currentTemp10 / 100) % 10);
            LCD_Data('0' + (currentTemp10 / 10) % 10);
            LCD_Data('.');
            LCD_Data('0' + currentTemp10 % 10);

            LCD_String(" C   ");

            oldTemp10 = currentTemp10;
        }

        __delay_ms(100);
    }
}
