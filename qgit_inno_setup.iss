;
; QGit installation script for Inno Setup compiler
;
; QGit should be compiled with MSVC 2013 and dynamically linked to Qt 4.8.0 or Qt 5.0.3
;

[Setup]
AppName=QGit
AppVerName=QGit version 2.6
DefaultDirName={pf}\QGit
DefaultGroupName=QGit
UninstallDisplayIcon={app}\qgit.exe
Compression=lzma
SolidCompression=yes
LicenseFile=${CMAKE_CURRENT_SOURCE_DIR}\COPYING.rtf
SetupIconFile=${CMAKE_CURRENT_SOURCE_DIR}\src\resources\qgit.ico
OutputDir=Release
OutputBaseFilename=qgit-2.6_win

[Files]
; QGit binaries
Source: "Release\qgit.exe"; DestDir: "{app}"
; MSVC runtime libraries
Source: "Release\msvcp120.dll"; DestDir: "{app}"
Source: "Release\msvcr120.dll"; DestDir: "{app}"
; Qt5 libraries
Source: "Release\Qt5Core.dll"; DestDir: "{app}"
Source: "Release\Qt5Gui.dll"; DestDir: "{app}"
Source: "Release\Qt5Widgets.dll"; DestDir: "{app}"
; Qt5 plugins
Source: "Release\platforms\qwindows.dll";   DestDir: "{app}\platforms"
; Documentation
Source: "${CMAKE_CURRENT_SOURCE_DIR}\README_WIN.txt"; DestDir: "{app}"; Flags: isreadme
Source: "${CMAKE_CURRENT_SOURCE_DIR}\COPYING.rtf"; DestDir: "{app}";

[Tasks]
Name: desktopicon; Description: "Create a &desktop icon";
Name: winexplorer; Description: "Add ""QGit Here"" in Windows Explorer context menu";

[Registry]
Root: HKCU; Subkey: "Software\qgit"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\qgit\qgit4"; ValueType: string; ValueName: "msysgit_exec_dir"; ValueData: "{code:GetMSysGitExecDir}";

; Windows Explorer integration
Root: HKCU; Subkey: "SOFTWARE\Classes\Directory\shell\qgit"; Flags: uninsdeletekey; Tasks: winexplorer
Root: HKCU; Subkey: "SOFTWARE\Classes\Directory\shell\qgit"; ValueType: string; ValueName: ""; ValueData: "&QGit Here"; Tasks: winexplorer
Root: HKCU; Subkey: "SOFTWARE\Classes\Directory\shell\qgit\command"; ValueType: string; ValueName: ""; ValueData: "{app}\qgit.exe"; Tasks: winexplorer

[Dirs]
Name: {code:GetMSysGitExecDir}; Flags: uninsneveruninstall

[Icons]
Name: "{group}\QGit"; Filename: "{app}\qgit.exe"; WorkingDir: "%USERPROFILE%";
Name: "{group}\Uninstall QGit"; Filename: "{uninstallexe}"
Name: "{commondesktop}\QGit"; Filename: "{app}\qgit.exe"; WorkingDir: "%USERPROFILE%"; Tasks: desktopicon

[Code]
var
  MSysGitDirPage: TInputDirWizardPage;

procedure InitializeWizard;
var
  Key, Val: String;

begin
  // Create msysgit directory find page
  MSysGitDirPage := CreateInputDirPage(wpSelectProgramGroup,
      'Select MSYSGIT Location', 'Where is MSYSGIT directory located?',
      'Select where MSYSGIT directory is located, then click Next.',
      False, '');

  // Add item (with an empty caption)
  MSysGitDirPage.Add('');

  // Set initial value
  Key := 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Git_is1';

  if RegQueryStringValue(HKEY_LOCAL_MACHINE, Key, 'InstallLocation', Val) then begin
    MSysGitDirPage.Values[0] := Val;
  end else
    MSysGitDirPage.Values[0] := ExpandConstant('{pf}\Git');

end;

function NextButtonClick(CurPageID: Integer): Boolean;
var
  BaseDir: String;

begin
  // Validate pages before allowing the user to proceed
  if CurPageID = MSysGitDirPage.ID then begin

      BaseDir := MSysGitDirPage.Values[0];

      if FileExists(ExpandFileName(BaseDir + '\bin\git.exe')) then begin
        Result := True;

      end else if FileExists(ExpandFileName(BaseDir + '\..\bin\git.exe')) then begin // sub dir selected
        MSysGitDirPage.Values[0] := ExpandFileName(BaseDir + '\..');
        Result := True;

      end else begin
        MsgBox('Directory ''' + BaseDir + ''' does not seem the msysgit one, retry', mbError, MB_OK);
        Result := False;
      end;

  end else
    Result := True;
end;

function GetMSysGitExecDir(Param: String): String;
begin
  Result := MSysGitDirPage.Values[0] + '\bin'; // already validated
end;
