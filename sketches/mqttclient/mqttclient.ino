#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Verbindungseinstellungen
const char* WIFI_SSID = "kleinanzeigen";
const char* WIFI_PASS = "section77";
const char* MQTT_SERVER = "192.168.0.1";
WiFiClient espClient;
PubSubClient client(espClient);

// Definition der Anschlüsse
// FP2800A unten: Spalte/Element
const int FP_SPALTE_A0 = 1; // TX, GPIO1
const int FP_SPALTE_A1 = D1;
const int FP_SPALTE_A2 = D2;
const int FP_SPALTE_B0 = D3;
const int FP_SPALTE_B1 = D4; // BUILTIN_LED
// FP2800A oben: Zeile/Segment
const int FP_ZEILE_A0 = D8;
const int FP_ZEILE_A1 = D7;
const int FP_ZEILE_A2 = D6;
const int FP_ZEILE_B0 = D5;
const int FP_ENABLE = D0;

// Zeile/Segment wird nur mit 3 Bit addressiert, Bit 4 steuert SET/UNSET und 5 wird nicht verwendet
const int ZEILE_ADDR[3] =  { FP_ZEILE_A0, FP_ZEILE_A1, FP_ZEILE_A2 };
const int SPALTE_ADDR[5] = { FP_SPALTE_A0, FP_SPALTE_A1, FP_SPALTE_A2, FP_SPALTE_B0, FP_SPALTE_B1};

// Zahlen
const boolean ZAHLEN[10][7] = { { true,  true,  true,  true,  true,  true, false},   // 0
                                {false, false, false, false,  true,  true, false},   // 1
                                { true, false,  true,  true, false,  true,  true},   // 2
                                { true, false, false,  true,  true,  true,  true},   // 3
                                {false,  true, false, false,  true,  true,  true},   // 4
                                { true,  true, false,  true,  true, false,  true},   // 5
                                { true,  true,  true,  true,  true, false,  true},   // 6
                                { true, false, false, false,  true,  true, false},   // 7
                                { true,  true,  true,  true,  true,  true,  true},   // 8
                                { true,  true, false,  true,  true,  true,  true} }; // 9

// Buchstaben
const boolean CHARS[26][7] = { { true,  true,  true, false,  true,  true,  true},   // A
                               {false,  true,  true,  true,  true, false,  true},   // b
                               { true,  true,  true,  true, false, false, false},   // C
                               {false, false,  true,  true,  true,  true,  true},   // d
                               { true,  true,  true,  true, false, false,  true},   // E
                               { true,  true,  true, false, false, false,  true},   // F
                               { true,  true, false,  true,  true,  true,  true},   // g (identisch mit 9)
                               {false,  true,  true, false,  true,  true,  true},   // H
                               {false,  true,  true, false, false, false, false},   // I
                               { true, false,  true,  true,  true,  true, false},   // J
                               {false, false, false, false, false, false, false},   // blank, da K nicht darstellbar
                               {false,  true,  true,  true, false, false, false},   // L
                               {false, false, false, false, false, false, false},   // blank, da M nicht darstellbar
                               {false, false,  true, false,  true, false,  true},   // n
                               {false, false,  true,  true,  true, false,  true},   // O
                               { true,  true,  true, false, false,  true,  true},   // P
                               { true,  true, false, false,  true,  true,  true},   // q
                               {false, false,  true, false, false, false,  true},   // r
                               { true,  true, false,  true,  true, false,  true},   // S (identisch mit 5)
                               {false,  true,  true,  true, false, false,  true},   // t
                               {false,  true,  true,  true,  true,  true, false},   // U
                               {false, false, false, false, false, false, false},   // blank, da V nicht darstellbar
                               {false, false, false, false, false, false, false},   // blank, da W nicht darstellbar
                               {false, false, false, false, false, false, false},   // blank, da X nicht darstellbar
                               {false, false, false, false, false, false, false},   // blank, da Y nicht darstellbar
                               { true, false,  true,  true, false,  true,  true} }; // Z (identisch mit 2)
const boolean DASH[7] = {false, false, false, false, false, false, true};
const boolean UNDERSCORE[7] = {false, false, false, true, false, false, false};
const boolean DEGREE[7] = {true, true, false, false, false, true, true};
const boolean EQUALS[7] = {false, false, false, true, false, false, true};
const boolean DOUBLEQUOTE[7] = {false, true, false, false, false, true, false};
const boolean SINGLEQUOTE[7] = {false, true, false, false, false, false, false};
const boolean BRACKET_OPEN[7] = {true, true, true, true, false, false, false};
const boolean BRACKET_CLOSE[7] = {true, false, false, true, true, true, false};
const boolean MUE[7] = {false, true, true, false, false, true, true};

// Zustände der Segmente
const boolean checkState = true;
boolean state[18][7];

// TODO Optimierung
// * Das mittlere Segment flipt an manchen Stellen nur unzuverlässig zurück. Scheint nicht an der Pulslänge oder dem Abstand des Pulses zu liegen. Mechanischer Verschleiß?
// * Globale Variablen für Pin-Adressen auf byte umstellen?
// * Arrays um ungenutzte Zeilen reduzieren und Adressierung entsprechend umstellen

// TODO Features
// * Funktion, die beliebig langen String in einer Zeile schreibt. Wenn String länger als Zeile muss dieser durchlaufen.

void setup() {
  pinMode(FP_SPALTE_B1, FUNCTION_3); // Alternativer Modus für TX: GPIO1
  
  // Alle Pins als OUTPUT setzen
  pinMode(FP_SPALTE_A0, OUTPUT);
  pinMode(FP_SPALTE_A1, OUTPUT);
  pinMode(FP_SPALTE_A2, OUTPUT);
  pinMode(FP_SPALTE_B0, OUTPUT);
  pinMode(FP_SPALTE_B1, OUTPUT);
  pinMode(FP_ZEILE_A0, OUTPUT);
  pinMode(FP_ZEILE_A1, OUTPUT);
  pinMode(FP_ZEILE_A2, OUTPUT);
  pinMode(FP_ZEILE_B0, OUTPUT);
  pinMode(FP_ENABLE, OUTPUT);

  // ENABLE muss auf HIGH gehalten werden und darf nur für kurze Pulse auf LOW gesetzt werden!
  digitalWrite(FP_ENABLE, HIGH);

  // Adressen auf HIGH initialisieren
  digitalWrite(FP_SPALTE_A0, HIGH);
  digitalWrite(FP_SPALTE_A1, HIGH);
  digitalWrite(FP_SPALTE_A2, HIGH);
  digitalWrite(FP_SPALTE_B0, HIGH);
  digitalWrite(FP_SPALTE_B1, LOW);
  digitalWrite(FP_ZEILE_A0, HIGH);
  digitalWrite(FP_ZEILE_A1, HIGH);
  digitalWrite(FP_ZEILE_A2, HIGH);
  digitalWrite(FP_ZEILE_B0, HIGH); // LOW=SET, HIGH=UNSET

  delay(2000); // Zeit zum Spannungsaufbau (Kondensator) lassen...

  resetState();
  writeVersion();

  delay(2000);

  // Netzwerkkonfiguration
  setupWiFi();
  client.setServer(MQTT_SERVER, 1883);  
  client.setCallback(callback);
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  connectWiFi();
}

void connectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    writeNoWiFi();
    delay(1000);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i=0; i<length; i++) {
    char receivedChar = (char)payload[i];
    msg += receivedChar;
  }

  if (msg.equals("s77")) {
    writeSection77();
  } else {
    writeString(msg);
    
    if (msg.length() < 13) {
      for (int i = msg.length(); i < 14; i++) {
        clearAt(getSpalte(i));
      }
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("Klapperbrett")) {
      // Verbunden, entsprechendes Topic abonnieren
      client.subscribe("klapperbrett");
      clearAll();
    } else {
      connectWiFi();
      writeDisconnect();
  delay(2000);
      
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

byte getSpalte(byte idx) {
  if (idx > 13) {
    return 0;
  }
  
  if (idx >= 0 && idx < 5) {
    return 5 - idx;
  }

  if (idx >= 5 && idx < 10) {
    byte spalte = 17 - idx;
    if (spalte % 8 == 0) {
      return spalte - 1;
    } else {
      return spalte;
    }
  }

  if (idx < 12) {
    return 28 - idx;
  } else {
    return 27 - idx;
  }
  
}

void writeNoWiFi() {
  writeChar(5,'n');
  writeChar(4,'o');
  writeChar(3,' ');
  writeChar(2,' ');
  writeChar(1,' ');
  writeChar(12,'n');
  writeChar(11,'e');
  writeChar(10,'t');
  writeChar(9,' ');
  writeChar(7,' ');
  writeChar(18,'c');
  writeChar(17,'o');
  writeChar(15,'n');
  writeChar(14,'n');
}

void writeDisconnect() {
  writeChar(5,'n');
  writeChar(4,'o');
  writeChar(3,' ');
  writeChar(2,' ');
  writeChar(1,' ');
  writeChar(12,'s');
  writeChar(11,'e');
  writeChar(10,'r');
  writeChar(9,' ');
  writeChar(7,' ');
  writeChar(18,'c');
  writeChar(17,'o');
  writeChar(15,'n');
  writeChar(14,'n');
}

void writeVersion() {
  writeChar(5,'s');
  writeChar(4,'u');
  writeChar(3,'b');
  writeChar(2,'c');
  writeChar(1,'l');
  writeChar(12,'i');
  writeChar(11,'e');
  writeChar(10,'n');
  writeChar(9,'t');

  writeChar(18,'r');
  writeChar(17,'0');
  writeChar(15,'0');
  writeChar(14,'3');
}

void writeSection77() {
  writeChar(5,'S');
  writeChar(4,'E');
  writeChar(3,'C');
  writeChar(12,'t');
  writeChar(11,'i');
  writeChar(10,'o');
  writeChar(9,'n');

  for (int i = 0; i < 78; i++) {
    byte links = i / 10;
    byte rechts = i % 10;
    writeNumber(18, links);
    writeNumber(17, rechts);
  }
}

// ----------------------------------------------------------------------
// Lib-Funktionen
// ----------------------------------------------------------------------

void writeString(String s) {
  for (int i = 0; i < s.length(); i++) {
    writeChar(getSpalte(i), s.charAt(i));
  }
}

void writeDegree(byte spalte) {
  for (int i = 0; i < 7; i++) {
    writeAt(spalte, i+1, DEGREE[i]);
  }
}

void writeMue(byte spalte) {
  for (int i = 0; i < 7; i++) {
    writeAt(spalte, i+1, MUE[i]);
  }
}

void writeChar(byte spalte, char c) {
  int number = c - '0';
  if (number >= 0 && number <= 9) {
    writeNumber(spalte, number);
    return;
  }
  
  if (c == '-') {
    for (int i = 0; i < 7; i++) {
      writeAt(spalte, i+1, DASH[i]);
    }
    return;
  }

  if (c == '_') {
    for (int i = 0; i < 7; i++) {
      writeAt(spalte, i+1, UNDERSCORE[i]);
    }
    return;
  }

  if (c == '=') {
    for (int i = 0; i < 7; i++) {
      writeAt(spalte, i+1, EQUALS[i]);
    }
    return;
  }

  if (c == '"') {
    for (int i = 0; i < 7; i++) {
      writeAt(spalte, i+1, DOUBLEQUOTE[i]);
    }
    return;
  }

  if (c == '\'' || c == '`') {
    for (int i = 0; i < 7; i++) {
      writeAt(spalte, i+1, SINGLEQUOTE[i]);
    }
    return;
  }

  if (c == '[' || c == '(' || c == '{') {
    for (int i = 0; i < 7; i++) {
      writeAt(spalte, i+1, BRACKET_OPEN[i]);
    }
    return;
  }

  if (c == ']' || c == ')' || c == '}') {
    for (int i = 0; i < 7; i++) {
      writeAt(spalte, i+1, BRACKET_CLOSE[i]);
    }
    return;
  }
  
  byte charArrayPos = getPosInAlphabet(c);
  if (charArrayPos >= 0 && charArrayPos < 26) {
    for (int i = 0; i < 7; i++) {
      writeAt(spalte, i+1, CHARS[charArrayPos][i]);
    }
  } else {
    clearAt(spalte);
  }
}

void writeNumber(byte spalte, byte number) {
  byte mod = number % 10;
  for (int i = 0; i < 7; i++) {
    writeAt(spalte, i+1, ZAHLEN[mod][i]);
  }
}

void clearAt(byte spalte) {
  for (int i = 1; i < 8; i++) {
    writeAt(spalte, i, false);
  }
}

void fillAt(byte spalte) {
  for (int i = 1; i < 8; i++) {
    writeAt(spalte, i, true);
  }
}

void clearAll() {
  writeAll(false);
}

void fillAll() {
  writeAll(true);
}

// ----------------------------------------------------------------------
// Hilfsfunktionen
// ----------------------------------------------------------------------

byte getPosInAlphabet(char c) {
  return tolower(c) - 'a';
}

void resetState() {
  for (int i = 1; i < 19; i++) {
    if (i == 6 || i == 8 || i == 13 || i == 16) {
      // Unbelegte Spaltenadressen
      continue;
    }
    for (int j = 1; j < 8; j++) {
      writeAtStateless(i, j, false);
    }
  }
}

void writeAll(boolean set) {
  for (int i = 1; i < 19; i++) {
    if (i == 6 || i == 8 || i == 13 || i == 16) {
      // Unbelegte Spaltenadressen
      continue;
    }
    writeAllSegments(i, set);
  }
}

void writeAllSegments(byte spalte, boolean set) {
  for (int i = 1; i < 8; i++) {
    writeAt(spalte, i, set);
  }
}

void writeAt(byte spalte, byte zeile, boolean set) {
  if (!checkState || state[spalte-1][zeile-1] != set) {
    writeAtStateless(spalte, zeile, set);
  }
}

void writeAtStateless(byte spalte, byte zeile, boolean set) {
  for (int i = 0; i < 5; i++) {
    boolean isSet = bitRead(spalte, i);
    if (isSet) {
      digitalWrite(SPALTE_ADDR[i], HIGH);
    } else {
      digitalWrite(SPALTE_ADDR[i], LOW);
    }
  }

  for (int i = 0; i < 3; i++) {
    boolean isSet = bitRead(zeile, i);
    if (isSet) {
      digitalWrite(ZEILE_ADDR[i], HIGH);
    } else {state[spalte-1][zeile-1] = set;state[spalte-1][zeile-1] = set;
      digitalWrite(ZEILE_ADDR[i], LOW);
    }
  }

  if (set) {
    digitalWrite(FP_ZEILE_B0, LOW);
  } else {
    digitalWrite(FP_ZEILE_B0, HIGH);
  }

  fire();
  state[spalte-1][zeile-1] = set;
}

void fire() {
  digitalWrite(FP_ENABLE, LOW);
  delay(1);
  digitalWrite(FP_ENABLE, HIGH);
  delay(1);

  // Adresse zurücksetzen
  // TODO Notwendig? ggf. entfernen...
  digitalWrite(FP_SPALTE_A0, HIGH);
  digitalWrite(FP_SPALTE_A1, HIGH);
  digitalWrite(FP_SPALTE_A2, HIGH);
  digitalWrite(FP_SPALTE_B0, HIGH);
  digitalWrite(FP_SPALTE_B1, LOW);
  digitalWrite(FP_ZEILE_A0, HIGH);
  digitalWrite(FP_ZEILE_A1, HIGH);
  digitalWrite(FP_ZEILE_A2, HIGH);
  digitalWrite(FP_ZEILE_B0, HIGH);
}
