# smallFire
Kleine Firewall auf Basis von inet_diag<br>

smallFire zeigt uns alle ausgehenden TCP Verbindungen und das<br>
fortlaufend als Monitor auf der Shell.<br><br>

Nach dem Compilieren startet man am besten das Script erst mal<br>
ohne sudo. Damit werden alle benötigten Dateien automatisch<br> 
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
Die einzugebende Nummer findet man übrigens immer am Anfang der<br> 
jeweiligen Auflistung.<br><br>

Es werden keine 0.0.0.0 TCP Verbindungen angezeigt diese werden<br>
raus gefiltert.<br><br>

Wenn man eine Verbindung stoppt wird sie nur solange geblockt bis<br>
der Rechner neu gestartet wird. Falls man die Verbindung wieder<br>
zulassen möchte prüft die Regeln mit sudo iptables -L -v und dann<br>
sudo iptables -D OUTPUT -d "destination" -p tcp -j DROP oder sudo<br>
iptables -D OUTPUT 1 für das Löschen der ersten Regel in OUTPUT<br> 
oder sudo iptables -F für das Löschen aller Regeln.<br>
Wenn man diese Regel dauerhaft speichern möchte muss man<br>
iptables-save, iptables-restore und wenn man es wünscht<br>
iptables-persisitent verwenden.<br><br>

Da in Ubuntu kein /a für Sound geht wurde das Lizenzfreie<br>
pferd.wav verwendet. Kann man natürlich mit dem selben Namen<br>
nach belieben ersetzen. Das Programm aplay sollte zum Abspielen<br> 
installiert sein.<br><br>


Compilieren:<br>
sudo apt install build-essential<br>
sudo apt install libncurses5-dev libncursesw5-dev<br>
gcc -o smallFire smallFire.c -lncurses -Wall<br><br>


Ich wünsche euch viel Spaß mit dem Tool
