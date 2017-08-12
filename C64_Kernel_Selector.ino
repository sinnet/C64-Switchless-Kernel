#include <EEPROM.h>
#include <Keypad.h>

// C64 key codes
#define KEY_RSHIFT      52
#define KEY_EQUAL       53
#define KEY_UP          54
#define KEY_HOME        55
#define KEY_F1          60
#define KEY_F3          61
#define KEY_F5          62
#define KEY_F7          63

// Shifted keys
#define KEY_F2          112   // KEY_RSHIFT + KEY_F1 = 112
#define KEY_F4          113   // KEY_RSHIFT + KEY_F3 = 113
#define KEY_F6          114   // KEY_RSHIFT + KEY_F5 = 114
#define KEY_F8          115   // KEY_RSHIFT + KEY_F7 = 115

// External 27C512 EPROM address pins A13, A14 & A15
#define EPROM_A13       A0  // PC0
#define EPROM_A14       A1  // PC1
#define EPROM_A15       A2  // PC2

// C64 reset pin
#define C64_RESET       3   // PD3

// Key input pins
#define KEY_RESTORE     2   // PD2
#define ROW_G           0   // PD0
#define ROW_H           1   // PD1
#define COL_4           10  // PB2
#define COL_5           9   // PB1
#define COL_6           8   // PB0
#define COL_7           7   // PD7

// Keyboard setup
const byte ROWS = 2;
const byte COLS = 4;

byte rowPins[ROWS] = {ROW_G, ROW_H};                  // Keyboard header pin 14, 13
byte colPins[COLS] = {COL_4, COL_5, COL_6, COL_7};    // Keyboard header pin 8, 7, 6, 5

char keys[ROWS][COLS] = {
  {KEY_RSHIFT, KEY_EQUAL, KEY_UP, KEY_HOME},
  {KEY_F1, KEY_F3, KEY_F5, KEY_F7}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Read default kernel from internal EEPROM at address 0
int address = 0;
int defaultBank = EEPROM.read(address);

void setup()
{
  pinMode(C64_RESET, OUTPUT);   // C64_RESET as output
  digitalWrite(C64_RESET, LOW); // Hold the C64 in reset state, until external EPROM address is set

  // External EEPROM address pins A13, A14 & A15
  pinMode(EPROM_A13, OUTPUT);
  pinMode(EPROM_A14, OUTPUT);
  pinMode(EPROM_A15, OUTPUT);
  
  // Select bank 0 as default if no value in EEPROM
  if(defaultBank < 0 || defaultBank > 7)
  {
    EEPROM.update(address, 0);
    defaultBank = 0;
  }

  delay(20);

  int bank = ReadC64Keys();
  
  SelectBank(bank);

  // Finally boot the C64
  digitalWrite(C64_RESET, HIGH); // Pull the C64 reset line HIGH, so it's able to exit reset state
  pinMode(C64_RESET, INPUT); //Set as Input so any external reset doesn't drive against the AVR pin

  // Interrupt for reset button on the C64
  //attachInterrupt(digitalPinToInterrupt(C64_RESET), Reset, FALLING);
}

// Read keys
int ReadC64Keys()
{
  char key = NO_KEY;
  
  // Get the key(s)
  keypad.getKeys();

  for (int i = 0; i < LIST_MAX; i++)
  {
      key += keypad.key[i].kchar;
  }

  switch (key)
  {
    case KEY_F1:
      return 0;
      break;

    case KEY_F2:
      return 1;
      break;

    case KEY_F3:
      return 2;
      break;
    
    case KEY_F4:
      return 3;
      break;

    case KEY_F5:
      return 4;
      break;

    case KEY_F6:
      return 5;
      break;

    case KEY_F7:
      return 6;
      break;

    case KEY_F8:
      return 7;
      break;

    // No key pressed, so use last bank selected
    default:
      return defaultBank;
      break;
  }
}

void SelectBank(int bank)
{
  switch(bank)
  {
    case 0:
      // Bank 0: $0000 - $1FFF
      // Outputs: A13=0, A14=0, A15=0
      digitalWrite(EPROM_A13, LOW);
      digitalWrite(EPROM_A14, LOW);
      digitalWrite(EPROM_A15, LOW);
      break;

    case 1:
      // Bank 1: $2000 - $3FFF
      // Outputs: A13=1, A14=0, A15=0
      digitalWrite(EPROM_A13, HIGH);
      digitalWrite(EPROM_A14, LOW);
      digitalWrite(EPROM_A15, LOW);
      break;

    case 2:
      // Bank 2: $4000 - $5FFF
      // Outputs: A13=0, A14=1, A15=0
      digitalWrite(EPROM_A13, LOW);
      digitalWrite(EPROM_A14, HIGH);
      digitalWrite(EPROM_A15, LOW);
      break;

    case 3:
      // Bank 3: $6000 - $7FFF
      // Outputs: A13=1, A14=1, A15=0
      digitalWrite(EPROM_A13, HIGH);
      digitalWrite(EPROM_A14, HIGH);
      digitalWrite(EPROM_A15, LOW);
      break;

    case 4:
      // Bank 4: $8000 - $9FFF
      // Outputs: A13=0, A14=0, A15=1
      digitalWrite(EPROM_A13, LOW);
      digitalWrite(EPROM_A14, LOW);
      digitalWrite(EPROM_A15, HIGH);
      break;

    case 5:
      // Bank 5: $A000 - $BFFF
      // Outputs: A13=1, A14=0, A15=1
      digitalWrite(EPROM_A13, HIGH);
      digitalWrite(EPROM_A14, LOW);
      digitalWrite(EPROM_A15, HIGH);
      break;

    case 6:
      // Bank 6: $C000 - $DFFF
      // Outputs: A13=0, A14=1, A15=1
      digitalWrite(EPROM_A13, LOW);
      digitalWrite(EPROM_A14, HIGH);
      digitalWrite(EPROM_A15, HIGH);
      break;

    case 7:
      // Bank 7: $E000 - $FFFF
      // Outputs: A13=1, A14=1, A15=1
      digitalWrite(EPROM_A13, HIGH);
      digitalWrite(EPROM_A14, HIGH);
      digitalWrite(EPROM_A15, HIGH);
      break;
  }

  EEPROM.update(address, bank); // Write the selected bank into internal AVR EEPROM
  defaultBank = bank;           // Set defaultBank to the selected bank
}

void Reset()
{
  asm volatile ("  jmp 0");
}

void ResetC64()
{
  pinMode(C64_RESET, OUTPUT);
  digitalWrite(C64_RESET, LOW);
  delay(2);
  digitalWrite(C64_RESET, HIGH);
  pinMode(C64_RESET, INPUT); //Set as Input so any external reset doesn't drive against the AVR pin
}

void loop()
{

}
