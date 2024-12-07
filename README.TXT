===========================================================================
Calculator - Rechner - Version 1.61                        7. Dezember 2005
===========================================================================

                    (c) 1991-2024 by Martin Gaeckler

Diese Datei entaelt Hinweise  sowohl fuer die  Windows-Version als auch die
GEM-Version (Atari ST), Sowie Mac OS X 10.1. Wenn Sie diese Datei auf einem
MS-DOS-Rechner lesen, benutzen Sie bitte den Notizblock von MS-Windows oder 
einen anderen Editor, der den ANSI-Zeichensatz unterstuetzt.

Der  Rechner ist die  Simulation eines wissenschaftlichen  Taschenrechners. 
Die  Benutzung  ist kostenlos.  Es darf frei kopiert werden,  der  Vertrieb 
ueber einen Public Domain Versand ist gestattet, es musz aber ein Belgexem-
plar an den Autor geschickt werden. 

Wenn Sie  immer ueber neueste  Entwicklungen informiert  werden wollen,  so 
senden Sie eine kurze Mitteilung an den Autor. Sie werden dann in einer Da-
tenbank gespeichert und bei Freigabe einer neuen Version informiert.  Diese
Registrierung ist kostenlos, nachfolgende Versionen werden jedoch nur gegen
Zahlung einer Lizenzgebuehr verschickt.

Geplant sind u.a. folgende Features:
- Einfache Programmierbarkeit
- Funktionsgraphen
- Strukturierte Programmiersprache

Anschrift: Martin Gaeckler, Hoffmannsthalweg. 14, A-4030 Linz
Web:       https://www.gaeckler.at
E-Mail:    martin@gaeckler.at

********************************* Achtung *********************************
*                                                                         *
* Dies ist keine ausfuehrliche  Bedienungsanleitung. Es wird davon ausge- *
* gangen, dasz der Benutzer mit Taschenrechnern umgehen kann.             *
*                                                                         *
***************************************************************************


Inhalt der Archive
==================

Atari ST (CALC160.LZH):
-----------------------
CALC.PRG          Das ausfuehrbare Programm.
CALC881.PRG       Spezial-Version fuer den Atari TT
CALC.RSC          Die Resourcedefinition
README.TXT        Diese Datei

Windows (CALCWIN.ZIP):
----------------------
CALC.EXE          Das ausfuehrbare Programm.
CALC32.EXE        Das ausfuehrbare Programm fuer 32-Bit Windows.
CALC.HLP          Die Hilfedatei
README.TXT        Diese Datei.

Mac OS X (CalcMacOS_X.dmg.gz):
------------------------------
CalcMacOS_X.dmg   Ein DiskVolume zum …ffnen mit Diskcopy.
- Calculator      Das ausfŸhrbare Programm.
- README.TXT      Diese Datei.


Installation und Starten des Programmes
=======================================

Atari ST:
---------
Kopieren  Sie die Dateien  CALC.PRG und  CALC.RSC in ein Verzeichniss Ihrer 
Wahl.  Jetzt koennen Sie den  Taschenrechner wie gewohnt durch  Doppelklick
auf das Icon von CALC.PRG starten.
Das Programm koennen Sie aber ebenso als Accessory verwenden. Hierzu kopie-
ren Sie die Dateien  CALC.PRG und CALC.RSC in das  Wurzelverzeichniss Ihres
Startlaufwerks  (bzw. Startdiskette) und  benennen  die Datei  CALC.PRG  in 
CALC.ACC um.  Nach einem Neustart Ihres Rechners erscheint im Deskmenue ein
neuer Menuepunkt "Calculator".

Windows:
--------
Kopieren  Sie die Datei CALC.EXE  in ein Verzeichniss Ihrer Wahl.  Beachten
Sie aber dabei, dasz sich im  Windows-Verzeichniss bereits ein Taschenrech-
ner  mit  dem gleichen Namen  befindet.  Sie koennen diesen  jedoch getrost 
ueberschreiben, da der neue besser ist.
Sie  koennen nun den  Taschenrechner im Programm  Manager installieren oder 
vom Datei Manager aus  ganz normal starten.  Das Ausfuehrbare Programm ent-
haelt 2 Icons:  Eine verkleinerte  Version der Dialogbox und  einen kleinen
Abakus. Welchen der Icons Sie im  Program Manager installieren bleibt Ihnen
ueberlassen.


Mac OS X:
---------
Kopieren Sie die Datei Calculator in ein Verzeichnis Ihrer Wahl.

Profile
=======

Der  Taschenrechner merkt  sich automatisch,  das aktuelle  Panel und deren 
Position. Hier folgenden grundsaetzliche Hinweise:

Atari ST:
---------
Der Taschenrechner  merkt sich die Information in der Datei CALC.INF. Diese
Datei ist eine Binaerdatei und kann nicht gelesen werden. Die Datei wird im
aktuellen Verzeichniss erwartet,  falls sie nicht gefunden wird, so legt er
sie automatisch im Wurzelverzeichniss des Laufwerks C: an. Das Profile wird
immer dann gesichert, wenn  das Panel gewechselt  oder geschlossen wird und
wenn  die aktuelle  Hauptapplikation terminiert.  Die Datei wird verworfen,
wenn die  eingestellte Aufl”sung eine  andere ist, als  die Aufl”sung,  die
beim letzten Programmlauf verwendet wurde.

Windows:
--------
Die Informationen  werden in der Datei WIN.INI im  Abschnitt [WinCalc]. Die
Datei wird aktualisiert, wenn  das Panel  gewechselt oder  geschlossen wird
und wenn Windows beendet wird.

Mac OS X:
---------
Die Information wird unter com.cresd.Calculator.plist im Preferences Ordner
des Benutzers gespeichtert. Zur Zeit wird die Fensterposition nicht gespei-
chert.

Menue-Referenz
==============

Edit/Copy:              Kopiert den Inhalt der Anzeige auf das Clipboard.
    /Paste:             Laedt  das Clipboard. Befindet  sich am  Anfang des
                        Textes eine  Flieszkommazahl, so wird diese zur An-
                        zeige gebracht. Ansonsten wird der Inhalt des Clip-
                        board ignoriert.
    /Quit:              Schlieszt das Fenster des Taschenrechners und been-
                        det das Programm  (letzteres gilt  auf dem Atari ST
                        nur dann, wenn es nicht  als Accessory geladen wur-
                        de).
Panel/Large:            Es wird das grosze Fenster benutzt.
     /Scientific:       Es wird der wissenschaftiche  Rechner benutzt, die-
                        ser kennt nur Flieszkommazahlen.
     /Computer:         Es wird das  Panel fuer  Computeranwender  benutzt.
                        Dieser  kennt alle Datentypen und nur die wichtigs-
                        ten Operatoren bzw. Funktionen.
     /Small:            Es  wird das  kleinste  Rechner  verwendet,  dieser 
                        kennt  nur  die   wichtigsten  Operatoren  und  die 
                        Flieszkommazahlen.
Calc/About... bzw.
Help/About...:          Zeigt die  Copyrightnotiz  und  Versionsnummer  des 
                        Taschenrechners.

Atari ST:
---------
Das Calc-Menue enthaelt auszerdem die Accessories,  falls der Rechner nicht
selbst als Accessory geladen wurde.

Ist der Rechner  als Accessory geladen,  musz  das Menu durch Mausklick auf 
einen der Titeleintraege eingeschaltet werden. Dann verhaelt es sich genau-
so wie das Standard-GEM-Menu.
                
Das Clipboard wird  automatisch vom  Taschenrechner installiert, falls dies
nicht  schon eine  andere Applikation  getan hat.  In diesem Falle wird das
Verzechniss  C:\CLIPBRD verwendet.  Es wird jedoch nicht  automatisch ange-
legt.


Mac OS X:
---------
Die Position der Befehle About... und Quit entspricht den Aquarichtlinien,
d.h. diese Befehle befinden sich im Menuepunkt Calc.

Windows:
--------
Die Windowsversion (16- und 32-Bit) wird mit einer Hilfedatei ausgeliefert, 
die ueber das Hilfemenue angezeigt werden kann.

Bedeutung der Buttons und deren Tastauraequivalente
===================================================

Hinweis:
Nicht alle Buttons befinden sich auf allen Panels

-------------+--------------+-----------------------------------------------
Button *)    | Tastatur *)  | Bedeutung
-------------+--------------+----------------------------------------------
                      Tasten zur Eingabe der Zahlen
-------------+--------------+----------------------------------------------
[0]...[9]    | 0 ... 9      | Dezimalziffern
[A]...[F]    | a/A ... f/F  | Hexziffern
[.]          | .            | Dezimalpunkt
[Exp]        | Tab          | Exponentialtaste, wenn noch keine Ziffer ein-
             |              | gegeben  wurde,  ergibt sie den  Wert von Pi.
             |              | Bei komplexen Zahlen schalten Sie zum naech-
             |              | sten Feld.
[ñ]([±])     |              | Vorzeichentaste (Negation)
[ø'"]([°´"]) |              | Winkeltaste   Mit dieser  Taste  koennen  Sie
             |              | Winkelgroeszen in  Grad-Minuten-Sekunden ein-
             |              | geben.  Zusammen mit der Inverstaste wird die
             |              | angezeigte Dezimalzahl in Grad-Minuten-Sekun-
             |              | den umgerechnet.
-------------+--------------+----------------------------------------------
                               Operatoren
-------------+--------------+----------------------------------------------
[BK]         |              | Binomialkoeffizient (x ueber y) = 
             |              | x * (x-1) * (x-2) * ... * (x-y+1)
             |              | ---------------------------------
             |              |              y!
             |              | fuer x reel und y natuerliche Zahl auszer 0
             |              | fuer y = 0 gilt x ueber y = 1
             |              | -> nur im "Scientific panel" verfuegbar
[]([pow])   |              | Potenzoperator (x hoch y)
[]([rot])   |              | Wurzeloperator (y-te Wurzel aus x)
[mod]        | %            | modulo operator
[*]          | *            | Produktoperator
[ö]([÷])     | /            | Divisionsoperator
[+]          | +            | Summenoperator
[-]          | -            | Differenzoperator
[and]        | &            | und (binaer)
[xor]        | ^ (#)        | exclusive oder (binaer)
[or]         | |            | inclusive oder (binaer)
-------------+--------------+----------------------------------------------
                                Funtionen
-------------+--------------+----------------------------------------------
[Inv]        |              | Aktivierung der Umkehrfunktionen
[hyp]        |              | Aktivierung der Hyperbelfunktionen
[sin]        |              | sinus
[cos]        |              | cosinus
[tan]        |              | tangens
[log]        |              | dekadischer logarithmus
[ln]         |              | natuerlicher logarithmus
[1/x]        |              | Reziprokwert
[û]([sqr])   |              | Quadratwurzel
[abs]        |              | Betragsfunktion
[not]        | ~            | nicht (binaer)
[int]        |              | ganzzahliger Anteil
[frc]        |              | Nachkommastellen
[x!]         | !            | Fakultaet
-------------+--------------+----------------------------------------------
                               Betriebsmodi
-------------+--------------+----------------------------------------------
[Rnd]        |              | Festlegung der signifikanten Ziffern
[Fix]        |              | Festlegung der Nachkommastellen
[Deg]        |              | Winkel werden in Graden gemessen
[Rad]        |              | Winkel werden in Bogenmasz gemessen
[Gra]        |              | Winkel werden in GON (Neugrad) gemessen
[Hex]        |              | Hexadezimalsystem
[Dec]        |              | Dezimalsystem
[Oct]        |              | Oktalsystem
[Bin]        |              | Dualsystem
[Flt]        |              | Flieszkommazahlen, ein eingegebener Term wird
             |              | geloescht, wenn der Datentyp gewechselt wird.
             |              | Flieszkommazahlen  werden  grundsaetzlich  im
             |              | Dezimalsystem  angezeigt.  Mit der  Inv-Taste
             |              | koennen Sie komplexe Zahlen bearbeiten. 
[Lng]        |              | Langwortzahlen  mit der Inv-Taste vorzeichen-
             |              | los,  ein eingegebener  Term wird  geloescht,
             |              | wenn der Datentyp  gewechselt wird.  Long ist
             |              | der  Standarddatentyp,  wenn das Zahlensystem
             |              | vom Dezimalsystem zu einem anderen Zahlensys-
             |              | tem gewechselt und  vorher der Datentyp Float
             |              | eingestellt war.
[Wrd]        |              | Wortzahlen  mit der Inv-Taste  vorzeichenlos,
             |              | ein  eingegebener Term  wird geloescht,  wenn
             |              | der Datentyp gewechselt wird.
[Byt]        |              | Bytezahlen  mit der  Inv-Taste vorzeichenlos,
             |              | ein  eingegebener  Term wird geloescht,  wenn
             |              | der Datentyp gewechselt wird.
[LR]         |              | Lineare Regression:         y = k + m * x
[LNR]        |              | Logarithmische Regression:  y = k + m * ln x
[EXR]        |              | Exponentielle Regression:   y = k * e^(m*x)
[PR]         |              | Leistungsregression:        y = k * x^m
-------------+--------------+----------------------------------------------
                                Speicher
-------------+--------------+----------------------------------------------
[Min]        |              | aktueller Operand in das M-Register uebertra-
             |              | gen
[MR]         |              | Inhalt des M-Register anzeigen
[M+]         |              | aktuellen Operand zum M-Register addieren
[M-]         |              | aktuellen Operand vom M-Register subtrahieren
[X/Y]        |              | Die aktuelle Anzeige wird mit dem letzten
             |              | Operanden vertauscht.
[X/M]        |              | Die aktuelle Anzeige wird mit dem Speicher
             |              | vertauscht.
-------------+--------------+----------------------------------------------
                          Statistik & Regression
-------------+--------------+----------------------------------------------
[Xin]        | x            | Statistikeingabetaste. Soll ein Wert mehrmals
             |              | eingegeben  werden,  so kann  dies durch  die
             |              | Taste [*] erreicht  werden. Beispiel der Wert
             |              | 5 soll 20 mal  eingegeben werden, dazu druek-
             |              | ken Sie die  Tasten [2][0] [*] [5] [Xin]. Zu-
             |              | sammen mit [Inv] koennen  einzelne (oder auch
             |              | mehrere) wieder geloescht werden.
[Yin]        | y            | Regressionseingabetaste. Wird nach dem ersten
             |              | X-Wert ein Y-Wert eingegeben, so schaltet der
             |              | Rechner  automatisch auf  Regression um.  Bei
             |              | der Eingabe der  Zahlenpaare muž der  Regres-
             |              | sionstyp noch nicht bestimmt werden.
[n]          | n            | Anzahl der Eingabewerte
[äx]([Ex])   |              | Summe der X-Werte
[äxý]([Ex²]) |              | Summe der Quadrate von x
[äy]([Ey])   |              | Summe der Y-Werte
[äyý]([Ey²]) |              | Summe der Quadrate von y
[äxy]([Ey²]) |              | Summe der Produkte von x * y
[³]([x])     |              | Arithmetisches Mittel ueber alle x
[ån]([dn])   |              | Grundgesamtheits-Standardabweichung
[ån1]([dn1]) |              | Stichproben-Standardabweichung 
[k]          |              | Konstanter Wert k der Regressionsgleichung
[m]          |              | Regressionskoeffizient m der Regressionsglei-
             |              | chung
[r]          |              | Korrleationskoeffizient der  Regressionsglei-
             |              | chung
[BST]        |              | Es  wird die  Regressionsgleichung  bestimmt,
             |              | die am besten zu  den vorliegenden Eingabeda-
             |              | ten  paszt.  Der entsprechende  Regressiontyp
             |              | wird  eingestellt  und  der Grad  (in %)  der 
             |              | Uebereinstimmung angezeigt.
[X]          |              | X-Wert der Regressionsgleichung Y gegebenen
[Y]          |              | Y-Wert der Regressionsgleichung X gegebenen
-------------+--------------+----------------------------------------------
             |              | Sonstige Tasten
-------------+--------------+----------------------------------------------
[[(]         | (            | Klammer auf
[)]]         | )            | Klammer zu
[=]          | Enter/Return | Ergebnisstaste
[C]          | Backspace    | Loeschen des aktuellen Operanden
[ AC ]       | Esc          | Kompletten Term loeschen, Fehlerbedingung zu-
             |              | ruecksetzen.  Zusammen mit der Inv-Taste wird
             |              | auch der Statistikspeicher geloescht.
[Ran]        |              | Zufallszahlen zwischen 0 incl. und 1 excl.
[Eng]        |              | Diese Taste hat zur Zeit keine Bedeutung
-------------+--------------+-----------------------------------------------

*) In  Klammern die  Bezeichnung der  Windows-Version, falls diese sich von
   der Atari-Version unterscheidet.

Folgende  Funktionen haben KEINE Wirkung,  wenn ein Integer-Datentyp (Byte,
U-Byte, Word, U-Word, Long, U-Long ) eingestellt ist:

[sin], [cos], [tan], [Ran], [log], [ln], [1/x], [û]([sqr]), [int], [frc],
[x!],  [Xin], [Yin], [n],   [äx]([Ex]),  [äxý]([Ex²]),      [äy]([Ey]),
[äyý]([Ey²]), [äxy]([Ey²]), [³]([x]),    [ån]([dn]),        [ån1]([dn1]),
[k],   [m],   [r],   [X],   [Y],  [Rnd], [Fix], [ø'"]([°´"])

Anmerkung: Die Wirkung von  [Rnd] und [Fix]  zeigt sich  erst dann (wieder)
           auf der Anzeige, wenn wieder der Float-Datentyp eingestellt ist.

Folgende Funktion hat KEINE Wirkung,  wenn der Flieszkomma-Datentyp (Float)
eingestellt ist:

[not]

Folgende Operatoren erzeugen einen Fehler, wenn ein Integer-Datentyp (Byte,
U-Byte, Word, U-Word, Long, U-Long ) eingestellt ist:

[BK], [] ([pow]), [] ([rot])

Folgende  Operatoren erzeugen  einen Fehler, wenn  der Flieszkomma-Datentyp
(Float) eingestellt ist:

[mod], [and], [xor], [or]

Die Anwendung dieser  Operatoren und  Funktionen ist aber  in den genannten
Faellen nicht moeglich.


Konstantenrechnung
==================

Wuenschen sie  den linken Operanden  einer Operation mehrfach zu verwenden,
so brauchen Sie  nur den Operator  2 mal zu druecken. Anschlieszend muessen
nur noch die rechten  Operanden eingegeben und  jeweils die  Ergebnisstaste
betaetigt werden.


Warnungen
=========

() Overflow         Die Schachtelungstiefe fuer die  Klammern wurde  ueber-
                    schritten.
Out of range        Die eingegebene Zahl ist  ausserhalb des gueltigen Zah-
                    lenbereichs  und kann deshalb  nicht weiter verarbeitet
                    werden.

Warnungen koennen  einfach durch Betaetigung  der Taste [C] behoben werden.
Der bisher eingebene Term kann weiter benutzt werden.


Fehlermeldungen des Rechners
============================

Buffer empty        Der Statistik- bzw.  Regressionsspeicher enthaelt keine
                    Daten
Division by zero    Division durch 0
General error       Allgemeiner Fehler (nicht naeher spezifiziert)
Horizontal line     Der Graph der Regressionsgleichung ist eine horizontale
                    Linie. Es gibt deshalb keine Umkehrfunktion.
Illegal argument    Das Argument einer Funktion ist nicht erlaubt
Illegal operand     Einer der Operanden eines Operators ist nicht erlaubt
Illegal operator    Der  Operator ist mit  dem eingestellten Datentyp nicht
                    anwendbar.
Inexact result      Das Ergebnis einer Funktion waere zu ungenau.
+Inf/-Inf           Das  Ergebnis einer  Berechnung ist zu  grosz (oder un-
                    endlich) um dargestellt werden zu koennen.
Nan                 Das Ergebnis einer Berechnung ist nicht definiert.
No operand expected Ein Operand ist in diesem Zusammenhang nicht erlaubt.
No regression data  Sie haben versucht eine  Regressionsberechnung durchzu-
                    fuehren, obwohl keine Regressionsdaten vorliegen
No statistic data   Sie haben versucht eine Statistikfunktion auszufuehren,
                    obwohl keine Statistikdaten eingegeben wurden.
Stack overflow      Der interne Speicherbereich fuer Zwischenergebnisse ist
                    voll.  Dieser Fehler darf  nie auftreten.  Bitte teilen
                    sie dem Autor die Umstaende mit,  wie sie diesen Fehler
                    erzeugt haben.
Too few regression data
                    Sie haben versucht eine  Regressionsberechnung durchzu-
                    fuehren, obwohl zu wenige Regressionsdaten vorliegen.
Too few statistic data
                    Sie haben versucht eine Statistikfunktion auszufuehren,
                    obwohl zu wenig Statistikdaten eingegeben wurden.
Too many values     Sie haben  versucht mehr  Eingabedaten zu  l”schen  als
                    vorhanden sind
X not expected      Sie  haben einen  X-Wert eingegeben,  obwohl das letzte
                    Zahlenpaar noch nicht vollstaendig ist.
Y not expected      Sie  haben einen  Y-Wert eingegeben,  obwohl das letzte
                    Zahlenpaar bereits vollstaendig ist.
Y missing           Sie haben versucht eine  Regressionsberechnung durchzu-
                    fuehren, obwohl das letzte  Zahlenpaar noch nicht voll-
                    staendig ist.

Im Falle  eines Fehlers  wird nur noch [ AC ] vom  Rechner akzeptiert.  Der
bisher eingegebene Term geht dabei verloren.


Technisches
===========

Der Rechner ist komplett in C geschrieben. Das Modul,  das den eigentlichen
Rechner darstellt (fuer alle  Systeme) entaelt 2696 Zeilen.  Das Modul fuer
die Panels unter GEM enthaelt 1124 Zeilen, fuer Windows 622 Zeilen und fuer
Mac OS X 476 Zeilen.

Als Entwicklungssystem  wurde auf dem Atari ST Turbo C Version 2.0, auf dem
Windows-Rechner  Borland C++ 5.0 verwendet  und bei Mac OS X .  Geplant ist 
auszerdem eine Version  unter UNIX.  Das Rechnermodul wurde bereits erfolg-
reich mit dem MIPS-Compiler uebersetzt.

Die Flieszkommazahlen  unterliegen in ihrer Beschraenkung, dem des Entwick-
lungssystems.  Borland C++ kennt leider nur 8-Byte-Double-Variablen.  Daher
ist  der moegliche  Zahlenbereich  beschraenkt  auf +/-1e+/-300  bei ca. 15 
gueltigen Stellen,  waehrend das Atari  Entwicklunssystem etwa +/-1e+/-4000
bei ca. 19 gueltigen Stellen verkraftet.


Nachrichtenprotokoll (nicht fuer Mac OS X)
==========================================

Der folgende Abschnitt ist nur fuer Softwareentwickler interessant. Als nur
Anwender kann man sofort aufhoeren zu lesen.

Der Taschenrechner kann  auch Nachrichten einer  Hauptapplikation empfangen
und  Antworten  zurueckschicken.  Es gibt 2 Nachrichten zum  Rechner und  1
Antwort zur Hauptapplikation.

- Restart calculator (zum Rechner)

  Der Taschenrechner erthaelt folgende Nutzdaten:

  Nachrichtennummer:     Identifiziert die Nachricht
  Absender:              Identifiziert  den  Empfaenger der  Antwort  des
                         Rechners.
  Value:                 Zahl fuer die Anzeige
  Type:                  Datentyp (FLOAT, BYTE etc. )
  Radix:                 Zahlensystem (2, 8, 10, 16)
  Operator (nur Atari):  Der erste Operator

  Datentypen    0    Fliežkommazahl
                1    Byte
                2    Byte (vorzeichenlos)
                3    Word
                4    Word (vorzeichenlos)
                5    Langwort
                6    Langwort (vorzeichenlos)

  Radix         2    Binaersystem
                8    Oktalsystem
               10    Dezimalsystem
               16    Hexadezimalsystem

  Operatoren   (alle erlaubten Tastenanschlaege des Taschenrechners)

  Der  Taschenrechner wird  neu initialisiert,  oeffnet sein  Fenster und
  stellt die uebergebene Zahl in seinem Anzeigefeld dar. Auf dem Atari ST
  wird der Operator angenommen,  so dasz nur noch mehr der zweite Operand
  eingegeben  und die  Ergebnisstaste  betaetigt werden musz.  Sobald der
  Benutzer  das Taschenrechner Fenster  wieder schlieszt (bei Windows ge-
  nuegt es zu einer  anderen Applikation zu  schalten) wird  die  Antwort 
  wieder an die rufende Applikation zurueck geschickt.

- Get result (zum Rechner)

  Der Taschenrechner erthaelt folgende Nutzdaten:

  Nachrichtennummer:     Identifiziert die Nachricht
  Absender:              Identifiziert  den  Empfaenger der  Antwort  des
                         Rechners.

  Der Taschenrechner schickt  als Antwort sein  aktuelles Anzeigefeld mit
  eingestelltem Radix und Datentyp.

- Answer (vom Rechner)

  Die Hauptapplikation erthaelt folgende Nutzdaten:

  Nachrichtennummer:     Identifiziert die Nachricht
  Value:                 Zahl in der Anzeige
  Type:                  Datentyp (siehe oben)
  Radix:                 Zahlensystem (siehe oben)

  Als Antwort schickt der Rechner  die aktuelle Zahl in der Anzeige sowie
  den eingestellten Datentyp und Radix.

Der  Mechanismus  fuer den  Nachrichtenaustausch ist  natuerlich auf beiden
Systemen unterschiedlich.  Es folgt daher eine Beschreibung des Mechanismus
fuer beide Systeme getrennt.

Atari ST:
---------
Nachrichten werden mit der GEM-Funktion appl_write() zum Rechner geschickt.
Die Antworten koennen mit evnt_mesag bzw. evnt_multi gelesen werden.

Der 8 Worte umfassende Buffer, der verschickt bzw. gelesen wird hat folgen-
den Aufbau:

buffer[0]   : Nachrichtennummer
              Restart calculator     1024
              Get result             1025
              Answer                 1026
buffer[1]   : Absender, die Id erhalten Sie von appl_init()
buffer[2]   : 0
buffer[3]   : Hiword eines  Zeigers auf eine  Zeichenkette,  die eine  Zahl
              darstellen musz
buffer[4]   : Lowword des Zeigers
buffer[5]   : Datentyp
buffer[6]   : Radix
buffer[7]   : Operator

Die  Eintraege  buffer[3] - buffer[7] muessen bei "Get result" nicht belegt
sein.

Der Eintrag buffer[7] ist  bei "Answer" nicht definiert.  Der Zeiger  zeigt 
auf einen statischen String, der beim naechsten Aufruf ueberschrieben wird.

Windows:
--------
Nachrichten werden mit  der Windows-Funktion  PostAppMessage() zum  Rechner
geschickt.  Die Antworten  koennen mit  GetMessage() bzw. mit dem  Fenster-
Callback gelesen werden.  Dabei erhalten die  Parameter folgende Bedeutung:

message   : Nachrichtennummer
            Restart calculator      WM_USER
            Get result              WM_USER + 1
            Answer                  WM_USER + 2
wParam    : immer 0
lParam    : Handle auf eine  Datenstruktur im  globalen Heap  des  Systems.
            Diese Datenstruktur musz grundsaetzlich von der rufenden Appli-
            kation angelegt werden und darf erst,  nach dem der Rechner die
            Antwort geschickt hat, wieder freigegeben werden.

Die Datenstruktur hat (in der Programmiersprache C) folgenden Aufbau:

typedef struct
{
    int     callerType;		/* 0: Instance, 1: Window	 */
    union
    {
        HANDLE  instance;   /* instance handle for reply */
        HWND    window;     /* window handle for reply   */
    } id;                   /* who called calc           */
    int    type,            /* data type 0 - 6           */
           radix;           /* radix    2, 8, 10, 16     */
	char   value[64];       /* which value to begin      */
}CALCMSG;

callerType: Gibt an  welchen Typ  die Absender-Id der rufenden  Applikation
            hat.  0 bedeutet  Instanzhandle,  d.h. die Anwort  wird an  die 
            Instanz der Applikation geschickt und kann mit GetMessage gele-
            sen  werden.  1 bedeutet  Windowhandle,  d.h. die Antwort  wird
            direkt  an ein Fenster  der rufenden  Applikation geschickt und
            kann in der CallBack dieses Fensters ausgewertet werden.
instance  : Das Instanzhandle des Absenders (bzw. Empfaengers der Antwort)
window    : Das Windowhandle
type      : Der Datentyp der uebergebenen Zahl
radix     : Das Zahlensystem
value     : Die Zeichenkette, die die Zahl enthaelt.

===========================================================================
