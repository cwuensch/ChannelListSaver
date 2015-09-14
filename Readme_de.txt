ChannelListSaver
================

Wer kennt das nicht? - Eine neue Firmware prangt auf der Topfield-Homepage und verspricht tolle neue Features, und dann die Entt�uschung: "Werksreset erforderlich" :-(
Und schwupps ist die ganze sch�ne Kanalliste, mit Favoriten und Co. f�r immer verschwunden...

Abhilfe schafft bisher das TAP RoboChannel, welches nach einem neuen Suchlauf in der Lage ist, die vorherige Reihenfolge der Sender wiederherzustellen.

Als Alternative steht nun aber auch der ChannelListSaver zur Verf�gung mit einem etwas anderen Konzept:
  * Es werden die Satelliten, Transponder, TV- und Radio-Sender und die Favoritenlisten vollst�ndig gesichert.
  * Diese k�nnen jederzeit zur�ckgespielt werden, OHNE dass ein Sendersuchlauf durchgef�hrt werden muss. 
  * Die Sicherung ist firmware- und modell�bergreifend.
    (Nat�rlich kann eine Sat-Senderliste aber nicht in einen Kabel-Topf eingespielt werden!) 
  * Im- und Export erfolgen innerhalb von < 1 Sekunde. 

Installation:
  Am einfachsten erfolgt die Installation �ber TAPtoDate, wobei alle ben�tigten Dateien aufgespielt werden.
  Alternativ wird das TAP in den Ordner 'ProgramFiles' kopiert, und die zugeh�rige 'ChannelListSaver.lng' in den Ordner 'ProgramFiles/Settings/ChannelListSaver' platziert.
  Erforderlich ist zudem eine aktuelle Version der 'FirmwareTMS.dat'.

Speicherung:
  ChannelListSaver speichert die Settings in drei verschiedenen Formaten:
   a.) als Bin�rdatei 'Channels.dat' (ein bitweises Abbild der relevanten Speicherbl�cke im Receiver-Flash)
   b.) als Textdatei 'Channels.txt' (die Senderlisten in menschenlesbarer Textform)
   c.) die 'Settings.std' (eine Komplettsicherung der Settings mittels der firmware-internen Export-Funktion, NICHT �bertragbar!)
  Jedes der drei Formate kann durch ChannelListSaver auch wieder importiert werden.

Bedienung:
  * Beim erstmaligen Start von ChannelListSaver wird ein Export in alle drei Formate durchgef�hrt, und im Ordner 'ProgramFiles/Settings/ChannelListSaver' abgelegt.
  * Wird das TAP erneut gestartet, wenn bereits eine Export-Datei im genannten Ordner vorliegt, dann erscheint ein Dialog, in dem man ausw�hlen kann zwischen dem Import der gefundenen Speicherung und dem Anlegen einer neuen Speicherung (ersetzt die alte).
  * Sollte beim Import nur eine der drei Dateien 'Channels.dat', 'Channels.txt' oder 'Settings.std' im entsprechenden Ordner vorliegen, so wird diese automatisch gew�hlt. Falls mehrere Sicherungen gleichzeitig vorhanden sind, so wird die Priorit�t durch den Parameter 'ImportFormat' bestimmt.
  * Vor dem Import von Settings wird grunds�tzlich automatisch ein Backup s�mtlicher Einstellungen mittels der Firmware-internen Export-Funktion durchgef�hrt ('Settings_vor.std'), welches im Notfall wiederhergestellt werden kann. Datenverlust und Fehlkonfiguration wird dadurch nahezu ausgeschlossen.

M�gliche Probleme:
  Das �berschreiben der Satelliten-Liste kann u.U. zu kleinen Problemen f�hren. Insbesondere dann, wenn eine Sat-Liste von einem anderen Modell importiert wird.
  Ggf. muss dann die (vor dem Import automatisch erzeugte) Backup-Datei 'Settings_vor.std' eingespielt werden, um die vorherigen Einstellungen vollst�ndig wiederherzustellen.
  Das �berschreiben der Sat-Liste kann durch den Parameter 'OverwriteSatellites' in der INI-Datei beeinflusst werden:
   [0] Hierbei wird die Sat-Liste keinesfalls �berschrieben. Dieser Modus bietet die maximale Sicherheit beim Einspielen, bei Abweichung der Sat-Anzahl wird der Import jedoch abgebrochen. (Beim Text-Import erfolgt der Abbruch erst NACH dem Reset der Daten)
   [1] Die Sat-Liste wird auf �bereinstimmung der Namen �berpr�ft. Stimmen diese �berein, so wird die Liste (mit s�mtlichen Einstellungen) nicht ver�ndert. Stimmt ein Sat-Name nicht �berein, so wird die Sat-Liste �berschrieben.
   [2] Die Sat-Liste wird grunds�tzlich immer �berschrieben (nicht empfehlenswert)

Text-Format:
  Die exportierte Textdatei 'Channels.txt' kann und darf mit dem PC editiert werden. Hierbei sind einige Punkte zu beachten:
   - Die Nummerierung innerhalb der Listen dient nur der �bersicht. Diese wird beim Einspielen NICHT ber�cksichtigt. Die Listenfolge ergibt sich allein aus der Reihenfolge der Zeilen.
   - Insbesondere das Feld 'SatIndex' in Transpondern und Sendern bezieht nicht NICHT auf die Nummerierung, sondern auf die Reihenfolge der Satelliten-Liste.
   - Die Transponder-Anzahl (NrTps) in der Satelliten-Liste wird beim Einspielen automatisch neu berechnet.
   - Die Leerzeichen zwischen den Spalten dienen nur der besseren Lesbarkeit. Diese k�nnen beliebig verringert werden.
   - Die Text-Eintr�ge basieren auf konstanten Strings. Es d�rfen nur bekannte Konstanten verwendet werden.
   - Die Codierung der Datei sollte nicht ge�ndert werden (UTF8 vs. ISO).
   - Bearbeitung der Datei ist mit MS Excel m�glich. Beim Import ist "Getrennt" durch "Semikolon" auszuw�hlen. Beim Abspeichern muss als Dateiformat "CSV (Trennzeichen getrennt)" ausgew�hlt werden.
   - Nach der Bearbeitung MUSS das Feld FileSize angepasst werden. Hier ist die exakte Dateigr��e (in Bytes) einzutragen. Ansonsten wird der Import verweigert.

Warnungen:
  * Es werden NUR TMS-Modelle (z.B. SRP-2401CI+, CRP-2401, ...) unterst�tzt, KEINE Smart-Modelle!!!
  * R�ckspielen ist nur innerhalb derselben Ger�teklasse (Satellit / Kabel / Terrestrisch) m�glich!
  * Beim Import der Einstellungen werden s�mtliche Timer gel�scht.

Known Bugs:
  - Nach einem Zur�cksetzen der Service-Liste (�ber das Topf-Men�) und anschlie�endem Import durch dieses TAP werden die Favoriten-Listen manchmal vom Topf wieder �berschrieben. Ein nochmaliger Import behebt das Problem.

INI-Datei:
  ImportFormat:		Zum Import bevorzugtes Datenformat: [0] Bin�rdatei, [1] Textdatei, [2] System-Backup (Settings.std)
  OverwriteSatellites:	�berschreiben der Sat-Liste: [0] niemals, [1] wenn abweichend, [2] immer
[seit v0.7]
  RestoreNameLock:	Beim Import das Flag "Sperre gegen Namens�nderung" wiederherstellen. [0,1]
