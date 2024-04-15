#include <avr/io.h>
#include <util/delay.h>


#define F_SCL 	100000L  	// 100 кГц - тактовая частота на линии SCL 

#define PCF8574_adr 0x27

// Коды состояний шины - значения битов TWS7…TWS3 регистра состояния TWSR
#define START       0x08    // СТАРТ
#define REP_START   0x10	// ПОВТОРНЫЙ СТАРТ
#define SLA_W_ACK	0x18	// ведомое устройство с указанным адресом найдено (есть сигнал ACK)
#define SLA_W_NACK	0x20	// ведомое устройство с указанным адресом не найдено (нет сигнала ACK)
#define DATA_W_ACK	0x28	// ведомое устройство успешно приняло байт данных (есть сигнал ACK) и готово к приему нового байта
#define DATA_W_NACK	0x30	// ведомое устройство не готово к приему нового байта (нет сигнала ACK)
 
#define STATUS (TWSR & 0xF8) // макрос для считывания кода состояния из регистра TWSR

#define CMD 0 // команда
#define DTA 1 // данные

//__________________________________________________________
// LCD HD44780
#define LCD_RS  0
#define LCD_RW  1
#define LCD_E   2
#define LCD_BL  3
#define LCD_D5  5


#define LCD_CLEAR       0x01   // очистить дисплей и установить курсор в нулевую позицию
#define LCD_OFF         0x08   // дисплей выключен, курсор не виден, курсор не мигает
#define LCD_ON          0x0C   // дисплей включен, курсор не виден, курсор не мигает
#define LCD_RETURN      0x02   // возврат курсора в нулевую позицию

 
//************МАССИВ КОМАНД ИНИЦИАЛИЗАЦИИ ЖКИ*************//
	
char lcd_init_commands[] = {0x20,0x28,0x28,LCD_OFF,LCD_CLEAR,0x06,0x0c};

void init_i2c(){
	TWSR = 0;
	TWBR = ((F_CPU/F_SCL)-16)/2;
}

char start_i2c(){
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	return (STATUS == START || STATUS == REP_START);
}

char sendAddr_i2c(unsigned char addr, unsigned char direction){
	TWDR = (addr<<1) | direction;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	return (STATUS == SLA_W_ACK);
}

char sendByte_i2c(unsigned char data){ 	
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	return (STATUS == DATA_W_ACK);	
}

void stop_i2c(){
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}

void send_lcd(char value, char mode){
	char LCD;
	LCD = (value & 0xF0) | (mode<<LCD_RS) | (1<<LCD_E) | (1<<LCD_BL);
	sendByte_i2c(LCD);

	LCD&=~(1<<LCD_E);
	sendByte_i2c(LCD);

	LCD = (value << 4) | (mode<<LCD_RS) | (1<<LCD_E) | (1<<LCD_BL);
	sendByte_i2c(LCD);

	LCD&=~(1<<LCD_E);
	sendByte_i2c(LCD);
}

void init_lcd(){
	for(int i = 0; i < 7; i++){
		send_lcd(lcd_init_commands[i],CMD);
		_delay_ms(40);
	}
}

void setCursor(int x, int y){
	char position[2][16] = {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF};
	send_lcd(position[x][y],CMD);
}

void clear(void){
	setCursor(0,0);
	for (int i = 0; i < 16; i++)
		send_lcd(' ', DTA);
	setCursor(1,0);
	for (int i = 0; i < 16; i++)
		send_lcd(' ', DTA);
	setCursor(0,0);
}


void type(int x, int y, char letter){
	if(y > 15 && x == 0)
		setCursor(1,y - 16);
	else if(y > 15 && x == 1)
		setCursor(0,y - 16);
	else
		setCursor(x,y);
	send_lcd(letter,DTA);
}


void first(void){
	while(1){
		for(int x = 0;x < 2;x++){
			for(int y = 0; y < 16; y++){
				type(x,y,0x61);
				type(x,y+1,0x62);
				type(x,y+2,0x63);
				type(x,y+3,0x64);
				_delay_ms(200);
				clear();
			}
		}
	}
}

void second(void){
	send_lcd('L', DTA);
	send_lcd('I', DTA);
	send_lcd('Z', DTA);
	send_lcd('A', DTA);
	setCursor(1,0);
	send_lcd('L', DTA);
	send_lcd('I', DTA);
	send_lcd('T', DTA);
	send_lcd('O', DTA);
	send_lcd('V', DTA);
	send_lcd('K', DTA);
	send_lcd('O', DTA);
	_delay_ms(10000);
	clear();
}

void main(void){
	init_i2c();

	start_i2c();
	sendAddr_i2c(PCF8574_adr,0x00);

	init_lcd();
	fourth();   // в зависимости от задания

	stop_i2c();
}