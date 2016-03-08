#include <File.au3>
#include <GuiConstants.au3>
#include <GuiListView.au3>
#include <StringConstants.au3>

Global $iVisualizerPID = 0

Opt("GUIOnEventMode",1)
Global $gMainGUI = GUICreate("Visualizer Loader",400,700)
	GUISetOnEvent(-3,guiexit)
Global $gSearchInput = GUICtrlCreateInput("",2,2,348)
Global $gSearchGo = GUICtrlCreateButton("Search",350,2,48,20)
	GUICtrlSetOnEvent($gSearchGo,Search)
GLobal $gListView = GUICtrlCreateListView("",2,27,396,671)
Global $hListView = GUICtrlGetHandle($gListView)
_GUICtrlListView_AddColumn($gListView, "MapID", 55)
_GUICtrlListView_AddColumn($gListView, "Name", 125)
_GUICtrlListView_AddColumn($gListView, "File ID", 62)
_GUICtrlListView_AddColumn($gListView, "Spawn X", 75)
_GUICtrlListView_AddColumn($gListView, "Spawn Y", 75)

GUIRegisterMsg($WM_NOTIFY,wm_notify)

Global $aListings

_FileReadToArray(@WorkingDir&"\mapinfo.csv",$aListings,0,',')

ConsoleWrite('@@ Debug(' & @ScriptLineNumber & ') : $aListings = ' & $aListings & @CRLF & '>Error code: ' & @error & @CRLF) ;### Debug Console

_GUICtrlListView_SetItemCount($gListView, UBound($aListings))

_GUICtrlListView_AddArray($gListView,$aListings)

GUISetState(@SW_SHOW)

While 1
	If $iVisualizerPID <> 0 Then
		While ProcessExists($iVisualizerPID)
			Sleep(10)
		WEnd

		ParseStdOut($iVisualizerPID)

		$iVisualizerPID = 0
	EndIf

	Sleep(100)
WEnd

Func ParseStdOut($nPid)

	Local $sStdOut = StdoutRead($nPid)
	ConsoleWrite('@@ Debug(' & @ScriptLineNumber & ') : $sStdOut = ' & $sStdOut & @CRLF & '>Error code: ' & @error & @CRLF) ;### Debug Console

	Local $nCleared = StringInStr($sStdOut, "=", $STR_NOCASESENSE, -1)

	If $nCleared Then $sStdOut = StringMid($sStdOut, $nCleared)

	Local $aPairs = StringRegExp($sStdOut, "(?<=\().*?(?=\))", $STR_REGEXPARRAYGLOBALMATCH)
	ConsoleWrite('@@ Debug(' & @ScriptLineNumber & ') : $aPairs = ' & $aPairs & @CRLF & '>Error code: ' & @error & @CRLF) ;### Debug Console

	Local $nElements = UBound($aPairs)
	ConsoleWrite('@@ Debug(' & @ScriptLineNumber & ') : $nElements = ' & $nElements & @CRLF & '>Error code: ' & @error & @CRLF) ;### Debug Console

	If Not $nElements Then Return

	Local $sOut = StringFormat("Global Const $WAYPOINTS[%u][2] = [[%u], _\r\n\t\t", $nElements + 1, $nElements)

	Local $aPoint

	$nElements -= 1

	For $i = 0 To $nElements
		$aPoint = StringSplit($aPairs[$i], ",")

		$aPoint[1] = Number($aPoint[1])
		$aPoint[2] = Number($aPoint[2])

		$sOut &= StringFormat("[%s%e, %s%e]", _
				$aPoint[1] >= 0 ? "+" : ""  , _
				$aPoint[1]                  , _
				$aPoint[2] >= 0 ? "+" : ""  , _
				$aPoint[2]                    )

		$sOut &= $i <> $nElements ? ", _" & @CRLF & @TAB & @TAB : "  ]"
	Next

	Opt('GUIOnEventMode',0)

	Local $gui = GUICreate('coords',500,600)
	Local $edit = GUICtrlCreateEdit($sOut,5,5,490,590)
	GUISetState(@SW_SHOW,$gui)
	While GUIGetMsg() <> -3
	WEnd

	GUIDelete()
	Opt('GUIOnEventMode',1)

EndFunc

Func Search()
	Local $sQuery = GUICtrlRead($gSearchInput)

	If $sQuery == "" Then
		_GUICtrlListView_DeleteAllItems($hListView)
		_GUICtrlListView_AddArray($gListView,$aListings)
		Return
	EndIf

	Local $aSearchListings[UBound($aListings)][6]
	Local $iIndexer = 0

	For $i = 0 To UBound($aListings) - 1
		If StringInStr($aListings[$i][1],$sQuery) Then

			For $j = 0 To 5
				$aSearchListings[$iIndexer][$j] = $aListings[$i][$j]
			Next
			$iIndexer += 1
		EndIf
	Next
	ReDim $aSearchListings[$iIndexer][6]
	 _GUICtrlListView_DeleteAllItems($hListView)
	 _GUICtrlListView_AddArray($gListView,$aSearchListings)
EndFunc

Func guiexit()
	Exit
EndFunc

Func wm_notify($hwnd,$msg,$wparam,$lparam)
	#forceref $hwnd, $msg, $wparam

	If Not $iVisualizerPID Then
		Local $tNMHDR = DllStructCreate($tagNMHDR,$lparam)
		If Hwnd(DllStructGetData($tNMHDR,1)) = $hListView Then
			If DllStructGetData($tNMHDR,3) = $NM_DBLCLK Then
				Local $vSelectedIndices = _GUICtrlListView_GetSelectedIndices($hListView,True)
				Local $cmdlinearg = '.\Visualization.exe ' & _GUICtrlListView_GetItemText($hListView,$vSelectedIndices[1],2)
				ConsoleWrite('@@ Debug(' & @ScriptLineNumber & ') : $cmdlinearg = ' & $cmdlinearg & @CRLF & '>Error code: ' & @error & @CRLF) ;### Debug Console
				$iVisualizerPID = Run($cmdlinearg, @ScriptDir, @SW_SHOW, $STDOUT_CHILD)
			EndIf
		EndIf
	EndIf

    Return $GUI_RUNDEFMSG
EndFunc
