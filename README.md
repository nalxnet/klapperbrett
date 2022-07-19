# klapperbrett

Die Klappenanzeige (Salzkotten EC2000-FP6) stammt aus einer Zapfsäule und zeigte die üblichen Werte (Literpreis, Gesamtmenge, Gesamtpreis) an.

Es sind drei Anschlüsse vorhanden:
* Pfostenstecker 10-polig, von außen zugänglich: vmtl. Anschluss an den Rest der Zapfsäule, inkl. Stromversorgung (2 Pins +24V, 2 Pins GND).
* Pfostenstecker 12-polig(?), nur von innen zugänglich: vmtl. Service-Anschluss zur Neuprogrammierung, kein +24V oder +5V, nur GND.
* 2 Bohrungen für +24V und GND, nicht verlötet.

Steuerung erfolgt original über einen 8-bit Mikrocontroller, der in einen 40-pol. IC-Sockel gesteckt ist. Hier setzen wir an und ersetzen diesen mit einem Arduino/ESP.

Es gibt 3 Zeilen, die oberen beiden haben 5 Elemente, die unterste hat 4. Jedes Element hat 7 Segmente zur Anzeige einer Zahl. Logisch sind zur Ansteuerung alle Elemente in einer Reihe aufgebaut, dadurch lassen sich die einzelnen Segmente als Matrix darstellen, wobei die Elemente die Spalten und die Segmente die Zeilen darstellen. Jedes Segment besteht aus einem Elektromagneten, der je nach Stromfluss anders polarisiert ist und dadurch das Segment in An- bzw. Aus-Position bewegt. Dafür ist eine Seite der Spule direkt an die Spaltenleitung angeschlossen. Die andere Seite der Spule ist mit jeweils einer Diode an zwei Elementleitungen angeschlossen. Eine zum Bewegen in An-Position, die andere für die Aus-Position.

Die Ansteuerung der einzelnen Segmente übernehmen zwei FP2800A. Der obere (FPZ) steuert die Zeilen, der untere (FPS) die Spalten.

Pinbelegung Mikrocontroller (alle nicht aufgeführten sind unbekannt):
```
1: FPS A0
2: FPS A1
3: FPS A2
4: FPS B0
5: FPS B1
6: (über JP1 an PCF8582C-2 Pin SDA --> I2C)
7: FPS ENABLE
...
20: GND
33: FPZ ENABLE
34: FPZ B0, FPZ DATA, FPS DATA
35: FPZ B1
...
37: FPZ A2
38: FPZ A1
39: FPZ A0
40: +5V
```


Ansteuerung der Segmente erfolgt durch Setzen der Spalten- und Zeilenadresse (je 8 Bit), DATA wird durch invertierenden Schmitt-Trigger bereits passend gesetzt. Um das Segment zu schalten muss abschließend ein kurzer Puls an beiden ENABLE-Pins gleichzeitig (HIGH nach LOW) erfolgen, für dessen Zeit die Spalten- und Zeilenleitung tatsächlich gesetzt wird.

Adressierung der Elemente erfolgt von oben nach unten, von rechts nach links. Freie Element-Plätze sind ebenfalls adressiert, 8 und 16 werden übersprungen.

```
 6  5  4  3  2  1

13 12 11 10  9  7

   19 18 17 15 14
```


Die Adressierung der Segmente erfolgt gegen den Uhrzeigersinn, beginnend mit dem obersten Segment und endend mit dem mittleren Segment. 1-7 schalten das Segment aktiv, 9-15 schalten das Segment inaktiv.

Netzteil 24V, mind. 200mA
 
Pinbelegung X5? (extern)
```
...
2: +24V
3: +24V
...
5: GND
...
9: GND
...
```
 
Pinbelegung X6
 
```
... 
14: GND
15: GND
...
```
 
Pinbelegung X7
 
```
1: +24V
2: GND
```