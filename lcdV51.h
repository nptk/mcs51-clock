#define lcd_clear() lcd_command(1)
#define lcd_origin() lcd_command(2)
#define NUMBER_OF_DIGITS	16
sbit e = P3^7;
sbit rs = P3^6;

// delay
void delay(int sec){
	int i,j;
	for(i=0;i<sec;i++)
		for(j=0;j<419;j++);
}

void lcd_command(unsigned char com){
	rs	= 0;	
	e	= 1;
	P2	= com;
	delay(2);
	e	= 0;
	delay(2);
}

void lcd_text(unsigned char text){
	rs	= 1;	
	e	= 1;
	P2	= text;
	delay(2);
	e	= 0;
	delay(2);
}

//string to LCD

void lcd_puts( char addr, char *ptr ) {
	lcd_origin();
	lcd_command(addr);
	while (*ptr) {
		lcd_text(*ptr);
		ptr++;
	}
}

void _ultoa( unsigned long value, char *string, unsigned char radix){
	unsigned char index;
	char buffer[NUMBER_OF_DIGITS];
	index = NUMBER_OF_DIGITS;
	do{
		buffer[--index] = '0' + (value % radix);
		if ( buffer[index] > '9'){
			buffer[index] += 'A' - '9' - 1;			
		}
		value /= radix;
	}while (value != 0);
	
	do{
		*string++ = buffer[index++];
	} while (index < NUMBER_OF_DIGITS);
	*string = 0;
}

void _ltoa(long value_l, char *string_l, unsigned char radix_l){
	if (value_l < 0 && radix_l == 10){
		*string_l++ = '-';
		_ultoa (-value_l, string_l, radix_l);
	}else{
		_ultoa (value_l, string_l, radix_l);
	}
}

void intToLCD(unsigned char posi, int value){
	char buff[12];
	_ltoa(value, &buff[0], 10);
	lcd_puts(posi, &buff[0]);
}

void lcd_init(){
	lcd_command(0x38);
	lcd_command(0x0c);
	lcd_command(0x01);
}