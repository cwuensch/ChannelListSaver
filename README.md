# ChannelListSaver
Ein TAP zum Sichern, Exportieren und Wiederherstellen der Kanal- und Favoritenlisten für Topfield TMS Receiver.

Wer kennt das nicht? - Eine neue Firmware prangt auf der Topfield-Homepage und verspricht tolle neue Features, und dann die Enttäuschung: "Werksreset erforderlich" :-(
Und schwupps ist die ganze schöne Kanalliste, mit Favoriten und Co. für immer verschwunden...

Abhilfe schafft bisher das TAP RoboChannel, welches nach einem neuen Suchlauf in der Lage ist, die vorherige Reihenfolge der Sender wiederherzustellen.

Als Alternative steht nun aber auch der ChannelListSaver zur Verfügung mit einem etwas anderen Konzept:
  * Es werden die Satelliten, Transponder, TV- und Radio-Sender und die Favoritenlisten vollständig gesichert.
  * Diese können jederzeit zurückgespielt werden, OHNE dass ein Sendersuchlauf durchgeführt werden muss. 
  * Die Sicherung ist firmware- und modellübergreifend.
    (Natürlich kann eine Sat-Senderliste aber nicht in einen Kabel-Topf eingespielt werden!) 
  * Im- und Export erfolgen innerhalb von < 1 Sekunde. 

## Installation
  Am einfachsten erfolgt die Installation über TAPtoDate, wobei alle benötigten Dateien aufgespielt werden.
  Alternativ wird das TAP in den Ordner 'ProgramFiles' kopiert, und die zugehörige 'ChannelListSaver.lng' in den Ordner 'ProgramFiles/Settings/ChannelListSaver' platziert.
  Erforderlich ist zudem eine aktuelle Version der 'FirmwareTMS.dat'.

## Speicherung
  ChannelListSaver speichert die Settings in drei verschiedenen Formaten:
   a.) als Binärdatei 'Channels.dat' (ein bitweises Abbild der relevanten Speicherblöcke im Receiver-Flash)
   b.) als Textdatei 'Channels.txt' (die Senderlisten in menschenlesbarer Textform)
   c.) die 'Settings.std' (eine Komplettsicherung der Settings mittels der firmware-internen Export-Funktion, NICHT übertragbar!)
  Jedes der drei Formate kann durch ChannelListSaver auch wieder importiert werden.

## Bedienung
  * Beim erstmaligen Start von ChannelListSaver wird ein Export in alle drei Formate durchgeführt, und im Ordner 'ProgramFiles/Settings/ChannelListSaver' abgelegt.
  * Wird das TAP erneut gestartet, wenn bereits eine Export-Datei im genannten Ordner vorliegt, dann erscheint ein Dialog, in dem man auswählen kann zwischen dem Import der gefundenen Speicherung und dem Anlegen einer neuen Speicherung (ersetzt die alte).
  * Sollte beim Import nur eine der drei Dateien 'Channels.dat', 'Channels.txt' oder 'Settings.std' im entsprechenden Ordner vorliegen, so wird diese automatisch gewählt. Falls mehrere Sicherungen gleichzeitig vorhanden sind, so wird die Priorität durch den Parameter 'ImportFormat' bestimmt.
  * Vor dem Import von Settings wird grundsätzlich automatisch ein Backup sämtlicher Einstellungen mittels der Firmware-internen Export-Funktion durchgeführt ('Settings_vor.std'), welches im Notfall wiederhergestellt werden kann. Datenverlust und Fehlkonfiguration wird dadurch nahezu ausgeschlossen.

## Mögliche Probleme
  Das Überschreiben der Satelliten-Liste kann u.U. zu kleinen Problemen führen. Insbesondere dann, wenn eine Sat-Liste von einem anderen Modell importiert wird.
  Ggf. muss dann die (vor dem Import automatisch erzeugte) Backup-Datei 'Settings_vor.std' eingespielt werden, um die vorherigen Einstellungen vollständig wiederherzustellen.
  Das Überschreiben der Sat-Liste kann durch den Parameter 'OverwriteSatellites' in der INI-Datei beeinflusst werden:
   [0] Hierbei wird die Sat-Liste keinesfalls überschrieben. Dieser Modus bietet die maximale Sicherheit beim Einspielen, bei Abweichung der Sat-Anzahl wird der Import jedoch abgebrochen. (Beim Text-Import erfolgt der Abbruch erst NACH dem Reset der Daten)
   [1] Die Sat-Liste wird auf Übereinstimmung der Namen überprüft. Stimmen diese überein, so wird die Liste (mit sämtlichen Einstellungen) nicht verändert. Stimmt ein Sat-Name nicht überein, so wird die Sat-Liste überschrieben.
   [2] Die Sat-Liste wird grundsätzlich immer überschrieben (nicht empfehlenswert)

## Text-Format
  Die exportierte Textdatei 'Channels.txt' kann und darf mit dem PC editiert werden. Hierbei sind einige Punkte zu beachten:
   - Die Nummerierung innerhalb der Listen dient nur der Übersicht. Diese wird beim Einspielen NICHT berücksichtigt. Die Listenfolge ergibt sich allein aus der Reihenfolge der Zeilen.
   - Insbesondere das Feld 'SatIndex' in Transpondern und Sendern bezieht nicht NICHT auf die Nummerierung, sondern auf die Reihenfolge der Satelliten-Liste.
   - Die Transponder-Anzahl (NrTps) in der Satelliten-Liste wird beim Einspielen automatisch neu berechnet.
   - Die Leerzeichen zwischen den Spalten dienen nur der besseren Lesbarkeit. Diese können beliebig verringert werden.
   - Die Text-Einträge basieren auf konstanten Strings. Es dürfen nur bekannte Konstanten verwendet werden.
   - Die Codierung der Datei sollte nicht geändert werden (UTF8 vs. ISO).
   - Bearbeitung der Datei ist mit MS Excel möglich. Beim Import ist "Getrennt" durch "Semikolon" auszuwählen. Beim Abspeichern muss als Dateiformat "CSV (Trennzeichen getrennt)" ausgewählt werden.
   - Nach der Bearbeitung MUSS das Feld FileSize angepasst werden. Hier ist die exakte Dateigröße (in Bytes) einzutragen. Ansonsten wird der Import verweigert.

## Warnungen
  * Es werden NUR TMS-Modelle (z.B. SRP-2401CI+, CRP-2401, ...) unterstützt, KEINE Smart-Modelle!!!
  * Rückspielen ist nur innerhalb derselben Geräteklasse (Satellit / Kabel / Terrestrisch) möglich!
  * Nach dem Import sollten die Timer kontrolliert werden.

## Known Bugs
  - Nach einem Zurücksetzen der Service-Liste (über das Topf-Menü) und anschließendem Import durch dieses TAP werden die Favoriten-Listen manchmal vom Topf wieder überschrieben. Ein nochmaliger Import behebt das Problem.

## INI-Datei
  ImportFormat:		Beim Import bevorzugtes Datenformat: [0] Binärdatei, [1] Textdatei, [2] System-Backup (Settings.std)
  OverwriteSatellites:	Überschreiben der Sat-Liste: [0] niemals, [1] wenn abweichend, [2] immer
  
[seit v0.7]
  RestoreNameLock:	Beim Import das Flag "Sperre gegen Namensänderung" wiederherstellen. [0,1]
