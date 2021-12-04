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
  File /a /r "..\..\openparsec-assets\cons\"
  SetOutPath "$INSTDIR\Images\"
  File /a /r "..\..\openparsec-assets\Images\"
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  File ..\..\openparsec-assets\init.con
  File lib\x64\libFLAC-8.dll
  File lib\x64\libmodplug-1.dll
  File lib\x64\libmpg123-0.dll
  File lib\x64\libogg-0.dll
  File lib\x64\libopus-0.dll
  File lib\x64\libopusfile-0.dll
  File lib\x64\libvorbis-0.dll
  File lib\x64\libvorbisfile-3.dll
  File ..\..\openparsec-assets\LICENSE.artwork_sound
  File ..\..\openparsec-assets\openparsec.ico
  File ..\..\openparsec-assets\parsecrc.con
  File ..\..\openparsec-assets\pscdata0.dat
  File ..\..\openparsec-assets\pscdata1.dat
  File ..\..\openparsec-assets\pscdata2.dat
  File ..\..\openparsec-assets\pscdata3.dat
  File ..\..\openparsec-assets\README.md
  File lib\x64\SDL2.dll
  File lib\x64\SDL2_mixer.dll
  File x64\Release\Parsec.exe
  
  CreateDirectory "$SMPROGRAMS\OpenParsec"
  CreateShortcut "$SMPROGRAMS\OpenParsec\Open Parsec.lnk" "$INSTDIR\Parsec.exe" "" "$INSTDIR\openparsec.ico"
SectionEnd
