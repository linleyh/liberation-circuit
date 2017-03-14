; Created by rHermes (2017.03.14 12:36:40 UTC)
; 
; This was made so that I could use the ingame code editor in the very cool
; game called "liberation-circut"[1]. It has only been minimally tested so
; please fork and share your improvements!
;
; [1] - [https://github.com/linleyh/liberation-circuit]

#NoEnv  ; Recommended for performance and compatibility with future AutoHotkey releases.
; #Warn  ; Enable warnings to assist with detecting common errors.
SendMode Input  ; Recommended for new scripts due to its superior speed and reliability.
SetWorkingDir %A_ScriptDir%  ; Ensures a consistent starting directory.

; ^!r::Reload ; QoL keybinding. Uncomment if you want to have access to quick reloads.

RD := false

; This sends the keys neccecary.
KeyPipeLine(agr,shi,nor) {
	global RD
	if (RD) { 
		Send %agr% 
	} else if (GetKeyState("Shift")) {
		Send %shi%
	} else {
		Send %nor%
	}
}

#IfWinActive ahk_exe LibCirc.exe
; These two are responsible for toggeling RD on and off.
*RAlt:: RD := true
*RAlt Up:: RD := false

*2::KeyPipeLine("@", """", "2") 	; 2 aka @
*3::KeyPipeLine("£", "{#}", "3") 	; 3 aka £ - NP: The editor has no support for £ in the code 
*4::KeyPipeLine("$", "¤", "4") 		; 4 aka $ - NP: The editor has no support for ¤ in the code

*7::KeyPipeLine("{{}", "/", "7") 	; 7 aka {
*8::KeyPipeLine("[", "(", "8") 		; 8 aka [
*9::KeyPipeLine("]", ")", "9") 		; 9 aka ]
*0::KeyPipeLine("{}}", "=", "0") 	; 0 aka }

*\::KeyPipeLine("´", "``", "\")		; \ aka ´ - NP: The editor has no support for ´ or ` in the code
*¨::KeyPipeLine("~", "{^}", "¨")	; ¨ aka ~ - NP: The editor has no support for ¨ in the code
#IfWinActive