#include <Wire.h>
#include <Adafruit_PN532.h>
#include <EEPROM.h>

#define relayPin 3
#define SDA_PIN A4 // I2C SDA pini
#define SCL_PIN A5 // I2C SCL pini

Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);  // PN532 modülü örneği

const int maxCards = 50;            // Yetkili kartlar için maksimum dizi boyutu
const int UIDLength = 12;           // UID uzunluğu (kart UID'si 12 karakter olarak saklanacak)
int cardCount = 0;                  // Yetkili kart sayısı
String masterUID = "D3EC2B28";      // Anahtar kartın UID'si
String eepromResetUID = "B2C82C1B"; // EEPROM'u sıfırlayacak kart UID'si
bool addMode = false;               // Kart ekleme modu

unsigned long lastReadTime = 0; // Son kart okuma zamanı
const unsigned long debounceDelay = 3000; // 3 saniye

// Önceden tanımlı kart UID'leri (statik olarak EEPROM'a yazılacak)
String predefinedUIDs[] = {
  "4A9F4D99",
  "3AE0D799",
  "5A920999",
  "638B272D",
  "E3825718",
  "43355D2D",
  "5A688A99",
  "5A4F2199",
  "335C3F2D",
  "F35A172D",
  "3AE2B199",
  "73368713",
  "4A1D1F99",
  "DA8E7A99",
  "139D452D",
  "4A230B99",
  "DA1AF899",
  "3AF29E99",
  "5A368E99",
  "4AD6B399",
  "DA113999",
  "DA82B199",
  "DAEA6D99",
  "D3E3252D",
  "4A27F599",
  "DA092899",
  "83D8332D",
  "5A1C0B99",
  "5A90E899",
  "DA995199",
  "F3CC2A2D"
};
const int numPredefinedUIDs = sizeof(predefinedUIDs) / sizeof(predefinedUIDs[0]);

// Fonksiyon prototipleri
void addUID(String newUID);
bool checkUID(String UID);
void printAuthorizedUIDs();
void loadUIDsFromEEPROM();
void resetEEPROM();
void saveUIDToEEPROM(int index, String UID);
String readUIDFromEEPROM(int index);
void addPredefinedUIDs();

void setup(void) {
  Serial.begin(9600);
  Wire.begin();  // I2C başlat
  nfc.begin();

  // nfc modülünü başlat
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("PN53x modülü bulunamadı!");
    while (1); // Dur ve hata ver
  }
  
  // PN532 modülünü başlat
  nfc.SAMConfig();

  Serial.println("Kart okuyucuya hazır. Kartınızı yaklaştırın...");

  // Röleyi başlat
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  // EEPROM'dan kartları yükle
  cardCount = EEPROM.read(0);  // EEPROM'daki ilk byte kart sayısını tutar
  Serial.print("EEPROM'dan kart sayısı yüklendi: ");
  Serial.println(cardCount);

  // Önceden tanımlı kartları EEPROM'a yaz
  addPredefinedUIDs();
}

void loop(void) {
  // Eğer belirli bir süre (3 saniye) geçmemişse kart okuma işlemi yapma
  if (millis() - lastReadTime < debounceDelay) {
    return;
  }

  // Kart UID'sini oku
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // UID dizesi
  uint8_t uidLength;

  // Yeni kart aranıyor
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    lastReadTime = millis(); // Kart okunduğunda zaman güncellemesi
    String content = "";

    for (uint8_t i = 0; i < uidLength; i++) {
      content += String(uid[i], HEX); // UID'yi HEX olarak oku
    }
    content.toUpperCase();
    Serial.print("Okunan Kart UID: ");
    Serial.println(content);

    // Anahtar kartı okuduğumuzu kontrol et
    if (content == masterUID) {
      Serial.println("Anahtar kart okundu. Kart ekleme moduna geçildi.");
      addMode = true;  // Kart ekleme moduna geç
      digitalWrite(relayPin, LOW);
      delay(1000);
      digitalWrite(relayPin, HIGH);
      delay(1000);
      digitalWrite(relayPin, LOW);
      delay(1000);
      digitalWrite(relayPin, HIGH);
      return;
    } else if (content == eepromResetUID) {
      resetEEPROM(); // EEPROM'u sıfırlama işlemi
      return;
    }

    // Kart ekleme modundaysak ve anahtar karttan farklı bir kart okunduysa, yeni kart ekle
    if (addMode) {
      Serial.println("Yeni kart eklendi: " + content);
      addUID(content);  // Yeni kartı ekle
      addMode = false;  // Kart ekleme modundan çık
      delay(2000);
      return;
    }

    // Yetkili kartlar listesinde olup olmadığını kontrol et
    if (checkUID(content)) {
      Serial.println("Yetkili erişim.");
      digitalWrite(relayPin, LOW);
      delay(3000);
      digitalWrite(relayPin, HIGH);
    } else {
      Serial.println("Erişim reddedildi.");
    }
    delay(2000);
  }
}

void resetEEPROM() {
  EEPROM.write(0, 0);  // EEPROM'daki ilk bayta kart sayısı olarak 0 yaz
  Serial.println("EEPROM sıfırlandı. Kart sayısı 0 yapıldı.");
  // Yazılım reseti
  asm volatile ("  jmp 0"); // Programın başlangıcına atla
}

// Kart UID'sini yetkili kartlar listesine ekler
void addUID(String newUID) {
  if (cardCount < maxCards) {
    if (!checkUID(newUID)) {
      saveUIDToEEPROM(cardCount++, newUID);  // Yeni UID'yi EEPROM'a ekle
      EEPROM.write(0, cardCount);  // Yeni kart sayısını EEPROM'a kaydet
      Serial.println("Kart başarıyla eklendi.");
    } else {
      Serial.println("Bu kart zaten eklenmiş.");
    }
  } else {
    Serial.println("Kart ekleme limiti aşıldı.");
  }
}

// Kartın yetkili olup olmadığını kontrol eder
bool checkUID(String UID) {
  for (int i = 0; i < cardCount; i++) {
    if (readUIDFromEEPROM(i) == UID) {
      return true;
    }
  }
  return false;
}

// EEPROM'a kart UID'si kaydet
void saveUIDToEEPROM(int index, String UID) {
  int start = 1 + (index * UIDLength);  // 1. bayttan başla, 0. bayt kart sayısını tutuyor
  for (int i = 0; i < UIDLength; i++) {
    EEPROM.write(start + i, UID[i]);
  }
}

// EEPROM'dan kart UID'sini oku
String readUIDFromEEPROM(int index) {
  int start = 1 + (index * UIDLength);  // 1. bayttan başla
  char UID[UIDLength + 1];
  for (int i = 0; i < UIDLength; i++) {
    UID[i] = char(EEPROM.read(start + i));
  }
  UID[UIDLength] = '\0';  // Null terminator ekle
  return String(UID);
}

// Önceden tanımlı kart UID'lerini EEPROM'a kaydeden fonksiyon
void addPredefinedUIDs() {
  for (int i = 0; i < numPredefinedUIDs; i++) {
    if (!checkUID(predefinedUIDs[i])) {
      addUID(predefinedUIDs[i]);  // Eğer EEPROM'da yoksa ekle
      Serial.println("Önceden tanımlı kart eklendi: " + predefinedUIDs[i]);
    }
  }
}
