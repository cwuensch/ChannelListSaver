ChannelListSaver
================

[Translated automatically by Google]

Does this sound familiar? - A new firmware emblazoned on the Topfield website and promises exciting new features, and then the disappointment: "Factory reset is required":-(
And suddenly the whole beautiful channel list, with favorites and Co. is gone forever ...

Remedy so far the TAP RoboChannel, which is for a new search in a position to restore the previous order of the channels.

As an alternative, but is now also the Channel List Saver are available with a slightly different concept:
  * It will be fully backed the satellite, transponder, TV and radio stations and the favorite lists.
  * This can always be restored WITHOUT having a tuning needs to be done.
  * The fuse is firmware- and across all models.
    (Of course, a satellite channel list can not be imported into a cable pot!)
  * Import and export occur within <1 second.

Installation:
  The easiest way to install through TAPtoDate, all required files are uploaded.
  Alternatively, the TAP is copied into the folder 'Program Files' and the associated 'ChannelListSaver.lng' placed in the folder 'Program Files / Settings / Channel List Saver'.
  Is also needed, a recent version of 'FirmwareTMS.dat'.

Storage:
  Channel List Saver stores the settings in three different formats:
   a.) as a binary file 'channels.dat' (a bitwise copy of the relevant blocks in the receiver-Flash)
   b.) as a text file 'Channels.txt' (channel lists in human-readable text form)
   c.) the 'Settings.std' (a complete backup of the settings using the firmware internal export function, NOT transferable!)
  Each of the three formats can also be imported by Channel List Saver again.

Waiter:
  * When you first launch of Channel List Saver export is carried out in all three formats, and stored in the folder 'Program Files / Settings / Channel List Saver'.
  * If the TAP restarted if there is already a file in the export folder called, then a dialog where you can choose between importing and storing found creating a new storage appearing (replaces the old one).
  * If the import only one of the three files 'channels.dat', 'Channels.txt' or 'Settings.std' present in the appropriate folder, it will be automatically selected. If multiple backups are simultaneously present, the priority is determined by the parameter 'Import Format'.
  * Before you import settings a backup of all settings is always automatically using the internal firmware export function performed ('Settings_vor.std'), which can be restored in case of emergency. Data loss and misconfiguration is virtually ruled out.

Possible problems:
  Overriding the satellite list can u.U. lead to small problems. In particular, when a satellite list is imported from another model.
  If necessary, then has to be installed to fully restore the previous settings (automatically generated before importing) backup file 'Settings_vor.std'.
  Overriding the satellite list can be influenced by the parameter 'Overwrite Satellites' in the INI file:
   [0] This is never overwritten the satellite list. This mode provides maximum safety when playing in deviation of the satellite number of import will be interrupted. (In the text import the termination occurs AFTER the reset of the data)
   [1] The satellite list is checked for a match of the name. If they match, then the list (with all settings) does not change. Does not match a satellite name, then the satellite list will be overwritten.
   [2] The satellite list is always overwritten (not recommended)

Text format:
  The exported text file 'Channels.txt' can and may be edited with the PC. Here are some points to consider:
   - The numbering within the lists just provides an overview. This is taken into account when applying NOT. The list consequence arises solely from the order of the rows.
   - In particular, the field 'SatIndex' in transponders and channels does not refer to the numbering NOT, but the order of the satellite list.
   - The transponder number (nrtPS) in the satellite list is automatically recalculated when importing.
   - The spaces between the columns are used for convenience only. These can be arbitrarily reduced.
   - The text messages are based on constant strings. Only known constants are used.
   - The encoding of the file should not be changed (UTF8 vs. ISO).
   - Editing the file is possible using MS Excel. When importing is "Disconnected" by selecting "semicolon". When saving as a file format "CSV (Comma delimited)" must be selected.
   - After editing the file size field MUST be adjusted. Here is the exact file size must be entered (in bytes). Otherwise, the import will be denied.

Warnings:
  * There are ONLY TMS models (eg SRP-2401CI +, CRP-2401, ...) support, NO Smart models !!!
  * Reverse Play is only possible within the same device class (satellite / cable / terrestrial)!
  * After importing the settings, the timers should be inspected.

Known Bugs:
  - After a reset the service list (on the pot menu) and then import this by TAP favorites lists are sometimes overwritten from the pot. Column again import fixes the problem.

INI file:
  Import format:	[0] binary import preferred [1] prefer text file, prefer [2] system backup (Settings.std)
  Overwrite Satellites:	[0] never override satellite list, [1] overwrite if different, [2] always overwrite
[since v0.7]
  RestoreNameLock:	Set the "Flag against renaming" on restore. [0,1]
