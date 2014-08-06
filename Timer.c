/*** Kontroller: m8515 ***/

/*** Include Files ***/
#include "E:\M242\C\Include\Display.c"
#include "E:\M242\C\Include\Matrix.c"
#include <avr/io.h>
#include <util/delay.h>
#include "E:\M242\C\Timer\Ausgabe.h"
#include <avr/interrupt.h>

/*** DEFINES ***/
#define GO_EBENE 1          //Parameter Ebene
#define SET_CURS 2          //Cursor um 1 Stelle nach Oben
#define ADD_1 3              //Variable += 1
#define SUB_1 4             //Variable -= 1
/*** Konstanten ***/
unsigned char ss = 40;
unsigned char mm = 56;
unsigned char hh = 23;
unsigned char dd = 03;
unsigned char MM = 04;
unsigned char YYYY = 14;
unsigned char ms = 0;
unsigned char Ebene = 0;
unsigned char M;
unsigned char X;
unsigned char Y;
unsigned char Parameter;
unsigned char Status;
unsigned char Taste_vorher;
void ReplaceText(unsigned char* Text);
void f_InitializeOverflowInterrupt();

/*** Zeilenlänge der Funktionstabelle ***/
#define TABELLEMAX 10

/*** MODUS *** 
* 0 = No Cursor
* 1 = Blink
* 2 = Choose '>'
*/

/*** Funktionstabelle ***/
char funktionstabelle[13][10] = {{GO_EBENE,1,GO_EBENE,1,GO_EBENE,1,GO_EBENE,1,GO_EBENE,1},
     // Funktionszeile 1 (Ebene = 1; Modus: 0; XPOS = 0; YPOS = 0)
     {GO_EBENE,2,GO_EBENE,2,GO_EBENE,2,GO_EBENE,2,GO_EBENE,2},
     // Funktionszeile 2 (Ebene = 2; Modus: 2; XPOS = 0; YPOS = 1)
     {0,0,0,0,GO_EBENE,3,0,0,SET_CURS,0b10000000},
     // Funktionszeile 3 (Ebene = 2; Modus: 2; XPOS = 0; YPOS = 2)
     {SET_CURS,0b01000000,0,0,GO_EBENE,1,0,0,0,0},
     // Funktionszeile 4 (Ebene = 3; Modus: 2; XPOS = 5; YPOS = 1)
     {0,0,0,0,GO_EBENE,4,0,0,SET_CURS,0b10000101},
     // Funktionszeile 5 (Ebene = 3; Modus: 2; XPOS = 5; YPOS = 2)
     {SET_CURS,0b01000101,0,0,GO_EBENE,5,0,0,SET_CURS,0b11000101},
     // Funktionszeile 6 (Ebene = 3; Modus: 2; XPOS = 5; YPOS = 3)
     {SET_CURS,0b10000101,0,0,GO_EBENE,2,0,0,0,0},
		 // Funktionszeile 7 (Ebene = 4 Modus: 1; XPOS = 4; YPOS = 2)
		 {ADD_1,0,0,0,GO_EBENE,3,SET_CURS,0b10000111,SUB_1,0},
		 // Funktionszeile 8 (Ebene = 4 Modus: 1; XPOS = 7; YPOS = 2)
		 {ADD_1,1,SET_CURS,0b10000100,GO_EBENE,3,SET_CURS,0b10001010,SUB_1,1},
		 // Funktionszeile 9 (Ebene = 4 Modus: 1; XPOS = 10; YPOS = 2)
		 {ADD_1,2,SET_CURS,0b10000111,GO_EBENE,3,0,0,SUB_1,2},
		 // Funktionszeile 10 (Ebene = 5 Modus: 1; XPOS = 3; YPOS = 2)
		 {ADD_1,3,0,0,GO_EBENE,3,SET_CURS,0b10000110,SUB_1,3},
		 // Funktionszeile 11 (Ebene = 5 Modus: 1; XPOS = 6; YPOS = 2)
		 {ADD_1,4,SET_CURS,0b10000011,GO_EBENE,3,SET_CURS,0b10001011,SUB_1,4},
		 // Funktionszeile 12 (Ebene = 5 Modus: 1; XPOS = 11; YPOS = 2)
		 {ADD_1,5,SET_CURS,0b10000110,GO_EBENE,3,0,0,SUB_1,5}};

char GO_EBENE_VAR[10][3] = {
			//Ebene 0
			{0,0,0},
			//Ebene 1
			{0,0,0},
			//Ebene 2
			{2,0,1},
			//Ebene 3
			{2,5,1},
			//Ebene 4
			{1,4,2},
			//Ebene 5
			{1,3,2}};

//Funktionensdefinitionen
unsigned char getfunktion();
void dofunktion(unsigned char Funktion);
unsigned char getTaste(unsigned char Funktionszeile);

int main(){
	unsigned char Funktionszeile;
	unsigned char Funktion;	

	f_InitializeOverflowInterrupt();
	LCD_Init();
	LCD_STR(EBENE0);	
	CURB_OFF();
	M = 0;
	
	while(1){
		Funktionszeile = getfunktion();
		Funktion = getTaste(Funktionszeile);
		if(Funktion != 0){
			dofunktion(Funktion);
			Bildschirmausgabe();
		}
	}
	return (0);
}


//Bestimmung der Zeilenposition für Funktionstabelle

//Vergleichswerte zur Bestimmung der Zeilenposition in Funktionstabelle
//<M> = Modus
//<Ebene> = Ebene
//<X> = XPosition
//<Y> = YPosition
unsigned char getfunktion(){
	unsigned char Funktion;
	
	//Funktionszeile 0
	if(Ebene == 0 && X == 0 && Y == 0){
		Funktion = 0;
	}
	
	//Funktionszeile 1
	else if(Ebene == 1 && X == 0 && Y == 0){
		Funktion = 1;
	}
	
	//Funktionszeile 2
	else if(Ebene == 2 && X == 0 && Y == 1){
		Funktion = 2;
	}
	
	//Funktionszeile 3
	else if(Ebene == 2 && X == 0 && Y == 2)
	{
		Funktion = 3;
	}
	//Funktionszeile 4
	else if(Ebene == 3 && X == 5 && Y == 1)
	{
		Funktion = 4;
	}
	//Funktionszeile 5
	else if(Ebene == 3 && X == 5 && Y == 2)
	{
		Funktion = 5;
	}
	//Funktionszeile 6
	else if(Ebene == 3 && X == 5 && Y == 3)
	{
		Funktion = 6;
	}
	//Funktionszeile 7
	else if(Ebene == 4 && X == 4 && Y == 2)
	{
		Funktion = 7;
	}
	//Funktionszeile 8
	else if(Ebene == 4 && X == 7 && Y == 2)
	{
		Funktion = 8;
	}
	//Funktionszeile 9
	else if(Ebene == 4 && X == 10 && Y == 2)
	{
		Funktion = 9;
	}
	//Funktionszeile 10
	else if(Ebene == 5 && X == 3 && Y == 2)
	{
		Funktion = 10;
	}
	//Funktionszeile 11
	else if(Ebene == 5 && X == 6 && Y == 2)
	{
		Funktion = 11;
	}
	//Funktionszeile 12
	else if(Ebene == 5 && X == 11 && Y == 2)
	{
		Funktion = 12;
	}
	return Funktion;

}




/***********************************************************************************
*dofunktion
***********************************************************************************/
void dofunktion(unsigned char Funktion){
	if(Funktion == GO_EBENE){
		Ebene = Parameter;
		M = GO_EBENE_VAR[Ebene][0];
		X = GO_EBENE_VAR[Ebene][1];
		Y = GO_EBENE_VAR[Ebene][2];
	}
	else if(Funktion == SET_CURS){
		X = Parameter & 0b00111111;
		Y = Parameter >> 6;
	}
	else if(Funktion == ADD_1){
		switch(Parameter)
		{
			case 0:{
				hh++;
				break;
			}
			case 1:{
				mm++;
				break;
			}
			case 2:{
				ss++;
				break;
			}
			case 3:{
				dd++;
				break;
			}
			case 4:{
				MM++;
				break;
			}
			case 5:{
				YYYY++;
				break;
			}
		}
	}
	else if(Funktion == SUB_1){
		switch(Parameter)
		{
			case 0:{
				hh--;
				break;
			}
			case 1:{
				mm--;
				break;
			}
			case 2:{
				ss--;
				break;
			}
			case 3:{
				dd--;
				break;
			}
			case 4:{
				MM--;
				break;
			}
			case 5:{
				YYYY--;
				break;
			}
		}
	}
}

unsigned char getTaste(unsigned char Funktionszeile){
	unsigned char Taste = GET_TASTE();
	if(Taste == Taste_vorher){
		Taste = 0;
	}
	else{
		Taste_vorher = Taste;
	}
	unsigned char Funktion = 0;
	
	switch(Taste){
		case 2:{
			Funktion = funktionstabelle[Funktionszeile][0];
			Parameter = funktionstabelle[Funktionszeile][1];
			break;
		}
		case 5:{
			Funktion = funktionstabelle[Funktionszeile][2];
			Parameter = funktionstabelle[Funktionszeile][3];
			break;
		}
		case 6:{
			Funktion = funktionstabelle[Funktionszeile][4];
			Parameter = funktionstabelle[Funktionszeile][5];
			break;
		}
		case 7:{
			Funktion = funktionstabelle[Funktionszeile][6];
			Parameter = funktionstabelle[Funktionszeile][7];
			break;
		}
		case 10:{
			Funktion = funktionstabelle[Funktionszeile][8];
			Parameter = funktionstabelle[Funktionszeile][9];
			break;
		}
	}
	return Funktion;
}

/***********************************************************************************
*Bildschirmausgabe
***********************************************************************************/
void Bildschirmausgabe(){
	unsigned char Zeile[4][20];
	
	
	switch(Ebene){
		case 1:{
			strcpy(Zeile[0], EBENE1_1);
			strcpy(Zeile[1], EBENE1_2);
			Zeile[2][0] = '\0';
			Zeile[3][0] = '\0';
			break;
		}
		case 2:{
			strcpy(Zeile[0], EBENE2_1);
			strcpy(Zeile[1], EBENE2_2);
			strcpy(Zeile[2], EBENE2_3);
			Zeile[3][0] = '\0';
			break;
		}
		case 3:{
			strcpy(Zeile[0], EBENE3_1);
			strcpy(Zeile[1], EBENE3_2);
			strcpy(Zeile[2], EBENE3_3);
			strcpy(Zeile[3], EBENE3_4);
			break;
		}
		case 4:{
			strcpy(Zeile[0], EBENE4_1);
			Zeile[1][0] = '\0';
			strcpy(Zeile[2], EBENE4_3);
			Zeile[3][0] = '\0';
			break;
		}
		case 5:{
			strcpy(Zeile[0], EBENE5_1);
			Zeile[1][0] = '\0';
			strcpy(Zeile[2], EBENE5_3);
			Zeile[3][0] = '\0';
			break;
		}
	}
	
	for(unsigned char Count = 0; Count < 4; Count ++){
		ReplaceText(Zeile[Count]);
	}
	
	LCD_CLR();
	//Text ausgeben
	LCD_RAM(0);
	LCD_STR(Zeile[0]);
	LCD_RAM(16);
	LCD_STR(Zeile[2]);
	LCD_RAM(64);
	LCD_STR(Zeile[1]);
	LCD_RAM(80);
	LCD_STR(Zeile[3]);
	
	//Cursor ausgeben
	char ram_adress = X;
	if(Y == 1)
	{
		ram_adress += 64;
	}
	else if(Y == 2)
	{
		ram_adress += 16;
	}
	else if(Y == 3)
	{
		ram_adress += 80;
	}
	LCD_RAM(ram_adress);
	
	if(M == 0)
	{
		CUR_OFF();
		CURB_OFF();
	}
	else if(M == 1)
	{
		CUR_ON();
		CURB_ON();
	}
	else if(M == 2)
	{
		CUR_OFF();
		CURB_OFF();
		LCD_CHR('>');
	}
}

void ReplaceText(unsigned char* Text)
{
	for(unsigned char Count = 0; Count < 16; Count++)
	{
		if(*(Text + Count) == 'h' && *(Text + Count + 1) == 'h')
		{
			*(Text + Count) = (char)(hh / 10) + 48;
			*(Text + Count + 1) = (char)(hh % 10) + 48;
		}
		else if(*(Text + Count) == 'm' && *(Text + Count + 1) == 'm')
		{
			*(Text + Count) = (char)(mm / 10) + 48;
			*(Text + Count + 1) = (char)(mm % 10) + 48;
		}
		else if(*(Text + Count) == 's' && *(Text + Count + 1) == 's')
		{
			*(Text + Count) = (char)(ss / 10) + 48;
			*(Text + Count + 1) = (char)(ss % 10) + 48;
		}
		else if(*(Text + Count) == 'd' && *(Text + Count + 1) == 'd')
		{
			*(Text + Count) = (char)(dd / 10) + 48;
			*(Text + Count + 1) = (char)(dd % 10) + 48;
		}
		else if(*(Text + Count) == 'M' && *(Text + Count + 1) == 'M')
		{
			*(Text + Count) = (char)(MM / 10) + 48;
			*(Text + Count + 1) = (char)(MM % 10) + 48;
		}
		else if(*(Text + Count) == 'Y' && *(Text + Count + 1) == 'Y' && *(Text + Count + 2) == 'Y' && *(Text + Count + 3) == 'Y')
		{
			*(Text + Count) = '2';
			*(Text + Count + 1) = '0';
			*(Text + Count + 2) = (char)(YYYY / 10) + 48;
			*(Text + Count + 3) = (char)(YYYY % 10) + 48;
		}
	}
}

/****************************************************************************
void f_InitializeOverflowInterrupt
****************************************************************************/
void f_InitializeOverflowInterrupt(){
	// Timer 0 konfigurieren
	TCCR0 = 5; //(1<<CS02); // Prescaler 1024	
 
	// Overflow Interrupt erlauben
	TIMSK |= (1<<TOIE0);
	
	// Global Interrupts aktivieren
	//sei();
}

/*
Der Overflow Interrupt Handler
wird aufgerufen, wenn TCNT0 von
255 auf 0 wechselt (256 Schritte),
d.h. ca. alle 2 ms
*/
#ifndef TIMER0_OVF_vect
// Für ältere WinAVR Versionen z.B. WinAVR-20071221 
#define TIMER0_OVF_vect TIMER0_OVF0_vect
#endif
 
ISR (TIMER0_OVF_vect)
{
    /* Interrupt Aktion alle
    (1000000/1024)/256 Hz = 3.814697 Hz
    bzw.
    1/3.814697 s = 262.144 ms  
    */
	if(Ebene == 1)
	{
		ms++;
		if(ms == 8)
		{
			ss++;
			ms = 0;
			if(ss == 60)
			{
				ss=0;
				mm++;
			}
			if(mm == 60)
			{
				mm = 0;
				hh++;
			}
			if(hh == 24)
			{
				hh = 0;
				dd++;
			}
			if(dd == 32)
			{
				dd = 1;
				MM++;
			}
			if(MM == 13)
			{
				MM = 1;
				YYYY++;
			}
			Bildschirmausgabe();
		}
	}
}

