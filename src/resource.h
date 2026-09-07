// resource.h : resource identifier definitions for Hover!
//
// Reconstructed from the resources extracted out of hover.exe (see the
// res directory). The numeric values are ground truth from hover.exe; the
// symbolic names are reconstructed (the original names are unknown).

#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#define IDR_MAINFRAME			128	// menu, accelerator and icon

#define IDD_CUSTOMIZE			101	// "Customize Game" dialog
#define IDD_STARTAT			106	// "Start At" dialog
#define IDD_HIGHSCORES			108	// "High Scores" dialog
#define IDD_SOUNDOPTIONS		109	// "Sound Options" dialog
#define IDD_ENTERHIGHSCORE		112	// "Enter High Score" name entry dialog
#define IDD_PLAYERCONTROLS		113	// "Player Controls" dialog
#define IDD_QUICKHELP			115	// "Quick Help" startup dialog
#define IDD_LOADING			143	// "Please Wait" level loading progress dialog
#define IDB_SPLASH				145	// splash screen bitmap
#define IDD_SPLASH				146	// startup splash dialog
#define IDD_KEYBOARD			103	// "Keyboard Settings" dialog (opened from
									// IDD_PLAYERCONTROLS' "Set Keys..." button)
#define IDD_DEBUG_SETPOSITION	133	// debug "set player position" dialog used
									// by CBumperDoc::OnCmd34002; the template
									// was stripped from the shipped hover.exe,
									// so DoModal() always fails there

// Dialog templates referenced by the CHoverApp debug command handlers but
// no longer present in hover.exe's resources (the templates were stripped
// from the shipped build, so DoModal on them fails).
#define IDD_DBG_130				130	// opened by ID_CMD_34012
#define IDD_DBG_148				148	// opened by ID_CMD_34008
#define IDD_DBG_149				149	// opened by ID_CMD_34009
#define IDD_DBG_152				152	// opened by ID_CMD_34031

// Control IDs
#define IDC_QUICKHELP_SKIP		1008	// "Skip Quick Help at Startup" checkbox

// IDD_CUSTOMIZE (101) "Customize Game"
#define IDC_CUST_SPEEDDETAIL	1104	// speed/detail trackbar (0..5)
#define IDC_CUST_EASY			1000	// "Vehicle Controls" radio group
#define IDC_CUST_INTERMEDIATE	1001
#define IDC_CUST_DIFFICULT		1002
#define IDC_CUST_REARVIEW		1012	// "Show Rear &View" checkbox
#define IDC_CUST_SEQUENTIAL		30100	// "Maze Type" radio group
#define IDC_CUST_RANDOM			30101
#define IDC_CUST_ROTATEMAP		30102	// "Map Options" radio group
#define IDC_CUST_ROTATECRAFT	30103

// IDD_KEYBOARD (103) "Keyboard Settings"
#define IDC_KEY_FORWARD			1006	// "&Forward"
#define IDC_KEY_DEFAULT			1007	// "&Default" button
#define IDC_KEY_FWDLEFT			1008	// "F&orward && Left"
#define IDC_KEY_LEFT			1009	// "&Left"
#define IDC_KEY_REVRIGHT		1010	// "Rever&se && Right"
#define IDC_KEY_REVLEFT			1012	// "R&everse && Left"
#define IDC_KEY_FWDRIGHT		1013	// "For&ward && Right"
#define IDC_KEY_RIGHT			1014	// "&Right"
#define IDC_KEY_REVERSE			1015	// "Re&verse"

// IDD_STARTAT (106) "Start At"
#define IDC_STARTAT_CUSTOMIZE	3		// "&Customize..." button
#define IDC_STARTAT_MAZETYPE	1090	// "Random" static
#define IDC_STARTAT_CONTROLS	1091	// "Easy" static
#define IDC_STARTAT_LEVELSCORE	1092	// "Completed Level:" score
#define IDC_STARTAT_LEVEL		1011	// level edit control
#define IDC_STARTAT_SPIN		1105	// level spin button (buddy of 1011)
#define IDC_STARTAT_CAPTURED	1109	// "Each Captured Flag:" score
#define IDC_STARTAT_REMAINING	1110	// "Each Remaining Flag:" score
#define IDC_STARTAT_SCOREBOX	1114	// group boxes/labels (help ids only)
#define IDC_STARTAT_LBL_LEVEL	1115
#define IDC_STARTAT_LBL_CAPTURED	1116
#define IDC_STARTAT_LBL_REMAINING	1117
#define IDC_STARTAT_SETTINGSBOX	1094
#define IDC_STARTAT_LBL_CONTROLS	1095
#define IDC_STARTAT_LBL_MAZETYPE	1096

// IDD_SOUNDOPTIONS (109) "Sound Options"
#define IDC_SOUND_MUSIC			1008	// "&Music"
#define IDC_SOUND_GAMESOUNDS	1009	// "&Game Sounds"
#define IDC_SOUND_ENGINE		1010	// "&Engine"
#define IDC_SOUND_MIXING		1012	// "&Sound Effects Mixing"
#define IDC_SOUND_EFFECTSBOX	1093	// "Effects" group box (help id only)

// IDD_PLAYERCONTROLS (113) "Player Controls"
#define IDC_CTRL_SETKEYS		1004	// "&Set Keys..." button
#define IDC_CTRL_CALIBRATE		1005	// "&Calibrate..." button
#define IDC_CTRL_JOYSTICK		1008	// "&Joystick" checkbox
#define IDC_CTRL_KEYBOARD		1009	// "Keyboard" checkbox (always checked)
#define IDC_CTRL_JUMP			1016	// "J&ump" object key combo
#define IDC_CTRL_WALL			1039	// "&Wall" object key combo
#define IDC_CTRL_CLOAK			1040	// "C&loak" object key combo
#define IDC_CTRL_DEFAULT		1042	// "&Default" button
#define IDC_CTRL_MOVEMENTBOX	1098	// group boxes/labels (help ids only)
#define IDC_CTRL_OBJECTSBOX		1099
#define IDC_CTRL_LBL_WALL		1100
#define IDC_CTRL_LBL_CLOAK		1101
#define IDC_CTRL_LBL_JUMP		1102

#define IDC_SPLASH_FRAME		1038	// black frame around the splash image
#define IDC_SPLASH_DIVIDER		1112	// etched line at the bottom of the splash
#define IDC_HIGHSCORE_NAME		1017	// name edit control in IDD_ENTERHIGHSCORE
#define IDC_CLEARSCORES			30104	// "Clear Scores" button in IDD_HIGHSCORES
#define IDC_DEBUG_X				1000	// X edit control in IDD_DEBUG_SETPOSITION
#define IDC_DEBUG_Y				1001	// Y edit control in IDD_DEBUG_SETPOSITION
#define IDC_DEBUG_ANGLE			1002	// angle edit control in IDD_DEBUG_SETPOSITION
#define IDC_LOADING_FRAME		1000	// black frame the progress bar is drawn
									// into in IDD_LOADING

// String resources (values from the string tables in hover.exe)
#define IDS_MIN_CONFIG			0xEF17	// 61207: "minimum configuration of a 486/33 with 8 MB"
#define IDS_DUPLICATE_KEYS		0xEF18	// 61208: "One or more keys you have chosen has already been assigned..."
#define IDS_CLEARSCORES_CONFIRM	0xEF3B	// 61243: "Are you sure you want to clear the high score table?"
#define IDS_PRESS_F2			0xEF4F	// 61263: "Are  you sure you want to quit
					// the game in progress?" (the name is a
					// misnomer: "Press F2 to start" is 0xEF3F)
#define IDS_QUIT_IN_PROGRESS	0xEF3C	// 61244: "Are  you sure you want to quit
					// the game in progress?" (duplicate of 0xEF4F)
#define IDS_SKILL_EASY			0xEF41	// 61249: "Easy" (IDD_STARTAT settings summary)
#define IDS_SKILL_INTERMEDIATE	0xEF42	// 61250: "Intermediate"
#define IDS_SKILL_DIFFICULT		0xEF43	// 61251: "Difficult"
#define IDS_MAZE_SEQUENTIAL		0xEF44	// 61252: "Sequential"
#define IDS_MAZE_RANDOM			0xEF45	// 61253: "Random"
#define IDS_NOT_256_COLOR		0xEF46	// 61254: "not running a 256 color video driver"
#define IDS_16_COLOR			0xEF47	// 61255: "will not function with the current 16 color driver"
#define IDS_BAD_WINDOWS			0xEF48	// 61256: "only runs on Windows 95, Windows NT 3.51 or better"
#define IDS_DEFEATED			0xEF4A	// 61258: "You have been defeated!  The Drones
										//   have captured all of your flags."
#define IDS_READ_FILE_ERROR		0xEF4B	// 61259: "problem reading a file in Hover!"
#define IDS_MEMORY_ERROR		0xEF4D	// 61261: "error allocating memory in Hover!"
#define IDS_UNEXPECTED_EXCEPTION	0xEF4E	// 61262: "unexpected exception in Hover!"
#define IDS_NO_RESOURCES		0xEF55	// 61269: "not enough Windows resources to run Hover!"
#define IDS_ALREADY_RUNNING		0xEF56	// 61270: "The application Hover! is already running."
#define IDS_WHATS_THIS			0xEF49	// 61257: "&What's This?" (view context menu)
#define IDS_WAVE_BUSY			0xEF51	// 61265: "All wave devices that can play files in this format are in use..."

// Exception description strings
#define IDS_LEVEL_TITLE			0xEF63	// 61283: "Hover! - Level %d" (frame title)
#define IDS_EXC_NOT_SUPPORTED	0xEF64	// 61284: "unsupported feature"
#define IDS_EXC_ARCHIVE_NONE	0xEF65	// 61285: "no archive error"
#define IDS_EXC_ARCHIVE_GENERIC	0xEF66	// 61286: "unspecified archive error"
#define IDS_EXC_ARCHIVE_READONLY	0xEF67	// 61287: "write into archive opened for loading"
#define IDS_EXC_ARCHIVE_ENDOFFILE	0xEF68	// 61288: "reached the end of a file"
#define IDS_EXC_ARCHIVE_WRITEONLY	0xEF69	// 61289: "read from archive opened for storing"
#define IDS_EXC_ARCHIVE_BADINDEX	0xEF6A	// 61290: "invalid file format"
#define IDS_EXC_ARCHIVE_BADCLASS	0xEF6B	// 61291: "object of the wrong type"
#define IDS_EXC_ARCHIVE_BADSCHEMA	0xEF6C	// 61292: "different version of the class"
#define IDS_EXC_RESOURCE		0xEF6D	// 61293: "failure to load a Windows resource"
#define IDS_EXC_OLE				0xEF6E	// 61294: "failure in OLE processing"
#define IDS_EXC_DAO				0xEF6F	// 61295: "failures in data access processing"
#define IDS_EXC_USER			0xEF70	// 61296: "stop a user-initiated operation"

// Key display names looked up by Hover_sub_42DC50 (61209-61238)
#define IDS_KEY_BACKSPACE		0xEF19	// 61209: "Back Space"
#define IDS_KEY_TAB			0xEF1A	// 61210: "Tab"
#define IDS_KEY_ENTER			0xEF1B	// 61211: "Enter"
#define IDS_KEY_SHIFT			0xEF1C	// 61212: "Shift"
#define IDS_KEY_CONTROL			0xEF1D	// 61213: "Control"
#define IDS_KEY_SPACE			0xEF1F	// 61215: "Space"
#define IDS_KEY_PGUP			0xEF20	// 61216: "Pg Up"
#define IDS_KEY_PGDN			0xEF21	// 61217: "Pg Dn"
#define IDS_KEY_END			0xEF22	// 61218: "End"
#define IDS_KEY_HOME			0xEF23	// 61219: "Home"
#define IDS_KEY_LEFT			0xEF24	// 61220: "Left"
#define IDS_KEY_UP			0xEF25	// 61221: "Up"
#define IDS_KEY_RIGHT			0xEF26	// 61222: "Right"
#define IDS_KEY_DOWN			0xEF27	// 61223: "Down"
#define IDS_KEY_INSERT			0xEF28	// 61224: "Insert"
#define IDS_KEY_DELETE			0xEF29	// 61225: "Delete"
#define IDS_KEY_NUMPAD0			0xEF2A	// 61226: "Num Pad 0"
#define IDS_KEY_NUMPAD1			0xEF2B	// 61227: "Num Pad 1"
#define IDS_KEY_NUMPAD2			0xEF2C	// 61228: "Num Pad 2"
#define IDS_KEY_NUMPAD3			0xEF2D	// 61229: "Num Pad 3"
#define IDS_KEY_NUMPAD4			0xEF2E	// 61230: "Num Pad 4"
#define IDS_KEY_NUMPAD5			0xEF2F	// 61231: "Num Pad 5"
#define IDS_KEY_NUMPAD6			0xEF30	// 61232: "Num Pad 6"
#define IDS_KEY_NUMPAD7			0xEF31	// 61233: "Num Pad 7"
#define IDS_KEY_NUMPAD8			0xEF32	// 61234: "Num Pad 8"
#define IDS_KEY_NUMPAD9			0xEF33	// 61235: "Num Pad 9"
#define IDS_KEY_MULTIPLY		0xEF34	// 61236: "Multiply"
#define IDS_KEY_DECIMAL			0xEF35	// 61237: "Decimal"
#define IDS_KEY_DIVIDE			0xEF36	// 61238: "Divide"

// Default key bindings for the 11 in-game actions, loaded by
// Hover_sub_4232E0 into theApp.m_avkControls (61271-61281)
#define IDS_DEFKEY_A			0xEF57	// 61271: "A" -> controls[2]
#define IDS_DEFKEY_S			0xEF58	// 61272: "S" -> controls[1]
#define IDS_DEFKEY_D			0xEF59	// 61273: "D" -> controls[0]
#define IDS_DEFKEY_HOME			0xEF5A	// 61274: "Home" -> controls[7]
#define IDS_DEFKEY_UP			0xEF5B	// 61275: "Up" -> controls[3]
#define IDS_DEFKEY_PGUP			0xEF5C	// 61276: "Pg Up" -> controls[8]
#define IDS_DEFKEY_LEFT			0xEF5D	// 61277: "Left" -> controls[5]
#define IDS_DEFKEY_RIGHT		0xEF5E	// 61278: "Right" -> controls[6]
#define IDS_DEFKEY_END			0xEF5F	// 61279: "End" -> controls[9]
#define IDS_DEFKEY_DOWN			0xEF60	// 61280: "Down" -> controls[4]
#define IDS_DEFKEY_PGDN			0xEF61	// 61281: "Pg Dn" -> controls[10]


// Command IDs from the IDR_MAINFRAME menu, the accelerator table and the
// original message maps (see the string table in hover.rc for the prompts).
// The ID_CMD_34xxx commands are not reachable from the menu or accelerators
// (leftover debug/cheat commands); rename them once their handlers are known.
#define ID_DBG_WALL                     32768
#define ID_DBG_POD                      32769
#define ID_DBG_FLOOR                    32770
#define ID_DBG_CAR                      32771
#define ID_GAME_START                   32777
#define ID_GAME_STARTAT                 32778
#define ID_GAME_PAUSE                   32779
#define ID_OPTIONS_FULLSCREEN           32780
#define ID_OPTIONS_MUSIC                32781
#define ID_OPTIONS_SOUND                32782
#define ID_OPTIONS_QUICKHELP_AT_STARTUP 32783
#define ID_OPTIONS_CONTROLS             32784
#define ID_OPTIONS_CUSTOMIZE            32785
#define ID_HELP_CONTENTS                32786
#define ID_HELP_HOWTOPLAY               32787
#define ID_ESCAPE                       32800
#define ID_HELP_QUICKHELP               32808
#define ID_CMD_32821                    32821
#define ID_CMD_34000                    34000
#define ID_CMD_34002                    34002
#define ID_CMD_34005                    34005
#define ID_CMD_34006                    34006
#define ID_CMD_34007                    34007
#define ID_CMD_34008                    34008
#define ID_CMD_34009                    34009
#define ID_CMD_34010                    34010
#define ID_CMD_34011                    34011
#define ID_CMD_34012                    34012
#define ID_CMD_34013                    34013
#define ID_CMD_34014                    34014
#define ID_CMD_34015                    34015
#define ID_CMD_34016                    34016
#define ID_CMD_34017                    34017
#define ID_CMD_34018                    34018
#define ID_CMD_34019                    34019
#define ID_CMD_34021                    34021
#define ID_CMD_34022                    34022
#define ID_CMD_34023                    34023
#define ID_CMD_34024                    34024
#define ID_CMD_34025                    34025
#define ID_CMD_34026                    34026
#define ID_CMD_34027                    34027
#define ID_CMD_34028                    34028
#define ID_CMD_34029                    34029
#define ID_CMD_34030                    34030
#define ID_CMD_34031                    34031
#define ID_CMD_34032                    34032
#define ID_CMD_34033                    34033
#define ID_CMD_34034                    34034
#define ID_GAME_HIGHSCORES              40006
#define ID_OPTIONS_SOUNDS               40009

#endif // __RESOURCE_H__
