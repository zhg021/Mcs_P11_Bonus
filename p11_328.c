#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>

////// p11 bonus funcional a 5v
#define F_CPU 16000000

#define Black 30
#define Red 31
#define Green 32
#define Yellow 33
#define Blue 34
#define Magenta 35
#define Cyan 36
#define White 37

// UART
/*
// en caso de usar 2560/1280
void UART_Init(uint8_t com, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop) {
  uint16_t *UBRR_ptr[] = {&UBRR0, &UBRR1, &UBRR2, &UBRR3};
  uint8_t *UCSRA_ptr[] = {&UCSR0A, &UCSR1A, &UCSR2A, &UCSR3A};
  uint8_t *UCSRB_ptr[] = {&UCSR0B, &UCSR1B, &UCSR2B, &UCSR3B};
  uint8_t *UCSRC_ptr[] = {&UCSR0C, &UCSR1C, &UCSR2C, &UCSR3C};

  if (com > 3) {
    com = 0;
  }
  *UBRR_ptr[com] = F_CPU / (16 * baudrate) - 1;

  *UCSRB_ptr[com] = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
  *UCSRC_ptr[com] |= ((size - 5) << UCSZ00) | ((stop - 1) << USBS0);
  *UCSRC_ptr[com] |= (1 << UPM01) | (parity << UPM00);
}*/

//en caso de usar 328, solo hay com 0
void UART_Init(uint8_t com, uint32_t baudrate, uint8_t size, uint8_t parity, uint8_t stop){
  /*uint16_t UBRR0;
  uint8_t UCSR0A;
  uint8_t UCSR0B;
  uint8_t UCSR0C;*/

  if (com > 0) {
    com = 0;
  }
  UBRR0 = F_CPU / (16 * baudrate) - 1;

  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
  UCSR0C |= ((size - 5) << UCSZ00) | ((stop - 1) << USBS0);
  UCSR0C |= (1 << UPM01) | (parity << UPM00);
}

void UART0_puts(char *str) {
  while (*str) {
    while (!(UCSR0A & (1 << UDRE0))) {
      ;
    }
    UDR0 = *str++;
  }
}

void UART0_putchar(char dato) {
  while (!(UCSR0A & (1 << UDRE0))) {
    ;
  }
  UDR0 = dato;
}

void gotoxy(uint8_t x, uint8_t y) {
  char xStr[3], yStr[3];
  xStr[0] = '0' + x / 10;
  xStr[1] = '0' + x % 10;
  xStr[2] = '\0';

  yStr[0] = '0' + y / 10;
  yStr[1] = '0' + y % 10;
  yStr[2] = '\0';

  UART0_putchar('\033');
  UART0_putchar('[');
  UART0_puts(yStr);
  UART0_putchar(';');
  UART0_puts(xStr);
  UART0_putchar('H');
}

void clear() {
  UART0_puts("\033[K");  // clear line from cursor right
}

void itoa(char* str, uint16_t number, uint8_t base) {
  int digits = 0;
  uint16_t tempNumber = number;
  while (tempNumber >= base) {
    digits++;
    tempNumber /= base;
  }
  str[digits + 1] = '\0';
  while (digits != 0) {
    if ((number % base) > 9){
      str[digits] = number % base + 'A' - 10;
    }else{
      str[digits] = number % base + '0';
    }
    number /= base;
    digits--;
  }
  if ((number % base) > 9) {
    str[digits] = number % base + 'A' - 10;
  } else {
    str[digits] = number % base + '0';
  }
}

uint16_t atoi(char *str) {
  uint16_t valor = 0;
  while (*str >= '0' && *str <= '9') {
    valor *= 10;
    valor += *str - '0';
    str++;
  }
  return valor;
}

void setColor(uint8_t color) {
  UART0_putchar('\033');
  UART0_putchar('[');
  UART0_putchar('0');
  UART0_putchar(';');
  char colorStr[3];
  itoa(colorStr, color, 10);
  UART0_puts(colorStr);
  UART0_putchar('m');
}

/////////////ADC1
void ADC_Ini() {
  // Habilitar el ADC
  ADCSRA |= (1 << ADEN);
  // Ajustar la referencia de voltaje a AVCC
  ADMUX = (1 << REFS0);
  // Configurar el prescaler a 8 
  ADCSRA |= (1 << ADPS1) | (1 << ADPS0);
  // Seleccionar el canal 0 como entrada
  ADMUX &= ~(1 << MUX0);
}

uint16_t ADC_Read() {
  // Inicia una conversion ADC iniciando el bit ADSC en el registro ADCSRA
  ADCSRA |= (1 << ADSC);  
   // Espera a que se complete la conversion ADC verificando el bit ADIF en el registro ADCSRA
  while (!(ADCSRA & (1 << ADIF))) {
    ;
  }
   // Devuelve el valor convertido almacenado en el registro ADC
  return ADC;
}

void print_ADC_Value() { 

  uint16_t value; // variable para almacenar el valor del ADC
  char cad[5]; // cadena de caracteres para el valor del ADC convertido a voltaje
  static uint16_t i = 0; // contador estatico para el indice de impresion
  static uint8_t j = 0; // contador estatico para el indice de impresion
  char IC[5]; // cadena de caracteres para el contador i convertido a cadena
  uint16_t ajuste; // variable para el ajuste del voltaje

  itoa(IC, i, 10); // convertir el contador i a cadena de caracteres

  value = ADC_Read(); // leer el valor del ADC
  uint16_t voltage = value * 50 / 1023; // calcular el voltaje

  cad[0] = '0' + voltage / 10; // obtener el primer digito del voltaje
  cad[1] = '.'; // punto decimal
  cad[2] = '0' + voltage % 10; // obtener el segundo digito del voltaje
  cad[3] = '\0'; // terminador de cadena

  ajuste = 50 - voltage; // calcular el ajuste

  clear(); // limpiar antes de imprimir nuevo
  //gotoxy(0,0);

  if (i < 10) { // condicion para imprimir el contador i
    UART0_puts("000"); // imprimir ceros
  } else if (i < 100) {
    UART0_puts("00");
  } else if (i < 1000) {
    UART0_puts("0");
  }else{

  }

  UART0_puts(IC); // imprimir el contador i
  UART0_putchar('|'); // imprimir el caracter '|'

  // Imprimir espacios antes del *
  for (int k = 0; k < voltage; k++) { // bucle para imprimir espacios
    UART0_putchar(' '); // imprimir un espacio
  }

  if (voltage > 25) { // condicion para el color verde
    setColor(Green); // establecer el color verde
  }else{ // si no
    setColor(Red); // establecer el color rojo
  }

  UART0_putchar('*'); // imprimir el caracter '*'

  for (int k = 0; k < 50 - voltage; k++) { // bucle para imprimir espacios restantes
    UART0_putchar(' '); // imprimir un espacio
  }

   
  // Imprimir la barra despues del * y espacios restantes
  setColor(White); // establecer el color blanco
  UART0_putchar('|'); // imprimir el caracter '|'

  if (voltage > 25) { // condicion para el color verde
    setColor(Green); // establecer el color verde
  } else { // si no
    setColor(Red); // establecer el color rojo
  }
  // Imprimir el valor despues del |
  UART0_puts(cad); // imprimir la cadena de caracteres del voltaje
  setColor(White); // establecer el color blanco

  i++; // incrementar el contador i
  if (i > 1000) { // condicion para reiniciar el contador i
    i = 0; // reiniciar el contador i
    UART0_puts("\r\n");
  }
}

int main(void) {
  UART_Init(0, 9600, 8, 0, 1);
  DDRC = 0x0;
  ADC_Ini();

  while (1) {
    print_ADC_Value();
    UART0_puts("\r\n");
  }
}
