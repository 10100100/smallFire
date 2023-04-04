# smallFire
Kleine Firewall auf Basis von inet_diag<br>

smallFire zeigt uns alle ausgehenden TCP Verbindungen und das<br>
fortlaufend als Monitor auf der Shell.<br><br>

Nach dem Compilieren startet man am besten das Scirpt erstmal<br>
ohne sudo. Damit werden alle benötigten Datein automatisch<br> 
angelegt. Nach dem Ausführen wird liste.txt gleich angelegt.<br>
Dann öffnet mal kurz ein Verbindung z.B. durch öffnen des <br>
Browsers damit wird die Datei verbindung.txt angelegt.<br>
Jetzt ist smallFire auch unter sudo ./smallFire einsatzbereit.<br>
Was man benötigt um alle Programme zu sehen und Verbindungen<br>
verbieten zu können.<br><br>

Dateien:<br>
liste.txt enthält die erlaubten Programme<br>
verbindung.txt enthält als Verbindungs log Datei alle nicht<br>
erlaubten Programme<br>
pferd.wav sound um zu merken das eine nicht erlaubte Verbindung<br> 
ausgeht<br><br>

Tasten Funktionen im Programm:<br>
q - für Beenden des Programms<br>
d - für DNS anzeigen<br>
s - für verbiete Verbindung mit Nummer<br>
e - für erlaube Programm mit Nummer<br>
l - für zeige erlaubte Programme<br>
r - für Lösche erlaubtes Programm mit Nummer<br> 
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;vorher am besten l drücken um die Nummer zusehen<br><br> 

Achtung:<br> 
Wenn man die Nummer angeben soll stoppt das Programm<br>
und wartet auf die Eingabe der Nummer. Wenn man die Nummer<br>
eingegeben hat drückt man danach Enter.<br>
Leider gibt es noch Probleme mit ncurses und getch die Nummer<br>
Abfrage korrekt anzuzeigen.<br>
Die einzugebene Nummer findet man übrigens immer am Anfang der<br> 
jeweiligen Auflistung.<br><br>
Es werden keine 0.0.0.0 TCP Verbindungen angezeigt diese werden<br>
rausgefiltert.<br><br>

Da in Ubuntu kein /a für Sound geht wurde das lizensfreie<br>
pferd.wav verwendet. Kann man natürlich mit dem selben Namen<br>
nach belieben ersetzen.<br><br>


Compilieren:<br>
gcc -o smallFire smallFire.c -lncurses -Wall<br><br>


Ich wünsche euch viel Spass mit dem Tool
