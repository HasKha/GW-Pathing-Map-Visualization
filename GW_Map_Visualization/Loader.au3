#include <File.au3>
#include <GuiConstants.au3>
#include <GuiListView.au3>
#include <StringConstants.au3>

Global $iVisualizerPID = 0

Opt("GUIOnEventMode",1)
Global $gMainGUI = GUICreate("Visualizer Loader",400,725)
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

GUICtrlCreateLabel("OS Version:", 2, 702, 100, 20)
Global $gRadiox86 = GUICtrlCreateRadio("x86", 102, 702, 50, 20)
Global $gRadiox64 = GUICtrlCreateRadio("x64", 152, 702, 50, 20)
GUICtrlSetState($gRadiox64, $GUI_CHECKED)

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

		$iVisualizerPID = 0
	EndIf

	Sleep(100)
WEnd


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

Func OpenMap($mapid)
	If Not $iVisualizerPID Then
		Local $bits = "x64"
		If GUICtrlRead($gRadiox86) == $GUI_CHECKED Then $bits = "x86"
		Local $cmdlinearg = $bits & '\Visualization.exe ' & $mapid
		ConsoleWrite('@@ Debug(' & @ScriptLineNumber & ') : $cmdlinearg = ' & $cmdlinearg & @CRLF & '>Error code: ' & @error & @CRLF) ;### Debug Console
		$iVisualizerPID = Run($cmdlinearg, $bits, @SW_SHOW, $STDOUT_CHILD)
	EndIf
EndFunc

Func wm_notify($hwnd,$msg,$wparam,$lparam)
	#forceref $hwnd, $msg, $wparam

	Local $tNMHDR = DllStructCreate($tagNMHDR,$lparam)
	If Hwnd(DllStructGetData($tNMHDR,1)) = $hListView Then
		If DllStructGetData($tNMHDR,3) = $NM_DBLCLK Then
			Local $vSelectedIndices = _GUICtrlListView_GetSelectedIndices($hListView,True)
			Local $mapid = _GUICtrlListView_GetItemText($hListView,$vSelectedIndices[1],2)
			OpenMap($mapid)
		EndIf
	EndIf

    Return $GUI_RUNDEFMSG
EndFunc

