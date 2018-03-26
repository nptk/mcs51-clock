#include <reg52.h>
#include <lcdV51.h>

//keypad

sbit c0 = P1^4;
sbit c1 = P1^5;
sbit c2 = P1^6;
sbit r0 = P1^0;
sbit r1 = P1^1;
sbit r2 = P1^2;
sbit r3 = P1^3;

//global clock Var
int clockTick=0;		// TR counter
int isClockEnable=0;	// TR keep run
int secondClock=0, minuteClock=0, hourClock=0;

//variable
int hour, minute, second, ledCounter, mode, alertCounter, isAlert;

// time counter var
int x=0, y=0;

//unchange variable
unsigned topPos = 0x80;
unsigned BottomPos = 0xC0;

// 8 LED -> P0
unsigned light[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
unsigned lightAlert[] = {0x00, 0xff};

//LCD -> P2 Data - P3 Control

int scankey(){
	unsigned char i,c;
	int key = 0xff;
	for (i=0, c=0xef; i<3; i++, c=(c<<1)^1 ){
		P1 = c;
		if ( r0 == 0 ) {while( r0 == 0 ); key = i+1;}
		if ( r1 == 0 ) {while( r1 == 0 ); key = i+4;}
		if ( r2 == 0 ) {while( r2 == 0 ); key = i+7;}
		if ( r3 == 0 ) {while( r3 == 0 ); key = i+10;}
	}
	return key;
}

void timeToLCD(){
	//hour
	if (hour >= 0) {
		intToLCD(BottomPos, hour/10);
		intToLCD(BottomPos+1, hour%10);
	}
	
	lcd_puts(BottomPos+2, ":");
	
	//min
	if (minute >= 0) {
		intToLCD(BottomPos+3, minute/10);
		intToLCD(BottomPos+4, minute%10);
	}
	
	lcd_puts(BottomPos+5, ":");
	
	//sec
	if (second >= 0) {
		intToLCD(BottomPos+6, second/10);
		intToLCD(BottomPos+7, second%10);
	}
	
	lcd_puts(BottomPos+8, "  ");
}

void clockToLCD(){
	//hour
	if (hourClock >= 0) {
		intToLCD(BottomPos, hourClock/10);
		intToLCD(BottomPos+1, hourClock%10);
	}
	
	lcd_puts(BottomPos+2, ":");
	
	//min
	if (minuteClock >= 0) {
		intToLCD(BottomPos+3, minuteClock/10);
		intToLCD(BottomPos+4, minuteClock%10);
	}
	
	lcd_puts(BottomPos+5, ":");
	
	//sec
	if (secondClock >= 0) {
		intToLCD(BottomPos+6, secondClock/10);
		intToLCD(BottomPos+7, secondClock%10);
	}
	
	lcd_puts(BottomPos+8, "  ");
}

void clampTime(){
	
	//mode 1
	if (secondClock>59) { secondClock=0; minuteClock++;}
	if (minuteClock>59) { minuteClock=0; hourClock++;}
	if (hourClock>23) { hourClock=0;minuteClock=0;secondClock=0;}
	
	//-- mode 2
	if (mode==2){
		if (second<0) { second=59; minute--;}
		if (minute<0) { minute=59; hour--;}
		if (hour<0) { hour=0;}
		if (hour==0 && minute==0 && second==0) { 
			isAlert = 1;
		} // Stop Timer once timer end
	}
	
	//++ mode 3
	if (mode==3) {
		if (second>59) { second=0; minute++;}
		if (minute>59) { minute=0; hour++;}
		if (hour>99) { hour=0;minute=0;second=0;}
	}
}

void myTimer0(void)interrupt 1
{
	if (isClockEnable == 1){
		clockTick++;
		if (clockTick == 4000){
			secondClock++;
			clampTime();
			clockTick = 0;
			// mode 1 on screen -> move LED
			if (mode == 1){
				ledCounter++;
			}
		}
	}
	
	if (mode == 3){
		x++;
		if(x==4000){	
			second++;
			clampTime();
			ledCounter++;
			x=0;
		}
		
	}
	
	if (mode == 2 && isAlert == 0){
		x++;
		if(x==4000){
			second--;
			clampTime();
			ledCounter--;
			x=0;
		}
		
	}
	
	//if any mode on screen, move LED
	if (isAlert == 0) {
		if (ledCounter>7) {ledCounter = 0;}
		if (ledCounter<0) {ledCounter = 7;}
		P0 = light[ledCounter];
	}else{ // alert LED once timer end(mode 2 only)
		y++;
		if( y==500 ){		
			if (alertCounter == 0){
				alertCounter = 1;
			}else{
				alertCounter = 0;
			}
			P0 = lightAlert[alertCounter]; 
			y = 0;
		}
	}
}

//add to first elment of the array
void addToLast(int inputA[], int value){
	if (inputA[0] < 10) {
		int i;
		for (i=5; i>0; i--){
			inputA[i] = inputA[i-1];
		}
	}
	inputA[0] = value;
}

void setTime(){
	int inputTimeKey, inputA[6] = {10,10,10,10,10,10}; // s m h
	int inputTime=0;
	lcd_puts(topPos, "Enter Time");
	timeToLCD();
	
	while (1) {
		do{
			inputTimeKey = scankey();
		}while (inputTimeKey == 0xff);
		
		if (inputTimeKey == 12){ // # stop setTime
			if (mode == 1) {
				isClockEnable = 1;
				break; //setTime Finish 
			}
		
			if (mode == 2 && hour>0 || minute>0 || second>0){
				break; //setTime Finish 
			}else{
				lcd_puts(topPos, "invalid input");
			}
		}
		
		if (inputTimeKey == 10){ // * reset setTime
			int i;
			for (i=0; i<6; i++){
				inputA[i] = 10;
			}
			
			hour = 0;
			minute = 0;
			second = 0;
			timeToLCD();
			continue;
			//****
		}
		
		if (inputTimeKey <= 9 || inputTimeKey == 11) { // only 1-9 and 0 (11)
			if (inputTimeKey == 11) {inputTimeKey = 0;}
			
			inputTime = inputTimeKey;
			
			addToLast( inputA, inputTime ); //****
		}
		
		hour 	= ((inputA[5]%10) * 10) + (inputA[4]%10);
		minute 	= ((inputA[3]%10) * 10) + (inputA[2]%10);
		second 	= ((inputA[1]%10) * 10) + (inputA[0]%10);
		
		//clamp setTime
		if (second>59) { second=59; }
		if (minute>59) { minute=59; }
		if (mode == 1 && hour>23) { hour=23; }
		
		timeToLCD();
		
		// end
	}
	if (mode == 1){
		secondClock=second;
		minuteClock=minute;
		hourClock=hour;	
		
		hour = 0;
		minute = 0;
		second = 0;
	}
	
	lcd_command(0x01);	  // clear LCD
}

void chooseMode(){
	int modeDisplay = 1;
	int nav;
	lcd_puts(topPos, "Select Mode");
	lcd_puts(BottomPos, "(1/3)Clock");
	
	do{
		nav = scankey();
		
		if (nav == 1 || nav == 3){
			
			//++ --
			if (nav == 1) {modeDisplay--;}
			if (nav == 3) {modeDisplay++;}
			
			//clamp
			if (modeDisplay < 1) {modeDisplay = 3;}
			if (modeDisplay > 3) {modeDisplay = 1;}
			
			//display
			if (modeDisplay == 1) {
				lcd_puts(BottomPos, "(1/3)Clock      ");
			}
			if (modeDisplay == 2) {
				lcd_puts(BottomPos, "(2/3)Timer      ");
			}
			if (modeDisplay == 3) {
				lcd_puts(BottomPos, "(3/3)StopWatch  ");
			}
		}
		
	}while ( nav != 12 ); // # to stop mode selecting
	
	mode = modeDisplay;
	lcd_command(0x01);	  // clear LCD
}

void stopWatch(){
	int stopWatchKey;
	int isEnable = 0;
	
	while(1){
		timeToLCD();
		stopWatchKey = scankey();
		
		//toggleToEnable #
		if (stopWatchKey == 12){
			
			if (isEnable == 0){
				lcd_puts(topPos, "StopWatch | On  ");
				TR0=1;
				isEnable = 1;
			}else{
				lcd_puts(topPos, "StopWatch | Off ");
				TR0=0;
				isEnable = 0;
			}
			
		};
		
		//backToMenu *
		if (stopWatchKey == 10){
			lcd_puts(topPos, "                ");
			break;
		};
		
		//stop and reset - num0
		if (stopWatchKey == 11){
			lcd_puts(topPos, "StopWatch | Off ");
			
			//reset inturrpt counter
			x=0;
			y=0;
			
			TR0=0;
			
			hour = 0;
			minute = 0;
			second = 0;
		};
	}
}

void main(void){
	myReset:
	//init+reset global variable
	hour=0;
	minute=0;
	second=0;
	ledCounter=0;
	mode=0;
	alertCounter=0;
	isAlert=0;
	TR0=0;
	
	P0 = 0x00;
	
	//TR counter
	x=0; 
	y=0; 
	
	TMOD=0x02;	// mode 2, auto
	EA=1;

	//tr for 1sec
	TH0=0X06; // start from 06->256 = 250 microSec
	ET0=1;
	
	lcd_init();
	lcd_command(0x0C); // text only

	chooseMode(); // with lcd clear
	
	if (mode == 1){
		if(isClockEnable == 0){
			setTime();
		}
		
		lcd_puts(topPos, "Clock");
		TR0=1; // start timer
		
		//scope
		{
			int tmpBackToMenuKey=0;
			
			while (1) {// show clock+scan
				clockToLCD();
				tmpBackToMenuKey = scankey();
				// * scan for back to menu
				if (tmpBackToMenuKey == 10){
					goto myReset;
				}
				
				//# resetClock and back to Menu
				if (tmpBackToMenuKey == 12){
					
					clockTick=0;
					isClockEnable=0;
					secondClock=0;
					minuteClock=0;
					hourClock=0;
					lcd_puts(topPos, "Clock Reset!    ");
					lcd_puts(BottomPos, "Press Any Key   ");
					
					{
						int tmpKey=0;
						do{
							tmpKey = scankey();
						}while (tmpKey == 0xff);
					}
					
					goto myReset;
				}
			}
			
		}
	}
	
	if (mode == 2){
		setTime();
		lcd_puts(topPos, "Timer");
		TR0=1; // start timer
		
		//scope
		{
			int resetKey=0;
			while (1) {// show clock+scan
				timeToLCD();
				{
					resetKey = scankey();
					// * scan for back to menu
					if (resetKey == 10){
						goto myReset;
					}
				}
				
				if (isAlert == 1) {
					lcd_puts(topPos, "Time UP !       ");
					lcd_puts(BottomPos, "Press * to Menu");
					{
						int resetAlertKey=0;
						do{
							resetAlertKey = scankey();
						}while (resetAlertKey != 10);
					}
					goto myReset;
				}
			}
		}
	}
	
	if (mode == 3) {
		lcd_puts(topPos, "StopWatch");
		stopWatch();
		goto myReset;
	}
}
