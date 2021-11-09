; The name of the installer
Name "Open Parsec 0.3"

; The file to write
OutFile "openparsec-0.3.exe"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

; Build Unicode installer
Unicode True

; The default installation directory
InstallDir $PROGRAMFILES\OpenParsec
;--------------------------------

; Pages

Page directory
Page instfiles

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important

  
  ; Put file there
  SetOutPath "$INSTDIR\cons\"
  File /a /r "cons\"
  SetOutPath "$INSTDIR\Images\"
  File /a /r "Images\"
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  File init.con
  File libFLAC-8.dll
  File libmodplug-1.dll
  File libmpg123-0.dll
  File libogg-0.dll
  File libopus-0.dll
  File libopusfile-0.dll
  File libvorbis-0.dll
  File libvorbisfile-3.dll
  File LICENSE.artwork_sound
  File openparsec.ico
  File parsecrc.con
  File pscdata0.dat
  File pscdata1.dat
  File pscdata2.dat
  File pscdata3.dat
  File README.md
  File SDL2.dll
  File SDL2_mixer.dll
  File Parsec.exe
  
  CreateDirectory "$SMPROGRAMS\OpenParsec"
  CreateShortcut "$SMPROGRAMS\OpenParsec\Open Parsec.lnk" "$INSTDIR\Parsec.exe" "" "$INSTDIR\openparsec.ico"
SectionEnd
