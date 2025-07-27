lude <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address

// Product structure
struct Product {
  byte uid[4];
  const char* name;
  int price;
  int weight;
  bool added;  // Track if added or removed
};

// Your products
Product products[] = {
  {{0xC5, 0xCA, 0x56, 0x01}, "Rice", 50, 1000, false},
  {{0x63, 0x0E, 0x05, 0x29}, "Sugar", 40, 500, false},
  {{0xF3, 0xD5, 0xB9, 0xF7}, "Oil", 90, 1000, false}
};

int totalPrice = 0;
int totalWeight = 0;

// Compare UIDs
bool compareUID(byte *uid1, byte *uid2) {
  for (byte i = 0; i < 4; i++) {
    if (uid1[i] != uid2[i]) return false;
  }
  return true;
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Smart Trolley");
  delay(2000);
  lcd.clear();

  Serial.println("Smart Trolley Ready");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  byte* scannedUID = rfid.uid.uidByte;

  Serial.print("Scanned UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print("0x");
    Serial.print(scannedUID[i], HEX);
    if (i < rfid.uid.size - 1) Serial.print(", ");
  }
  Serial.println();

  bool found = false;

  for (int i = 0; i < sizeof(products)/sizeof(Product); i++) {
    if (compareUID(scannedUID, products[i].uid)) {
      found = true;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(products[i].name);

      if (!products[i].added) {
        // Add item
        totalPrice += products[i].price;
        totalWeight += products[i].weight;
        products[i].added = true;

        lcd.setCursor(0, 1);
        lcd.print("Added Rs.");
        lcd.print(products[i].price);

        Serial.print("Added: ");
        Serial.println(products[i].name);
      } else {
        // Remove item
        totalPrice -= products[i].price;
        totalWeight -= products[i].weight;
        products[i].added = false;

        lcd.setCursor(0, 1);
        lcd.print("Removed Rs.");
        lcd.print(products[i].price);

        Serial.print("Removed: ");
        Serial.println(products[i].name);
      }

      // Print totals
      Serial.print("Total Price: Rs.");
      Serial.println(totalPrice);
      Serial.print("Total Weight: ");
      Serial.print(totalWeight);
      Serial.println("g");

      break;
    }
  }

  if (!found) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Item not found");
    Serial.println("Unknown RFID tag");
  }

  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Rs:");
  lcd.print(totalPrice);
  lcd.setCursor(9, 0);
  lcd.print("Wt:");
  lcd.print(totalWeight / 1000.0); // Show in kg

  rfid.PICC_HaltA();
}