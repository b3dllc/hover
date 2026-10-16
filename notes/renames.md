# Renamed identifiers

The audit of 2026-10-14 replaced placeholder names (CSubXXXXXX,
Sub_XXXXXX, m_dwUnkXX, g_dwUnk_XXXXXX, ID_CMD_NNNNN, ...) with names
inferred from their use. Older rules in perfect-match.md and older commit
messages still use the placeholders; this table maps them, one section
per pull request. Members are listed as Class::old (with the class's new
name).

## Dialogs, frame, resources (2026-10-14)

| Old | New |
|---|---|
| `CSub413F10` | `CCustomizeDialog` |
| `CSub42C890` | `CPodTimesDialog` |
| `CSub42D910` | `CPlayerControlsDialog` |
| `CSub42FE20` | `CKeyboardDialog` |
| `CSub42E790` | `CSoundOptionsDialog` |
| `CSub42F640` | `CStartAtDialog` |
| `CSub430780` | `CHumanParamsDialog` |
| `CSub4319A0` | `CRobotParamsDialog` |
| `CSub432A50` | `CRobotAIParamsDialog` |
| `CSub424720` | `CLoadingDialog` |
| `CSub424BD0` | `CHighScoresDialog` |
| `CSub434170` | `CEnterHighScoreDialog` |
| `Hover_sub_42DC50` | `Hover_KeyNameToVK` |
| `Hover_sub_42E420` | `Hover_SavePlayerControls` |
| `Hover_sub_40EB50` | `Hover_LoadProgress` |
| `g_pLoadDlg_4C57D4` | `g_pLoadingDlg` |
| `g_dwUnk_4C57C4` | `g_dwLoadDone` |
| `g_dwUnk_4C57C8` | `g_dwLoadTotal` |
| `g_dwUnk_4C483C` | `g_bFirstWindowSnap` |
| `g_dwUnk_4C4840` | `g_bInSnapResize` |
| `s_adwHelpMap` | `s_adwHighScoresHelp` |
| `IDS_PRESS_F2` | `IDS_QUIT_GAME_CONFIRM` |
| `IDD_DBG_130` | `IDD_DBG_PODTIMES` |
| `IDD_DBG_148` | `IDD_DBG_HUMANPARAMS` |
| `IDD_DBG_149` | `IDD_DBG_ROBOTPARAMS` |
| `IDD_DBG_152` | `IDD_DBG_ROBOTAIPARAMS` |
| `ID_CMD_32821` | `ID_DBG_WAVEMIX_CONFIG` |
| `OnCmd32821` | `OnDbgWaveMixConfig` |
| `ID_CMD_34002` | `ID_DBG_SETPOSITION` |
| `OnCmd34002` | `OnDbgSetPosition` |
| `ID_CMD_34005` | `ID_DBG_FULLCLIENT_RENDER` |
| `OnCmd34005` | `OnDbgFullClientRender` |
| `OnUpdateCmd34005` | `OnUpdateDbgFullClientRender` |
| `ID_CMD_34006` | `ID_DBG_FULLSCREEN_TOPMOST` |
| `OnCmd34006` | `OnDbgFullScreenTopmost` |
| `OnUpdateCmd34006` | `OnUpdateDbgFullScreenTopmost` |
| `ID_CMD_34007` | `ID_DBG_SHOW_BEACONS` |
| `OnCmd34007` | `OnDbgShowBeacons` |
| `OnUpdateCmd34007` | `OnUpdateDbgShowBeacons` |
| `ID_CMD_34008` | `ID_DBG_HUMANPARAMS` |
| `OnCmd34008` | `OnDbgHumanParams` |
| `ID_CMD_34009` | `ID_DBG_ROBOTPARAMS` |
| `OnCmd34009` | `OnDbgRobotParams` |
| `ID_CMD_34031` | `ID_DBG_ROBOTAIPARAMS` |
| `OnCmd34031` | `OnDbgRobotAIParams` |
| `ID_CMD_34010` | `ID_DBG_STATUSBAR` |
| `OnCmd34010` | `OnDbgStatusBar` |
| `OnUpdateCmd34010` | `OnUpdateDbgStatusBar` |
| `ID_CMD_34012` | `ID_DBG_PODTIMES` |
| `OnCmd34012` | `OnDbgPodTimes` |
| `ID_CMD_34013` | `ID_DBG_ADD_JUMPPOD` |
| `OnCmd34013` | `OnDbgAddJumpPod` |
| `OnUpdateCmd34013` | `OnUpdateDbgAddJumpPod` |
| `ID_CMD_34014` | `ID_DBG_ADD_SMOKEPOD` |
| `OnCmd34014` | `OnDbgAddSmokePod` |
| `OnUpdateCmd34014` | `OnUpdateDbgAddSmokePod` |
| `ID_CMD_34015` | `ID_DBG_ADD_WALLPOD` |
| `OnCmd34015` | `OnDbgAddWallPod` |
| `OnUpdateCmd34015` | `OnUpdateDbgAddWallPod` |
| `ID_CMD_34016` | `ID_DBG_APPLY_SLOW` |
| `OnCmd34016` | `OnDbgApplySlow` |
| `OnUpdateCmd34016` | `OnUpdateDbgApplySlow` |
| `ID_CMD_34017` | `ID_DBG_APPLY_SPEED` |
| `OnCmd34017` | `OnDbgApplySpeed` |
| `OnUpdateCmd34017` | `OnUpdateDbgApplySpeed` |
| `ID_CMD_34018` | `ID_DBG_APPLY_INVINCIBLE` |
| `OnCmd34018` | `OnDbgApplyInvincible` |
| `OnUpdateCmd34018` | `OnUpdateDbgApplyInvincible` |
| `ID_CMD_34019` | `ID_DBG_APPLY_MAPERASER` |
| `OnCmd34019` | `OnDbgApplyMapEraser` |
| `OnUpdateCmd34019` | `OnUpdateDbgApplyMapEraser` |
| `ID_CMD_34021` | `ID_DBG_SHADE_BASE_DEC` |
| `OnCmd34021` | `OnDbgShadeBaseDec` |
| `ID_CMD_34028` | `ID_DBG_SHADE_BASE_INC` |
| `OnCmd34028` | `OnDbgShadeBaseInc` |
| `ID_CMD_34022` | `ID_DBG_SHADE_RED_INC` |
| `OnCmd34022` | `OnDbgShadeRedInc` |
| `ID_CMD_34023` | `ID_DBG_SHADE_RED_DEC` |
| `OnCmd34023` | `OnDbgShadeRedDec` |
| `ID_CMD_34024` | `ID_DBG_SHADE_BLUE_INC` |
| `OnCmd34024` | `OnDbgShadeBlueInc` |
| `ID_CMD_34025` | `ID_DBG_SHADE_BLUE_DEC` |
| `OnCmd34025` | `OnDbgShadeBlueDec` |
| `ID_CMD_34026` | `ID_DBG_SHADE_GREEN_INC` |
| `OnCmd34026` | `OnDbgShadeGreenInc` |
| `ID_CMD_34027` | `ID_DBG_SHADE_GREEN_DEC` |
| `OnCmd34027` | `OnDbgShadeGreenDec` |
| `ID_CMD_34029` | `ID_DBG_INVINCIBLE` |
| `OnCmd34029` | `OnDbgInvincible` |
| `OnUpdateCmd34029` | `OnUpdateDbgInvincible` |
| `ID_CMD_34030` | `ID_DBG_RESTOCK_FLAGS` |
| `OnCmd34030` | `OnDbgRestockFlags` |
| `OnUpdateCmd34030` | `OnUpdateDbgRestockFlags` |
| `ID_CMD_34032` | `ID_DBG_PIXEL_DOUBLE` |
| `OnCmd34032` | `OnDbgPixelDouble` |
| `OnUpdateCmd34032` | `OnUpdateDbgPixelDouble` |
| `ID_CMD_34033` | `ID_DBG_ALT_PRESENT` |
| `OnCmd34033` | `OnDbgAltPresent` |
| `OnUpdateCmd34033` | `OnUpdateDbgAltPresent` |
| `CPlayerControlsDialog::Sub_42DB60` | `AreKeysUnique` |
| `CPlayerControlsDialog::Sub_42DBE0` | `SelectKey` |
| `CPlayerControlsDialog::Sub_42DD70` | `FillKeyCombo` |
| `CStartAtDialog::Sub_42FA80` | `SetScoreText` |
| `CHumanParamsDialog::Sub_4315E0` | `ApplyParams` |
| `CRobotParamsDialog::Sub_432660` | `ApplyParams` |
| `CRobotAIParamsDialog::Sub_432DB0` | `ApplyParams` |
| `CHighScoresDialog::Sub_424C80` | `LoadScores` |
| `CHighScoresDialog::Sub_424DF0` | `CheckNewHighScore` |
| `CHighScoresDialog::Sub_433860` | `SetScoreText` |
| `CHighScoresDialog::Sub_433940` | `SortScores` |
| `CHighScoresDialog::Sub_433CF0` | `ResetScores` |
| `CHighScoresDialog::Sub_433F10` | `SaveScores` |
| `CHighScoresDialog::Sub_434010` | `OnHelpInfo` |
| `CHighScoresDialog::Sub_434030` | `OnHelpContextMenu` |
| `CHighScoresDialog::Sub_433C60` | `OnClearScores` |
| `CMainFrame::Sub_408A70` | `SetPaneMessage` |
| `CMainFrame::Sub_426CF0` | `RealizeGamePalette` |
| `CMainFrame::Sub_41D850` | `SnapWindowRect` |
| `CQuickHelpDialog::Sub_424F30` | `ReadQuickHelpFlag` |
| `CPodTimesDialog::m_dwUnk44` | `m_nInvincibleTime` |
| `CPodTimesDialog::m_dwUnk48` | `m_nSlowTime` |
| `CPodTimesDialog::m_dwUnk4C` | `m_nSmokeTime` |
| `CPodTimesDialog::m_dwUnk50` | `m_nSpeedTime` |
| `CPodTimesDialog::m_dwUnk54` | `m_nWallTime` |
| `CHighScoresDialog::m_dwUnk134` | `m_bDialogUp` |
| `CMainFrame::m_dwUnk00C0` | `m_bViewPrecreated` |
| `CMainFrame::m_dwUnk00C8` | `m_bInSizeMove` |
| `CMainFrame::m_dwUnk00CC` | `m_bAutoStatusBar` |
| `CMainFrame::m_dwUnk00D0` | `m_bWindowRectValid` |
| `CMainFrame::m_rcUnk00D4` | `m_rcWindow` |

## hover.cpp/hover.h: globals, pools, free functions, CHoverApp (2026-10-14)

| Old | New |
|---|---|
| `g_cbUnk_46002C` | `g_cbTextureHeap` |
| `g_pUnk_46003C` | `g_pTextureHeap` |
| `g_dwUnk_461CFC` | `g_dwMipBias` |
| `g_dwUnk_4C56D8` | `g_bUseWaveMix` |
| `g_dwUnk_46048C` | `g_dwStartAtLevel` |
| `g_dwUnk_46049C` | `g_dwLevel` |
| `g_dwUnk_4C4CE0` | `g_bStartingTimer` |
| `g_dwUnk_4C4CE4` | `g_bTickPending` |
| `g_listUnk_4A45B8` | `g_listPodObjBlocks` |
| `g_dwUnk_4C5330` | `g_dwPodObjFree` |
| `g_listUnk_4A45E0` | `g_listFloorObjBlocks` |
| `g_dwUnk_4C53E0` | `g_dwFloorObjFree` |
| `g_listUnk_4A46A0` | `g_listRobotBlocks` |
| `g_dwUnk_4C54D0` | `g_dwRobotFree` |
| `g_listUnk_4A46D0` | `g_listWallBlocks` |
| `g_dwUnk_4C5588` | `g_dwWallFree` |
| `g_listUnk_4A46F8` | `g_listRegionNodeBlocks` |
| `g_dwUnk_4C55D0` | `g_dwRegionNodeFree` |
| `g_dwUnk_4C55D4` | `g_dwRegionNodeChunk` |
| `g_listUnk_4A4748` | `g_listMerlinDynamicBlocks` |
| `g_dwUnk_4C5620` | `g_dwMerlinDynamicFree` |
| `g_listUnk_4A4858` | `g_listBeaconBlocks` |
| `g_dwUnk_4C57A8` | `g_dwBeaconFree` |
| `g_dwUnk_4C52D8` | `g_dwDirtyObjList` |
| `g_dwUnk_4C52DC` | `g_dwDrawObjList` |
| `Hover_sub_401140` | `Hover_GetView` |
| `Hover_sub_408780` | `Hover_CommitDirtyObjects` |
| `Hover_sub_406C60` | `Hover_ReleaseDrawList` |
| `Hover_sub_40E560` | `Hover_WaveMixActivate` |
| `Hover_sub_427A10` | `Hover_WaitPumpCopyData` |
| `Hover_sub_42D180` | `Hover_LoadGameSettings` |
| `Hover_sub_42D290` | `Hover_SaveGameSettings` |
| `Hover_sub_4234F0` | `Hover_LoadSoundSettings` |
| `Hover_sub_423140` | `Hover_InitSound` |
| `Hover_sub_422E30` | `Hover_ShutdownSound` |
| `Hover_sub_438440` | `Hover_MidiStreamProc` |
| `Hover_sub_438540` | `Hover_BuildTrigTables` |
| `Hover_sub_4385F0` | `Hover_FixAngle` |
| `Hover_sub_4386D0` | `Hover_FixAngleP` |
| `Hover_sub_408A00` | `Hover_ApproxDistance` |
| `Hover_sub_40B6E0` | `Hover_SplitVelocity` |
| `Hover_sub_41B7A0` | `Hover_TangentialComponent` |
| `Hover_sub_41B830` | `Hover_AngleInFrontArc` |
| `Hover_sub_4232E0` | `Hover_LoadControls` |
| `Hover_sub_41F540` | `Hover_InitZones` |
| `Hover_sub_4249C0` | `Hover_LoadHumanParams` |
| `Hover_sub_424A70` | `Hover_LoadRobotParams` |
| `Hover_sub_424B20` | `Hover_LoadRobotAIParams` |
| `Hover_sub_42AED0` | `Hover_FreePodObjPool` |
| `Hover_sub_42B710` | `Hover_FreeFloorObjPool` |
| `Hover_sub_42C2D0` | `Hover_FreeRobotPool` |
| `Hover_sub_42C590` | `Hover_FreeWallPool` |
| `Hover_sub_42C700` | `Hover_FreeRegionNodePool` |
| `Hover_sub_42CB70` | `Hover_FreeMerlinDynamicPool` |
| `Hover_sub_42F050` | `Hover_FreeBeaconPool` |
| `CFixed::Sub_4038E0` | `MulEq` |
| `CFixed::Sub_40A490` | `DivEq` |
| `CFixed::Sub_40B4F0` | `MulShortEq` |
| `CHoverApp::Sub_425870` | `SelectSkillParams` |
| `CHoverApp::Sub_425E40` | `CheckMinimumConfig` |
| `CHoverApp::Sub_425E80` | `CheckSingleInstance` |
| `CHoverApp::Sub_41C7E0` | `FindDataDirectory` |
| `CHoverApp::Sub_4253D0` | `EnterFullScreen` |
| `CHoverApp::Sub_425600` | `LeaveFullScreen` |
| `CHoverApp::Sub_41C8E0` | `StartLevel` |
| `CHoverApp::Sub_425AD0` | `OnLevelComplete` |
| `CHoverApp::Sub_425B20` | `OnGameOver` |
| `CHoverApp::m_csUnk00D4` | `m_csScene` |
| `CHoverApp::m_csUnk00EC` | `m_csOffscreen` |
| `CHoverApp::m_csUnk0104` | `m_csUnused` |
| `CHoverApp::m_csUnk011C` | `m_csWorkerCmd` |
| `CHoverApp::m_csUnk0134` | `m_csInput` |
| `CHoverApp::m_dwUnk0158` | `m_dwSpeedPodTime` |
| `CHoverApp::m_dwUnk015C` | `m_dwInvinciblePodTime` |
| `CHoverApp::m_dwUnk0160` | `m_dwSlowPodTime` |
| `CHoverApp::m_dwUnk0164` | `m_dwWallPodTime` |
| `CHoverApp::m_dwUnk0168` | `m_dwSmokePodTime` |
| `CHoverApp::m_dwUnk01B8` | `m_dwGameState` |
| `CHoverApp::m_dwUnk01BC` | `m_bSuspended` |
| `CHoverApp::m_dwUnk01C0` | `m_bLevelBonus` |
| `CHoverApp::m_dwUnk01C4` | `m_dwSpeedDetail` |
| `CHoverApp::m_dwUnk01C8` | `m_dwSkill` |
| `CHoverApp::m_dwUnk01CC` | `m_dwMazeOrder` |
| `CHoverApp::m_dwUnk01D0` | `m_bRearView` |
| `CHoverApp::m_dwUnk01D4` | `m_bRotateMap` |
| `CHoverApp::m_dwUnk01E0` | `m_bShowBeacons` |
| `CHoverApp::m_dwUnk03B0` | `m_bFullScreen` |
| `CHoverApp::m_dwUnk03B4` | `m_bAppActive` |
| `CHoverApp::m_dwUnk03B8` | `m_bSwitchingMode` |
| `CHoverApp::m_dwUnk03BC` | `m_bFullScreenTopmost` |
| `CHoverApp::m_adwUnk03C0` | `m_adwSavedWindowState` |
| `CHoverApp::m_dwUnk03C0` | `m_dwSavedStyle` |
| `CHoverApp::m_dwUnk03C4` | `m_dwSavedExStyle` |
| `CHoverApp::m_rcUnk03C8` | `m_rcSavedWindow` |
| `CHoverApp::m_hMenuUnk03D8` | `m_hSavedMenu` |
| `CHoverApp::m_dwUnk03E0` | `m_bDisplayDibAvailable` |
| `CHoverApp::m_dwUnk03E4` | `m_bUseDisplayDib` |
| `CHoverApp::m_dwUnk03E8` | `m_bDisplayDibActive` |
| `CHoverApp::m_pUnk03EC` | `m_pbmiDisplayDib` |
| `CHoverApp::m_dwUnk03F0` | `m_dwDisplayDibWnd` |
| `CHoverApp::m_dwUnk03F4` | `m_dwWinVersion` |
| `CHoverApp::m_dwUnk03F8` | `m_bNotNT` |
| `CHoverApp::m_dwUnk03FC` | `m_bNewShell` |
| `CHoverApp::m_dwUnk0404` | `m_bEngineSound` |
| `CHoverApp::m_dwUnk0408` | `m_bGameSounds` |
| `CHoverApp::m_dwUnk040C` | `m_bMusic` |
| `CHoverApp::m_dwUnk0410` | `m_bMidiAvailable` |
| `CHoverApp::m_strUnk0414` | `m_strDataDir` |

## bumpview: dashboard helpers, map renderer, CBumperView (2026-10-14)

| Old | New |
|---|---|
| `CSub423030` | `CVelocityIndicator` |
| `CSub422F50` | `CMapRenderer` |
| `CSub422BD0` | `CDigitDisplay` |
| `CSub42AE90` | `CGaugeFraction` |
| `CSub422C80` | `CSegmentGauge` |
| `CSub422C30` | `CFlagStrip` |
| `CVelocityIndicator::Sub_405A10` | `UpdateNeedle` |
| `CVelocityIndicator::Sub_401E80` | `DrawNeedle` |
| `CVelocityIndicator::Sub_4140E0` | `ComputeScale` |
| `CVelocityIndicator::Sub_411D80` | `CreateTile` |
| `CVelocityIndicator::m_dwUnk20` | `m_dwTileWidth` |
| `CVelocityIndicator::m_dwUnk24` | `m_dwTileHeight` |
| `CVelocityIndicator::m_pUnk28` | `m_pOldBitmap` |
| `CVelocityIndicator::m_pUnk4C` | `m_pTargetDC` |
| `CVelocityIndicator::m_dwUnk50` | `m_dwCenterX` |
| `CVelocityIndicator::m_dwUnk54` | `m_dwCenterY` |
| `CVelocityIndicator::m_dwUnk58` | `m_dwBits` |
| `CVelocityIndicator::m_dwUnk5C` | `m_dwNeedleDX` |
| `CVelocityIndicator::m_dwUnk60` | `m_dwNeedleDY` |
| `CVelocityIndicator::m_pUnk64` | `m_pPen` |
| `CMapRenderer::Sub_407160` | `ProjectScene` |
| `CMapRenderer::Sub_404BB0` | `ProjectObject` |
| `CMapRenderer::Sub_4055B0` | `ProjectPoint` |
| `CMapRenderer::Sub_4053A0` | `DrawScene` |
| `CMapRenderer::Sub_401C00` | `DrawObject` |
| `CMapRenderer::Sub_401980` | `DrawLine` |
| `CMapRenderer::Sub_403910` | `DrawMarker` |
| `CMapRenderer::Sub_401520` | `ClipEndpoint` |
| `CMapRenderer::Sub_42D3E0` | `PresentMap` |
| `CMapRenderer::Sub_40FEE0` | `SetViewport` |
| `CMapRenderer::Sub_40FB00` | `SetBits` |
| `CMapRenderer::Sub_40FBB0` | `SetClipRect` |
| `CMapRenderer::m_pUnk10` | `m_pBits` |
| `CMapRenderer::m_dwUnk14` | `m_dwBitsWidth` |
| `CMapRenderer::m_dwUnk18` | `m_dwBitsHeight` |
| `CMapRenderer::m_bUnk1C` | `m_bColor` |
| `CMapRenderer::m_dwUnk20` | `m_dwColor4` |
| `CMapRenderer::m_dwUnk24` | `m_dwPitch` |
| `CMapRenderer::m_dwUnk28` | `m_dwPitchDwords` |
| `CMapRenderer::m_dwUnk2C` | `m_dwLineDX` |
| `CMapRenderer::m_dwUnk30` | `m_dwLineDY` |
| `CMapRenderer::m_pUnk0084` | `m_pMapDC` |
| `CMapRenderer::m_wUnk0088` | `m_wViewRadius` |
| `CMapRenderer::m_dwUnk008C` | `m_dwColorHuman` |
| `CMapRenderer::m_dwUnk0090` | `m_dwColorRobot` |
| `CMapRenderer::m_dwUnk0094` | `m_dwColorRobotAlt` |
| `CMapRenderer::m_dwUnk0098` | `m_dwColorYellow` |
| `CMapRenderer::m_dwUnk009C` | `m_dwColorWhite` |
| `CMapRenderer::m_dwUnk00A0` | `m_dwColorBlack` |
| `CMapRenderer::m_dwUnk00A4` | `m_dwColorLightGray` |
| `CMapRenderer::m_dwUnk00A8` | `m_dwColorGray` |
| `CMapRenderer::m_dwUnk00AC` | `m_dwBlinkPhase` |
| `CMapRenderer::m_dwUnk00C8` | `m_dwZoom` |
| `CDigitDisplay::Sub_40FCE0` | `DrawDigits` |
| `CDigitDisplay::Sub_411A00` | `LoadDigits` |
| `CDigitDisplay::Sub_411920` | `SetPosition` |
| `CDigitDisplay::m_dwUnk00` | `m_bDigitsDirty` |
| `CDigitDisplay::m_dwUnk04` | `m_dwDigits` |
| `CDigitDisplay::m_dwUnk08` | `m_dwDisplayValue` |
| `CDigitDisplay::m_dwUnk0C` | `m_dwStripWidth` |
| `CDigitDisplay::m_dwUnk10` | `m_dwStripHeight` |
| `CDigitDisplay::m_dwUnk14` | `m_dwDigitWidth` |
| `CDigitDisplay::m_dwUnk18` | `m_dwDigitHeight` |
| `CDigitDisplay::m_pUnk1C` | `m_pStripBits` |
| `CDigitDisplay::m_dwUnk20` | `m_dwDestX` |
| `CDigitDisplay::m_dwUnk24` | `m_dwDestY` |
| `CSegmentGauge::Sub_42A9F0` | `SetFraction` |
| `CSegmentGauge::Sub_401AD0` | `DrawGauge` |
| `CSegmentGauge::Sub_411D60` | `SetStyle` |
| `CSegmentGauge::Sub_411D10` | `SetLayout` |
| `CSegmentGauge::m_dwUnk10` | `m_dwBrushOff` |
| `CSegmentGauge::m_dwUnk14` | `m_dwBrushOn` |
| `CSegmentGauge::m_dwUnk18` | `m_dwPen` |
| `CSegmentGauge::m_dwUnk1C` | `m_dwSegments` |
| `CSegmentGauge::m_dwUnk20` | `m_bHorizontal` |
| `CSegmentGauge::m_dwUnk24` | `m_dwLevel` |
| `CSegmentGauge::m_dwUnk28` | `m_dwLastLevel` |
| `CSegmentGauge::m_dwUnk2C` | `m_dwSegmentSize` |
| `CFlagStrip::Sub_40E9A0` | `DrawFlags` |
| `CFlagStrip::Sub_411BB0` | `LoadIcons` |
| `CFlagStrip::Sub_411CF0` | `SetPosition` |
| `CFlagStrip::m_dwUnk00` | `m_bFlagsDirty` |
| `CFlagStrip::m_dwUnk04` | `m_dwDestX` |
| `CFlagStrip::m_dwUnk08` | `m_dwDestY` |
| `CFlagStrip::m_pUnk14` | `m_pFullBits` |
| `CFlagStrip::m_pUnk18` | `m_pEmptyBits` |
| `CFlagStrip::m_rcSource1` | `m_rcFull` |
| `CFlagStrip::m_rcSource2` | `m_rcEmpty` |
| `CFlagStrip::m_dwUnk3C` | `m_dwSlots` |
| `CFlagStrip::m_dwUnk44` | `m_dwFilled` |
| `CBumperView::Sub_427CF0` | `SetGameState` |
| `CBumperView::Sub_41F1C0` | `CreateGdiObjects` |
| `CBumperView::Sub_40EE90` | `SuspendGame` |
| `CBumperView::Sub_410280` | `StopGameTimer` |
| `CBumperView::Sub_40DC20` | `RequestRenderFrame` |
| `CBumperView::Sub_4277E0` | `RequestDispDibBegin` |
| `CBumperView::Sub_4278E0` | `RequestDispDibEnd` |
| `CBumperView::Sub_4279E0` | `WaitWorkerIdle` |
| `CBumperView::Sub_41F4E0` | `SetMessageState` |
| `CBumperView::Sub_413A00` | `RequestDetailLevel` |
| `CBumperView::Sub_413B10` | `RequestDashboardRebuild` |
| `CBumperView::Sub_41E5B0` | `ResetDashboard` |
| `CBumperView::Sub_4282F0` | `ComputeLevelBonus` |
| `CBumperView::Sub_41F480` | `OnFrameResized` |
| `CBumperView::Sub_402250` | `RenderFrame` |
| `CBumperView::Sub_407430` | `PresentFrame` |
| `CBumperView::Sub_4184B0` | `SetupZone` |
| `CBumperView::Sub_41ECE0` | `WorkerThreadMain` |
| `CBumperView::Sub_4274B0` | `RequestResize` |
| `CBumperView::Sub_410510` | `BuildDashboard` |
| `CBumperView::Sub_410380` | `DrawKeyLabels` |
| `CBumperView::Sub_428120` | `DrawF4Hint` |
| `CBumperView::Sub_413C10` | `ApplyDetailLevel` |
| `CBumperView::Sub_4126C0` | `StartGameTimer` |
| `CBumperView::Sub_427A80` | `ZoomMap` |
| `CBumperView::Sub_427BB0` | `BeginDispDib` |
| `CBumperView::Sub_427CC0` | `EndDispDib` |
| `CBumperView::Sub_40D340` | `DrawPausedMessage` |
| `CBumperView::Sub_40CCE0` | `DrawMessageBox` |
| `CBumperView::Sub_40D0A0` | `DrawMessageLine` |
| `CBumperView::Sub_427F40` | `DrawBevelBox` |
| `CBumperView::Sub_4283F0` | `DrawBonusMessage` |
| `CBumperView::Sub_4275D0` | `RequestMapZoom` |
| `CBumperView::Sub_4276E0` | `RequestWorkerExit` |
| `CBumperView::Sub_41F350` | `DestroyRenderer` |
| `CBumperView::Sub_408D00` | `GameStep` |
| `CBumperView::Sub_408DF0` | `UpdateStatusBar` |
| `CBumperView::Sub_427D40` | `StepObjects` |
| `CBumperView::Sub_427E50` | `ThinkAll` |
| `CBumperView::Sub_427E90` | `RunCollider` |
| `CBumperView::m_dwUnk0034` | `m_bCursorShown` |
| `CBumperView::m_dwUnk003C` | `m_dwKeySeqPos` |
| `CBumperView::m_dwUnk0040` | `m_dwRenderThread` |
| `CBumperView::m_dwUnk0048` | `m_dwEvtCommand` |
| `CBumperView::m_hUnk004C` | `m_hEvtWorkerBusy` |
| `CBumperView::m_hUnk0050` | `m_hEvtWorkerIdle` |
| `CBumperView::m_hUnk0054` | `m_hEvtCommandAck` |
| `CBumperView::m_hUnk0058` | `m_hEvtUnused1` |
| `CBumperView::m_hUnk005C` | `m_hEvtUnused2` |
| `CBumperView::m_dwUnk0060` | `m_dwTimerID` |
| `CBumperView::m_dwUnk006C` | `m_bInputLeft` |
| `CBumperView::m_dwUnk0070` | `m_bInputRight` |
| `CBumperView::m_dwUnk0074` | `m_bInputUp` |
| `CBumperView::m_dwUnk0078` | `m_bInputDown` |
| `CBumperView::m_dwUnk007C` | `m_dwStateCountdown` |
| `CBumperView::m_dwUnk0080` | `m_bNeedRepaint` |
| `CBumperView::m_dwUnk0084` | `m_bInitialized` |
| `CBumperView::m_dwUnk008C` | `m_dwOffscreenWidth` |
| `CBumperView::m_dwUnk0090` | `m_dwOffscreenHeight` |
| `CBumperView::m_rcUnk0094` | `m_rcMessage` |
| `CBumperView::m_dwUnk00A4` | `m_dwRequestedWidth` |
| `CBumperView::m_dwUnk00A8` | `m_dwRequestedHeight` |
| `CBumperView::m_dwUnk00AC` | `m_dwWorkerCmd` |
| `CBumperView::m_dwUnk00B0` | `m_bZoomIn` |
| `CBumperView::m_dcUnk00B4` | `m_dcOffscreen` |
| `CBumperView::m_dwUnk00C4` | `m_dwOldBitmap` |
| `CBumperView::m_pUnk00C8` | `m_pViewCtx` |
| `CBumperView::m_pUnk00CC` | `m_pMirrorCtx` |
| `CBumperView::m_pUnk00D0` | `m_pRasterizer` |
| `CBumperView::m_dwUnk00D4` | `m_dwCamX` |
| `CBumperView::m_dwUnk00D8` | `m_dwCamY` |
| `CBumperView::m_dwUnk00DC` | `m_dwCamAngle` |
| `CBumperView::m_dwUnk00E0` | `m_dwCamZ` |
| `CBumperView::m_dwUnk00E4` | `m_dwDrawerMode` |
| `CBumperView::m_obj00E8` | `m_needle` |
| `CBumperView::m_obj0150` | `m_map` |
| `CBumperView::m_dwUnk021C` | `m_bSmokeBlind` |
| `CBumperView::m_rcUnk0220` | `m_rc3DView` |
| `CBumperView::m_rcUnk0230` | `m_rcMirrorView` |
| `CBumperView::m_obj0240` | `m_ctrJumpPods` |
| `CBumperView::m_obj0268` | `m_ctrSmokeBombs` |
| `CBumperView::m_obj0290` | `m_ctrTempWalls` |
| `CBumperView::m_obj02B8` | `m_score` |
| `CBumperView::m_obj02E0` | `m_gaugeSpeed` |
| `CBumperView::m_obj0310` | `m_gaugeInvincible` |
| `CBumperView::m_obj0340` | `m_gaugeSlow` |
| `CBumperView::m_obj0370` | `m_gaugeSpeedPod` |
| `CBumperView::m_obj03A0` | `m_gaugeAltitude` |
| `CBumperView::m_obj03D0` | `m_gaugeSmoke` |
| `CBumperView::m_obj0400` | `m_gaugeTempWall` |
| `CBumperView::m_obj0430` | `m_flagsPlayer` |
| `CBumperView::m_obj0480` | `m_flagsRobot` |
| `CBumperView::m_pen04D0` | `m_penBlack` |
| `CBumperView::m_pen04D8` | `m_penYellow` |
| `CBumperView::m_brush04E0` | `m_brushBlack` |
| `CBumperView::m_brush04E8` | `m_brushYellow` |
| `CBumperView::m_brush04F0` | `m_brushGreen` |
| `CBumperView::m_brush04F8` | `m_brushRed` |
| `CBumperView::m_dwUnk0500` | `m_dwMessageState` |
| `CBumperView::m_dwUnk0504` | `m_dwFlagsLeft` |
| `CBumperView::m_dwUnk0508` | `m_dwPointsPerFlag` |
| `CBumperView::m_dwUnk050C` | `m_dwFlagBonus` |
| `CBumperView::m_dwUnk0510` | `m_dwNextLevel` |
| `CBumperView::m_dwUnk0514` | `m_dwLevelBonus` |
| `CBumperView::m_dwUnk0528` | `m_dwFpsBaseTick` |
| `CBumperView::m_dwUnk052C` | `m_dwLastPresentTick` |
| `CBumperView::m_dwUnk0530` | `m_dwPresentCount` |
| `CBumperView::m_dwUnk0534` | `m_dwStepRateBaseTick` |
| `CBumperView::m_dwUnk0538` | `m_dwLastStepTick` |
| `CBumperView::m_dwUnk053C` | `m_dwStepCount` |
| `CBumperView::m_dwUnk0540` | `m_bFullMap` |
| `CBumperView::m_dwUnk0544` | `m_bDoubleSize` |
| `g_adwUnk_4C4CC8` | `g_adwDrawerModeForDetail` |
| `g_paletteUnk_4A1D50` | `g_palGame` |
| `g_dwUnk_460998` | `g_dwCurZone` |
| `g_dwUnk_4C4CA8` | `g_bFullFramePresent` |
| `g_nUnk_4C4CEC` | `g_nStatusRefreshCountdown` |
| `g_dwUnk_4C4CF0` | `g_dwPrevJoyButtons` |
| `g_dwUnk_4A4790` | `g_dwMapCamX` |
| `g_dwUnk_4A4794` | `g_dwMapCamY` |
| `g_dwUnk_4A4798` | `g_dwMapCullRight` |
| `g_dwUnk_4A479C` | `g_dwMapCullBottom` |
| `g_dwUnk_4A47BC` | `g_dwMapCullLeft` |
| `g_dwUnk_4A47D8` | `g_dwMapCullTop` |
| `g_dwUnk_4A47B0` | `g_dwMapFaceCount` |
| `g_dwUnk_4A47B8` | `g_dwMapRotAngle` |
| `g_dwUnk_4A47E0` | `g_dwMapOriginX` |
| `g_dwUnk_4A47E4` | `g_dwMapOriginY` |
| `Hover_sub_453352` | `Hover_FindResource` |
| `Hover_sub_418A30` | `Hover_LookupTexture` |
| `Hover_sub_408AA0` | `Hover_GameTimerProc` |
| `Hover_sub_410CE0` | `Hover_RemapDibToPalette` |
| `Hover_sub_411770` | `Hover_TransparentBlt8` |
| `Hover_sub_4044A0` | `Hover_InsertRenderObj` |
| `Hover_sub_404580` | `Hover_RemoveRenderObj` |
| `Hover_sub_4045E0` | `Hover_BuildDepthList` |
| `Hover_sub_406D30` | `Hover_UpdateDepthList` |
| `Hover_sub_41ECC0` | `Hover_RenderThreadProc` |

## gameobj/texture: CGameObj's virtual slots, CPlayer, CSegment, the Merlin classes, CSoundSlot (2026-10-14)

| Old | New |
|---|---|
| `Sub_414A50` | `SetMerlin` |
| `Sub_42AD30` | `Think` |
| `Sub_42AD40` | `OnTouch` |
| `Sub_42AD50` | `UsePod` |
| `Sub_42AD70` | `IsNear` |
| `Sub_42AD60` | `Touches` |
| `Sub_408820` | `UpdateRegions` |
| `Sub_411570` | `IntersectLine` |
| `Sub_42AD90` | `OnProbeHit` |
| `Sub_4087D0` | `BeginFrame` |
| `Sub_4087F0` | `EndFrame` |
| `Sub_413020` | `KillObject` |
| `Sub_4089E0` | `GetProbe` |
| `CSub40D320` | `CSegment` |
| `CSub40B800` | `CSoundSlot` |
| `MERLIN_TEXENTRY` | `MERLIN_TEXFRAME` |
| `tagMERLIN_TEXENTRY` | `tagMERLIN_TEXFRAME` |
| `Hover_sub_43A860` | `Hover_WaveMixFreeWave` |
| `Hover_sub_41A4E0` | `Hover_FindStatic` |
| `Hover_sub_40FE40` | `Hover_FindLocation` |
| `g_dwUnk_4A45A0` | `g_dwGameObjects` |
| `CSegment::Sub_42C820` | `Recalc` |
| `CSegment::Sub_422BF0` | `SetEndpoints` |
| `CSegment::Sub_40E8E0` | `SetFromLine` |
| `CSegment::Sub_40D2F0` | `SideOf` |
| `CSegment::Sub_40C9B0` | `DistanceTo` |
| `CSegment::m_dwUnk00` | `m_dwX1` |
| `CSegment::m_dwUnk04` | `m_dwY1` |
| `CSegment::m_dwUnk08` | `m_dwX2` |
| `CSegment::m_dwUnk0C` | `m_dwY2` |
| `CSegment::m_dwUnk10` | `m_dwDeltaX` |
| `CSegment::m_dwUnk14` | `m_dwDeltaY` |
| `CSegment::m_dwUnk18` | `m_dwDistSq` |
| `CSegment::m_dwUnk1C` | `m_dwAngle` |
| `tagMERLIN_TEXFRAME::m_dwUnk00` | `m_dwColMask` |
| `tagMERLIN_TEXFRAME::m_dwUnk08` | `m_dwWidth` |
| `tagMERLIN_TEXFRAME::m_dwUnk0C` | `m_dwHeight` |
| `tagMERLIN_TEXFRAME::m_dwUnk14` | `m_cbPixels` |
| `tagMERLIN_TEXFRAME::m_dwUnk18` | `m_dwPixels` |
| `tagMERLIN_TEXFRAME::m_dwUnk1C` | `m_dwColumns` |
| `CMerlinObject::m_strUnk04` | `m_strName` |
| `CMerlinTexture::m_wUnk14` | `m_wWidth` |
| `CMerlinTexture::m_wUnk16` | `m_wColMask` |
| `CMerlinTexture::m_wUnk18` | `m_wHeight` |
| `CMerlinTexture::m_dwUnk1C` | `m_bTransparent` |
| `CMerlinTexture::m_pUnk28` | `m_pFrames` |
| `CMerlinTexture::m_dwUnk2C` | `m_dwFrameCount` |
| `CMerlinTexture::m_pUnk38` | `m_pNextFrame` |
| `CMerlinLine::Sub_4010C0` | `Recalc` |
| `CMerlinLine::Sub_401260` | `IntersectSegment` |
| `CMerlinStatic::m_strUnk5C` | `m_strFrontTex` |
| `CMerlinStatic::m_strUnk68` | `m_strBackTex` |
| `CMerlinStatic::m_strUnk44` | `m_strFrontTopTex` |
| `CMerlinStatic::m_strUnk50` | `m_strBackTopTex` |
| `CMerlinStatic::m_strUnk74` | `m_strFrontBottomTex` |
| `CMerlinStatic::m_strUnk80` | `m_strBackBottomTex` |
| `CMerlinStatic::m_pUnk8C` | `m_pFrontTopTex` |
| `CMerlinStatic::m_pUnk90` | `m_pBackTopTex` |
| `CMerlinStatic::m_pUnk94` | `m_pFrontTex` |
| `CMerlinStatic::m_pUnk98` | `m_pBackTex` |
| `CMerlinStatic::m_pUnk9C` | `m_pFrontBottomTex` |
| `CMerlinStatic::m_pUnkA0` | `m_pBackBottomTex` |
| `CMerlinStatic::m_wUnkA4` | `m_wBottom` |
| `CMerlinStatic::m_wUnkA6` | `m_wTop` |
| `CMerlinStatic::m_dwUnkB0` | `m_bFrontTransparent` |
| `CMerlinStatic::m_dwUnkB4` | `m_bBackTransparent` |
| `CMerlinStatic::m_wUnkB8` | `m_wFrontUOrg` |
| `CMerlinStatic::m_wUnkBA` | `m_wBackUOrg` |
| `CMerlinStatic::m_dwUnkBC` | `m_bSolid` |
| `CMerlinDynamic::m_strUnk44` | `m_strTexName` |
| `CMerlinDynamic::m_pUnk50` | `m_pTexture` |
| `CMerlinDynamic::m_wUnk54` | `m_wBottom` |
| `CMerlinDynamic::m_wUnk56` | `m_wTop` |
| `CMerlinDynamic::m_wUnk58` | `m_wHalfWidth` |
| `CMerlinDynamic::m_wUnk5A` | `m_wX` |
| `CMerlinDynamic::m_wUnk5C` | `m_wY` |
| `CMerlinDynamic::m_dwUnk64` | `m_bBillboard` |
| `CMerlinDynamic::m_dwUnk68` | `m_bMirror` |
| `CMerlinDynamic::m_dwUnk6C` | `m_dwNextFree` |
| `CMerlinLocation::m_wUnk14` | `m_wX` |
| `CMerlinLocation::m_wUnk16` | `m_wY` |
| `CMerlinLocation::m_wUnk18` | `m_wAngle` |
| `CMerlinLocation::m_wUnk1A` | `m_wHeight` |
| `CMerlinBSP::m_wUnk44` | `m_wFace` |
| `CMerlinBSP::m_wUnk48` | `m_wFront` |
| `CMerlinBSP::m_wUnk4A` | `m_wBack` |
| `CMerlinWorld::m_arrUnk14` | `m_arrStatics` |
| `CMerlinWorld::m_arrUnk28` | `m_arrSprites` |
| `CMerlinWorld::m_arrUnk3C` | `m_arrLocations` |
| `CMerlinWorld::m_arrUnk50` | `m_arrNodes` |
| `CMerlinWorld::Sub_418EA0` | `GetTextureBytes` |
| `CMerlinWorld::Sub_418F00` | `FlushSystemPalette` |
| `CMerlinWorld::Sub_418FA0` | `BuildIdentityPalette` |
| `CMerlinWorld::Sub_413DB0` | `RemoveObject` |
| `CMerlinWorld::Sub_4128A0` | `AddSprite` |
| `CMerlinWorld::Sub_414300` | `RemoveSprite` |
| `CGameObj::Sub_408530` | `QueueBeginFrame` |
| `CGameObj::Sub_403880` | `GetCellPos` |
| `CGameObj::Sub_4038A0` | `GetFixedPos` |
| `CGameObj::Sub_406C40` | `GetPrevPos` |
| `CGameObj::Sub_413000` | `GetFramePos` |
| `CGameObj::Sub_40F880` | `SetCellPos` |
| `CGameObj::Sub_40A9B0` | `ReleaseRegionNodes` |
| `CGameObj::Sub_40D6E0` | `PlayObjSound` |
| `CGameObj::Sub_41A580` | `ResetState` |
| `CGameObj::m_obj04` | `m_seg` |
| `CGameObj::m_dwUnk28` | `m_bInUse` |
| `CGameObj::m_dwUnk2C` | `m_bSolidHit` |
| `CGameObj::m_dwUnk30` | `m_dwRegionNodes` |
| `CGameObj::m_dwUnk34` | `m_dwLastSoundTick` |
| `CGameObj::m_dwUnk38` | `m_bHideOnMap` |
| `CGameObj::m_dwUnk3C` | `m_bOnMap` |
| `CGameObj::m_dwUnk40` | `m_dwMapColor` |
| `CGameObj::m_dwUnk4C` | `m_dwFrameX` |
| `CGameObj::m_dwUnk50` | `m_dwFrameY` |
| `CGameObj::m_dwUnk54` | `m_dwPrevX` |
| `CGameObj::m_dwUnk58` | `m_dwPrevY` |
| `CGameObj::m_dwUnk5C` | `m_bKilled` |
| `CGameObj::m_dwUnk60` | `m_bDead` |
| `CGameObj::m_dwUnk68` | `m_bOnStartList` |
| `CGameObj::m_dwUnk6C` | `m_bOnEndList` |
| `CGameObj::m_dwUnk70` | `m_dwNextStart` |
| `CGameObj::m_dwUnk74` | `m_dwNextEnd` |
| `CGameObj::m_dwUnk78` | `m_dwMerlin` |
| `CPlayer::Sub_414A80` | `ResetMotion` |
| `CPlayer::Sub_414BD0` | `ClearPods` |
| `CPlayer::Sub_413ED0` | `SetParams` |
| `CPlayer::Sub_408550` | `UpdateProbe` |
| `CPlayer::Sub_40A9F0` | `ApplyControls` |
| `CPlayer::Sub_40AB70` | `UpdateHeight` |
| `CPlayer::Sub_40ACE0` | `UpdateHoldPad` |
| `CPlayer::Sub_40ADE0` | `UpdateSled` |
| `CPlayer::Sub_40B0B0` | `MovePlayer` |
| `CPlayer::Sub_42B8E0` | `CollidePlayer` |
| `CPlayer::Sub_42BE50` | `TouchesPlayer` |
| `CPlayer::Sub_41B2D0` | `Spawn` |
| `CPlayer::Sub_41B0E0` | `PlacePlayer` |
| `CPlayer::Sub_41A6B0` | `BounceOffWall` |
| `CPlayer::Sub_41AB10` | `Reflect` |
| `CPlayer::Sub_421B50` | `BounceOffEndpoint` |
| `CPlayer::m_obList00B4` | `m_listFlags` |
| `CPlayer::m_obList00D0` | `m_listTempWallPods` |
| `CPlayer::m_obList00EC` | `m_listSmokeBombPods` |
| `CPlayer::m_obList0108` | `m_listJumpPods` |
| `CPlayer::m_dwUnk0124` | `m_dwInvinciblePod` |
| `CPlayer::m_dwUnk0128` | `m_dwSpeedPod` |
| `CPlayer::m_dwUnk012C` | `m_dwSlowPod` |
| `CPlayer::m_dwUnk0130` | `m_dwTempWallPod` |
| `CPlayer::m_dwUnk0134` | `m_dwSmokeBombPod` |
| `CPlayer::m_dwUnk0138` | `m_dwJumpPod` |
| `CPlayer::m_dwUnk013C` | `m_bOnTop` |
| `CPlayer::m_dwUnk0140` | `m_bInvincible` |
| `CPlayer::m_dwUnk0144` | `m_bGodMode` |
| `CPlayer::m_dwUnk0148` | `m_dwHoldTimer` |
| `CPlayer::m_dwUnk014C` | `m_dwSledTimer` |
| `CPlayer::m_dwUnk0154` | `m_bOnHoldPad` |
| `CPlayer::m_dwUnk0158` | `m_bOnFlagRemover` |
| `CPlayer::m_dwUnk015C` | `m_dwFlagRemoverState` |
| `CPlayer::m_dwUnk0190` | `m_dwProbeRadius` |
| `CPlayer::m_dwUnk01A0` | `m_dwFrameHeading` |
| `CPlayer::m_dwUnk01A4` | `m_dwAltitude` |
| `CPlayer::m_dwUnk01A8` | `m_dwFrameAltitude` |
| `CPlayer::m_dwUnk01CC` | `m_bBounced` |
| `CPlayer::m_dwUnk01DC` | `m_bSteerAssist` |
| `CPlayer::m_dwUnk01E0` | `m_bFriction` |
| `CPlayer::m_dwUnk01E4` | `m_dwHeadingFrame` |
| `CPlayer::m_dwUnk01E8` | `m_dwFrame` |
| `CPlayer::m_dwUnk01EC` | `m_bMirror` |
| `CPlayer::m_dwUnk01F8` | `m_bHuman` |
| `CSoundSlot::Sub_417860` | `LoadWave` |
| `CSoundSlot::Sub_40B800` | `PlayWave` |
| `CSoundSlot::Sub_4118A0` | `StopWave` |
| `CSoundSlot::m_pUnk04` | `m_pWaveData` |
| `CSoundSlot::m_pUnk08` | `m_pMixWave` |
| `CSoundSlot::m_strUnk0C` | `m_strWaveName` |
| `CSoundSlot::m_dwUnk18` | `m_bOpenOnPlay` |

## mazeobj/bumperdoc: CBumperDoc, CMidiStream, pods, walls, pads, beacons (2026-10-14)

| Old | New |
|---|---|
| `CSub425050` | `CMidiStream` |
| `CSub41E1E0` | `CGameObjList` |
| `CSub42CFE0` | `CSetPositionDialog` |
| `CSub424210` | `CBeaconArray` |
| `g_pfnUnk_4C5908` | `g_pfnMidiStreamOpen` |
| `g_pfnUnk_4C590C` | `g_pfnMidiStreamClose` |
| `g_pfnUnk_4C5910` | `g_pfnMidiStreamProperty` |
| `g_pfnUnk_4C5914` | `g_pfnMidiStreamPosition` |
| `g_pfnUnk_4C5918` | `g_pfnMidiStreamOut` |
| `g_pfnUnk_4C591C` | `g_pfnMidiStreamPause` |
| `g_pfnUnk_4C5920` | `g_pfnMidiStreamRestart` |
| `g_pfnUnk_4C5924` | `g_pfnMidiStreamStop` |
| `g_szUnk_4C59A4` | `g_szMidiStreamOpen` |
| `g_szUnk_4C5994` | `g_szMidiStreamClose` |
| `g_szUnk_4C5980` | `g_szMidiStreamProperty` |
| `g_szUnk_4C596C` | `g_szMidiStreamPosition` |
| `g_szUnk_4C595C` | `g_szMidiStreamOut` |
| `g_szUnk_4C594C` | `g_szMidiStreamPause` |
| `g_szUnk_4C5938` | `g_szMidiStreamRestart` |
| `g_szUnk_4C5928` | `g_szMidiStreamStop` |
| `g_pUnk_4C5904` | `g_pCurrentMidiStream` |
| `g_apszUnk_4C4950` | `g_apszSoundNames` |
| `g_mapUnk_4A4728` | `g_mapDynamicSprites` |
| `Hover_sub_4151E0` | `Hover_DeleteBeacons` |
| `g_dwUnk_4C5334` | `g_dwPodPoolBlock` |
| `g_dwUnk_4C53E4` | `g_dwFloorObjPoolBlock` |
| `g_dwUnk_4C558C` | `g_dwWallPoolBlock` |
| `g_dwUnk_4C57AC` | `g_dwBeaconPoolBlock` |
| `g_dwUnk_4C5590` | `g_dwWallRegisterCount` |
| `g_dwUnk_4C5624` | `g_dwSpritePoolBlock` |
| `g_obArrUnk_4A4840` | `g_arrBeacons` |
| `Hover_sub_423EA0` | `Hover_BuildBeacons` |
| `Hover_sub_423CD0` | `Hover_LinkBeacons` |
| `CMidiStream::Sub_438260` | `CloseStream` |
| `CMidiStream::Sub_412220` | `PauseMusic` |
| `CMidiStream::Sub_413710` | `ResumeMusic` |
| `CMidiStream::Sub_438300` | `StartStream` |
| `CMidiStream::Sub_437F00` | `LoadMusicFile` |
| `CMidiStream::Sub_41C030` | `OpenMusic` |
| `CMidiStream::m_dwUnk04` | `m_dwFile` |
| `CMidiStream::m_dwUnk08` | `m_dwTimeDiv` |
| `CMidiStream::m_dwUnk0C` | `m_cbMaxBuffer` |
| `CMidiStream::m_dwUnk10` | `m_dwFreeHead` |
| `CMidiStream::m_dwUnk14` | `m_dwFreeTail` |
| `CMidiStream::m_dwUnk18` | `m_dwStream` |
| `CMidiStream::m_dwUnk1C` | `m_dwQueued` |
| `CMidiStream::m_dwUnk20` | `m_bStopping` |
| `CBumperDoc::Sub_412250` | `PlaceObject` |
| `CBumperDoc::Sub_412630` | `RetireObject` |
| `CBumperDoc::Sub_412650` | `RemoveObject` |
| `CBumperDoc::Sub_417C10` | `CountSpawnPoints` |
| `CBumperDoc::Sub_417D40` | `GetRandomSpawnPoint` |
| `CBumperDoc::Sub_410250` | `RemoveFromMazeList` |
| `CBumperDoc::Sub_426E90` | `CreateSounds` |
| `CBumperDoc::Sub_426EE0` | `DestroySounds` |
| `CBumperDoc::m_obj0060` | `m_music` |
| `CBumperDoc::m_dwUnk0184` | `m_bLevelLoaded` |
| `CBumperDoc::m_dwUnk0188` | `m_dwWorld` |
| `CBumperDoc::m_dwUnk8388` | `m_dwCollider` |
| `CBumperDoc::m_list838C` | `m_listMazeObjs` |
| `CBumperDoc::m_list83A8` | `m_listActiveObjs` |
| `CBumperDoc::m_dwUnk83C4` | `m_dwFloorHeight` |
| `CBumperDoc::m_dwUnk83C8` | `m_dwMaxWallHeight` |
| `CBumperDoc::m_dwUnk83CC` | `m_dwHeightRange` |
| `CBumperDoc::m_adwUnk83D0` | `m_adwSpawnPoints` |
| `CBumperDoc::m_dwUnk83E4` | `m_dwScore` |
| `CBumperDoc::m_dwUnk83E8` | `m_dwRobotFlags` |
| `CBumperDoc::m_adwUnk83EC` | `m_adwSounds` |
| `CBumperDoc::m_dblUnk8478` | `m_dblShadeBase` |
| `CBumperDoc::m_dblUnk8480` | `m_dblShadeRed` |
| `CBumperDoc::m_dblUnk8488` | `m_dblShadeBlue` |
| `CBumperDoc::m_dblUnk8490` | `m_dblShadeGreen` |
| `CBumperDoc::m_dwUnk8498` | `m_bAttractMode` |
| `CPodObj::m_dwUnk84` | `m_dwNextFree` |
| `CPodObj::m_dwUnk88` | `m_dwOwner` |
| `CPodObj::m_pUnk8C` | `m_pTexture` |
| `CPodObj::m_dwUnk90` | `m_dwSpriteTexture` |
| `CPodObj::m_dwUnk94` | `m_bOnBoard` |
| `CPodObj::m_dwUnk98` | `m_bRespawn` |
| `CPodObj::m_dwUnk9C` | `m_dwRadius` |
| `CPodObj::m_dwUnkA0` | `m_dwTimeLeft` |
| `CPodObj::m_dwUnkA4` | `m_dwBaseZ` |
| `CPodObj::m_dwUnkAC` | `m_dwZOffset` |
| `CPodObj::m_dwUnkB0` | `m_dwZ` |
| `CPodObj::m_dwUnkB8` | `m_dwHeight` |
| `CPodObj::m_dwUnkBC` | `m_dwHalfWidth` |
| `CPodObj::m_dwUnkCC` | `m_bRobotFlag` |
| `CPodObj::m_dwUnkD0` | `m_dwInner` |
| `CPodObj::m_dwUnkD4` | `m_dwForceField` |
| `CWall::Sub_414D40` | `SetFace` |
| `CWall::m_dwUnk94` | `m_dwBottom` |
| `CWall::m_dwUnk98` | `m_dwTop` |
| `CWall::m_dwUnk9C` | `m_dwNextFree` |
| `CFloorObj::m_dwUnk84` | `m_dwNextFree` |
| `CFloorObj::m_dwUnk88` | `m_dwHitEdge` |
| `CFloorObj::m_dwUnk8C` | `m_dwLowHeight` |
| `CFloorObj::m_dwUnk90` | `m_dwHighHeight` |
| `CFloorObj::m_list94` | `m_listEdges` |
| `CFloorObj::m_dwUnkB0` | `m_dwKind` |
| `CFloorObj::m_dwUnkB4` | `m_dwSledDir` |
| `CBeacon::Sub_40C0E0` | `PickNextHop` |
| `CBeacon::m_arr84` | `m_arrLinks` |
| `CBeacon::m_pUnk98` | `m_pLocation` |
| `CBeacon::m_dwUnkA4` | `m_bBlocked` |
| `CBeacon::m_dwUnkA8` | `m_bTargeted` |
| `CBeacon::m_dwUnkAC` | `m_dwNextFree` |

## texload/texmisc: robot AI states, CRobotPlayer, collider and region grid (2026-10-14)

| Old | New |
|---|---|
| `CSub423670` | `CRobotState` |
| `CSub4235E0` | `CRobotSeekState` |
| `CSub423710` | `CRobotSpinState` |
| `CSub4237A0` | `CRobotGotoState` |
| `CSub4238D0` | `CRobotFlagChaseState` |
| `CSub423960` | `CRobotFlagWaitState` |
| `CSub423A90` | `CRobotRouteState` |
| `CSub423B80` | `CRobotRamState` |
| `CSub424130` | `CBeaconSearch` |
| `Sub_42ECE0` | `OnEnter` |
| `Sub_42ECF0` | `OnLeave` |
| `Hover_sub_413730` | `Hover_NearestPaletteIndex` |
| `Hover_sub_40E730` | `Hover_BlitRect` |
| `Hover_sub_422370` | `Hover_CacheDroneFrames` |
| `Hover_sub_4262B0` | `Hover_FatalException` |
| `g_dwUnk_4C54D4` | `g_dwRobotPoolBlock` |
| `g_szUnk_4C47B0` | `g_szWinMM` |
| `g_awUnk_4C4DB0` | `g_awSquares` |
| `g_apUnk_4A4620` | `g_apDroneFrames` |
| `g_szUnk_460060` | `g_szFatalError` |
| `g_apszUnk_4C4910` | `g_apszSpawnFamilies` |
| `CRobotState::Sub_41BC10` | `ChangeState` |
| `CRobotState::Sub_40A790` | `TryLockOnHuman` |
| `CRobotState::Sub_40BB40` | `TryLockOnFlag` |
| `CRobotState::Sub_42ED00` | `CanSeeHuman` |
| `CRobotState::Sub_40BA10` | `FindVisibleFlag` |
| `CRobotState::m_dwUnk84` | `m_bBlocked` |
| `CRobotState::m_pUnk88` | `m_pRobot` |
| `CRobotSeekState::m_dwUnk8C` | `m_bProbing` |
| `CRobotSpinState::m_dwUnk8C` | `m_dwSpinFrames` |
| `CRobotGotoState::m_dwUnk8C` | `m_dwTimeout` |
| `CRobotFlagChaseState::m_dwUnk8C` | `m_dwTimeout` |
| `CRobotRouteState::m_obj8C` | `m_search` |
| `CRobotRouteState::Sub_40BFF0` | `GiveUp` |
| `CBeaconSearch::Sub_40BEF0` | `NextBeacon` |
| `CBeaconSearch::m_dwUnk04` | `m_dwDepthLimit` |
| `CBeaconSearch::m_dwUnk08` | `m_dwDepth` |
| `CBeaconSearch::m_pUnk0C` | `m_pTop` |
| `CBeaconSearch::m_adwUnk10` | `m_adwFrames` |
| `CBeaconSearch::m_dwUnk38` | `m_dwIteration` |
| `CRobotPlayer::Sub_409350` | `Steer` |
| `CRobotPlayer::Sub_40BC10` | `RememberVisited` |
| `CRobotPlayer::Sub_40C1F0` | `WasRecentlyVisited` |
| `CRobotPlayer::m_dwUnk01FC` | `m_dwTarget` |
| `CRobotPlayer::m_dwUnk0204` | `m_bBrakeAtTarget` |
| `CRobotPlayer::m_dwUnk0208` | `m_bSpinning` |
| `CRobotPlayer::m_dwUnk020C` | `m_bSpinLeft` |
| `CRobotPlayer::m_dwUnk0214` | `m_bStopping` |
| `CRobotPlayer::m_dwUnk0218` | `m_bOvershooting` |
| `CRobotPlayer::m_dwUnk021C` | `m_bHunter` |
| `CRobotPlayer::m_pUnk0220` | `m_pState` |
| `CRobotPlayer::m_adwUnk0224` | `m_adwRecent` |
| `CRobotPlayer::m_dwUnk024C` | `m_dwRecentNext` |
| `CRobotPlayer::m_pUnk0260` | `m_pNextFree` |
| `CRobotPlayer::m_obj0264` | `m_stSeek` |
| `CRobotPlayer::m_obj02F4` | `m_stSpin` |
| `CRobotPlayer::m_obj0388` | `m_stGoto` |
| `CRobotPlayer::m_obj0418` | `m_stFlagChase` |
| `CRobotPlayer::m_obj04A8` | `m_stFlagWait` |
| `CRobotPlayer::m_obj0534` | `m_stRoute` |
| `CRobotPlayer::m_obj05FC` | `m_stRam` |
| `CRegionMatrix::Sub_4086A0` | `AddNode` |
| `CRegionMatrix::m_pUnk04` | `m_pCells` |
| `CRegionMatrix::m_dwUnk08` | `m_dwCols` |
| `CRegionMatrix::m_dwUnk0C` | `m_dwRows` |
| `CRegionMatrix::m_dwUnk10` | `m_dwCells` |
| `CRegionMatrix::m_pUnk14` | `m_pCellsEnd` |
| `CCollider::m_dwUnk04` | `m_dwCols` |
| `CCollider::m_dwUnk08` | `m_dwRows` |
| `CCollider::m_dwUnk0C` | `m_dwCellSize` |
| `CCollider::m_obj10` | `m_grid` |
| `CCollider::m_listUnk28` | `m_listMovers` |
| `CCollider::m_dwUnk44` | `m_bQuerying` |
| `CCollider::m_arrUnk48` | `m_arrQueryHits` |
| `CCollider::Sub_40F900` | `AddStatic` |
| `CCollider::Sub_40E710` | `RemoveObject` |
| `CCollider::Sub_40B510` | `ReinsertObject` |
| `CCollider::Sub_41B770` | `AddMover` |
| `CCollider::Sub_40A650` | `DetectCollisions` |
| `CCollider::Sub_40F8A0` | `CollectCell` |
| `CCollider::Sub_40F9B0` | `IsSpotBlocked` |
| `CCollider::Sub_40C2B0` | `IsPathClear` |
| `CCollider::Sub_411600` | `ProbeSegment` |
| `CShadeTable::Sub_414550` | `BuildTable` |

## texheap: rasterizer, render context, flat-run heap, render structs, renderer globals (2026-10-14)

| Old | New |
|---|---|
| `CSub406090` | `CSpanRasterizer` |
| `CSub406C90` | `CFlatRunHeap` |
| `CSub410F20` | `CRenderContext` |
| `SPANPTS` | `TEXCOLUMN` |
| `tagSPANPTS` | `tagTEXCOLUMN` |
| `SPANPT` | `TEXRUN` |
| `tagSPANPT` | `tagTEXRUN` |
| `HEAPNODE` | `EDGENODE` |
| `tagHEAPNODE` | `tagEDGENODE` |
| `SPANNODE` | `FLATNODE` |
| `tagSPANNODE` | `tagFLATNODE` |
| `Hover_sub_406A80` | `Hover_SortBehindNeighbours` |
| `g_pUnk_4A1D60` | `g_pArenaCursor` |
| `g_dwUnk_4A1D64` | `g_dwColTopY` |
| `g_pUnk_4A1D68` | `g_pRenderCtx` |
| `g_dwUnk_4A1D6C` | `g_dwColProjScale` |
| `g_dwUnk_4A1D70` | `g_dwHorizonY` |
| `g_dwUnk_4A1D74` | `g_dwFlatMipLevel` |
| `g_dwUnk_4A1D78` | `g_bWallMipMapping` |
| `g_abUnk_4A1D80` | `g_abSpanArena` |
| `g_pUnk_4A4490` | `g_pEdgeList` |
| `g_lineUnk_4A4498` | `g_lineFrustumLeft` |
| `g_lineUnk_4A44E0` | `g_lineFrustumRight` |
| `g_dwUnk_4A4524` | `g_dwClipTopY` |
| `g_dwUnk_4A4564` | `g_dwClipBottomY` |
| `g_wUnk_4A4528` | `g_wBspOrder` |
| `g_pdwUnk_4A452C` | `g_pdwDistTable` |
| `g_dwUnk_4A4530` | `g_dwBelowClipY` |
| `g_dwUnk_4A4534` | `g_dwCurColumn` |
| `g_dwUnk_4A4538` | `g_dwLogRowStep` |
| `g_dwUnk_4A453C` | `g_dwRowStep` |
| `g_dwUnk_4A4540` | `g_dwColumnOrigin` |
| `g_dwUnk_4A4544` | `g_dwEyeZ` |
| `g_pbUnk_4A4548` | `g_pbMipLut` |
| `g_dwUnk_4A454C` | `g_dwViewWidth` |
| `g_pdwUnk_4A4550` | `g_pdwSpanLinkCursor` |
| `g_dwUnk_4A4554` | `g_dwLogColStep` |
| `g_dwUnk_4A455C` | `g_dwColStep` |
| `g_dwUnk_4A4568` | `g_dwAboveClipY` |
| `g_dwUnk_4A456C` | `g_dwColBottomY` |
| `g_pUnk_4A4714` | `g_pObjPool` |
| `g_pUnk_4A4718` | `g_pObjArenaCursor` |
| `g_pUnk_4A471C` | `g_pColumnObjs` |
| `g_pUnk_4A4720` | `g_pActiveObjs` |
| `g_dwUnk_4A4A14` | `g_dwSpanCount` |
| `g_dwUnk_4A4A18` | `g_dwSkyU` |
| `g_dwUnk_4A4A1C` | `g_dwTexels` |
| `g_dwUnk_4A4A20` | `g_dwUMask` |
| `g_dwUnk_4A4A24` | `g_dwVStep` |
| `g_dwUnk_4A4A28` | `g_dwU` |
| `g_dwUnk_4A4A2C` | `g_dwV` |
| `g_dwUnk_4A4A30` | `g_dwDstRowStride` |
| `g_dwUnk_4A4A34` | `g_dwRowPitch` |
| `g_dwUnk_4A4A38` | `g_dwVMask` |
| `g_dwUnk_4A4A3C` | `g_dwShadeTable` |
| `g_dwUnk_4A4A40` | `g_dwColumnV` |
| `g_dwUnk_4A4A44` | `g_dwSpanDepth` |
| `g_dwUnk_4A4A48` | `g_dwVShift` |
| `g_dwUnk_4A4A4C` | `g_dwDst` |
| `g_dwUnk_4A4A50` | `g_dwUStep` |
| `g_dwUnk_4C5208` | `g_dwNoEdgeY` |
| `tagTEXRUN::m_wUnk00` | `m_wTop` |
| `tagTEXRUN::m_wUnk02` | `m_wBottom` |
| `tagTEXRUN::m_dwUnk04` | `m_dwPixels` |
| `tagTEXCOLUMN::m_wUnk00` | `m_wRunCount` |
| `tagTEXCOLUMN::m_pUnk04` | `m_pRuns` |
| `tagSPANENTRY::m_dwUnk00` | `m_dwTopZ` |
| `tagSPANENTRY::m_dwUnk04` | `m_dwBottomZ` |
| `tagSPANENTRY::m_dwUnk08` | `m_dwTopY` |
| `tagSPANENTRY::m_dwUnk0C` | `m_dwBottomY` |
| `tagSPANENTRY::m_dwUnk10` | `m_dwPixels` |
| `tagRENDEROBJ::m_wUnk08` | `m_wDistance` |
| `tagRENDEROBJ::m_wUnk0A` | `m_wTexU` |
| `tagRENDEROBJ::m_dwUnk0C` | `m_bSprite` |
| `tagRENDEROBJ::m_pUnk10` | `m_pSource` |
| `tagRENDEROBJ::m_dwUnk14` | `m_bFrontFace` |
| `tagRENDEROBJ::m_dwUnk18` | `m_bHidden` |
| `tagRENDEROBJ::m_wUnk1C` | `m_wBspOrder` |
| `tagRENDEROBJ::m_dwUnk20` | `m_bCovered` |
| `tagRENDEROBJ::m_dwUnk24` | `m_dwSubKey` |
| `tagRENDEROBJ::m_pUnk28` | `m_pPoolPrev` |
| `tagRENDEROBJ::m_pUnk2C` | `m_pPoolNext` |
| `tagRENDEROBJ::m_pUnk30` | `m_pColPrev` |
| `tagRENDEROBJ::m_pUnk34` | `m_pColNext` |
| `tagRENDEROBJ::m_pUnk38` | `m_pActiveNext` |
| `tagRENDEROBJ::m_pUnk3C` | `m_pActivePrev` |
| `tagRENDEROBJ::m_pUnk40` | `m_pTopNeighbour` |
| `tagRENDEROBJ::m_dwUnk44` | `m_bTopAboveEye` |
| `tagRENDEROBJ::m_dwUnk48` | `m_dwTopFlatTex` |
| `tagRENDEROBJ::m_pUnk4C` | `m_pBottomNeighbour` |
| `tagRENDEROBJ::m_dwUnk50` | `m_bBottomAboveEye` |
| `tagRENDEROBJ::m_dwUnk54` | `m_dwBottomFlatTex` |
| `tagRENDEROBJ::m_dwUnk58` | `m_bVisible` |
| `tagRENDEROBJ::m_pUnk5C` | `m_pTex` |
| `tagRENDEROBJ::m_dwUnk60` | `m_dwTopZ` |
| `tagRENDEROBJ::m_dwUnk64` | `m_dwBottomZ` |
| `tagRENDEROBJ::m_dwUnk68` | `m_dwHeight16` |
| `tagRENDEROBJ::m_dwUnk6C` | `m_dwTopDZ` |
| `tagRENDEROBJ::m_dwUnk70` | `m_dwBottomDZ` |
| `tagRENDEROBJ::m_dwUnk74` | `m_dwMinY` |
| `tagRENDEROBJ::m_dwUnk78` | `m_dwMaxY` |
| `tagRENDEROBJ::m_pUnk84` | `m_pTopEdgeY` |
| `tagRENDEROBJ::m_pUnk88` | `m_pBottomEdgeY` |
| `tagRENDEROBJ::m_dwUnk8C` | `m_bHasFlats` |
| `tagRENDEROBJ::m_dwUnk90` | `m_dwRunCount` |
| `tagRENDEROBJ::m_dwUnk94` | `m_dwVMask` |
| `tagRENDEROBJ::m_dwUnk98` | `m_dwVStep` |
| `tagRENDEROBJ::m_dwUnk9C` | `m_dwTexScaleLog2` |
| `tagRENDEROBJ::m_dwUnkA0` | `m_bMasked` |
| `tagRENDEROBJ::m_dwUnkA4` | `m_bMappingCached` |
| `tagRENDEROBJ::m_pUnkA8` | `m_pRuns` |
| `tagRENDEROBJ::m_acUnkAC` | `m_run` |
| `tagEDGENODE::m_pUnk00` | `m_pObj` |
| `tagEDGENODE::m_pUnk04` | `m_pFlatTex` |
| `tagEDGENODE::m_pUnk08` | `m_pNext` |
| `tagEDGENODE::m_pUnk0C` | `m_pPrev` |
| `tagEDGENODE::m_dwUnk10` | `m_bTopEdge` |
| `tagEDGENODE::m_dwUnk14` | `m_dwEdgeZ` |
| `tagFLATNODE::m_pUnk00` | `m_pRunsA` |
| `tagFLATNODE::m_pUnk04` | `m_pRunsB` |
| `tagFLATNODE::m_pUnk08` | `m_ppPrevRuns` |
| `tagFLATNODE::m_pUnk0C` | `m_ppCurRuns` |
| `tagFLATNODE::m_pUnk10` | `m_pRowStartCol` |
| `tagFLATNODE::m_dwUnk14` | `m_dwCurRuns` |
| `tagFLATNODE::m_dwUnk18` | `m_dwPrevRuns` |
| `tagFLATNODE::m_dwUnk1C` | `m_dwHeight` |
| `tagFLATNODE::m_dwUnk20` | `m_dwKey` |
| `tagFLATNODE::m_dwUnk24` | `m_dwTex` |
| `tagFLATNODE::m_pUnk28` | `m_pNext` |
| `tagSPANLINK::m_wUnk00` | `m_wOrder` |
| `tagSPANLINK::m_dwUnk04` | `m_bOpen` |
| `tagSPANLINK::m_dwUnk08` | `m_bSprite` |
| `tagSPANLINK::m_dwUnk0C` | `m_dwObj` |
| `tagSPANLINK::m_dwUnk10` | `m_dwSegment` |
| `tagSPANLINK::m_pUnk14` | `m_pNext` |
| `CRenderContext::Sub_410F20` | `SetMode` |
| `CRenderContext::Sub_406C00` | `SetCamera` |
| `CRenderContext::Sub_410EC0` | `BuildMipShiftTable` |
| `CRenderContext::Sub_414A10` | `SetClipSpans` |
| `CRenderContext::Sub_419940` | `SetSkyTexture` |
| `CRenderContext::Sub_419530` | `SetViewport` |
| `CRenderContext::Sub_419960` | `BuildSkyGradient` |
| `CRenderContext::Sub_403850` | `GetShadeRow` |
| `CRenderContext::m_pfnUnk00` | `m_pfnDrawWall` |
| `CRenderContext::m_pfnUnk04` | `m_pfnDrawSky` |
| `CRenderContext::m_ppfnUnk08` | `m_ppfnDrawFlatByShift` |
| `CRenderContext::m_pfnUnk0C` | `m_pfnDrawFlat` |
| `CRenderContext::m_pUnk14` | `m_pMipShift` |
| `CRenderContext::m_dwUnk18` | `m_dwViewX` |
| `CRenderContext::m_dwUnk1C` | `m_dwViewY` |
| `CRenderContext::m_dwUnk20` | `m_dwViewW` |
| `CRenderContext::m_dwUnk24` | `m_dwViewH` |
| `CRenderContext::m_dwUnk28` | `m_dwScanOrigin` |
| `CRenderContext::m_dwUnk2C` | `m_dwHorizon` |
| `CRenderContext::m_pUnk30` | `m_pColumnScale` |
| `CRenderContext::m_pUnk34` | `m_pClipTop` |
| `CRenderContext::m_pUnk38` | `m_pClipBottom` |
| `CRenderContext::m_dwUnk3C` | `m_dwPixelW` |
| `CRenderContext::m_dwUnk40` | `m_dwPixelH` |
| `CRenderContext::m_dwUnk44` | `m_dwMode` |
| `CRenderContext::m_dwUnk48` | `m_dwCamX` |
| `CRenderContext::m_dwUnk4C` | `m_dwCamY` |
| `CRenderContext::m_dwUnk50` | `m_dwCamZ` |
| `CRenderContext::m_dwUnk54` | `m_dwCamAngle` |
| `CRenderContext::m_pUnk5C` | `m_pRowDistance` |
| `CRenderContext::m_pUnk60` | `m_pShade` |
| `CRenderContext::m_pUnk64` | `m_pRayDirs` |
| `CRenderContext::m_dwUnk68` | `m_bMirror` |
| `CRenderContext::m_pUnk6C` | `m_pSkyGradient` |
| `CRenderContext::m_pUnk70` | `m_pSkyTex` |
| `CRenderContext::m_dwUnk74` | `m_dwSkyMode` |
| `CRenderContext::m_dwUnk78` | `m_dwTargetPitch` |
| `CRenderContext::m_dwUnk7C` | `m_dwRowStride` |
| `CRenderContext::m_dwUnk80` | `m_bUseShiftTable` |
| `CRenderContext::m_dwUnk84` | `m_dwTargetHeight` |
| `CRenderContext::m_dwUnk88` | `m_dwColumns` |
| `CRenderContext::m_dwUnk8C` | `m_dwRows` |
| `CRenderContext::m_dwUnk90` | `m_bLit` |
| `CRenderContext::m_dwUnkA8` | `m_dwLastCamZ` |
| `CFlatRunHeap::Sub_406C90` | `ResetHeap` |
| `CFlatRunHeap::Sub_403980` | `AddFlatRun` |
| `CFlatRunHeap::Sub_403A60` | `FlushRuns` |
| `CFlatRunHeap::Sub_406D00` | `AddSkyRun` |
| `CFlatRunHeap::m_dwUnk00` | `m_dwRows` |
| `CFlatRunHeap::m_pUnk04` | `m_pScratchRuns` |
| `CFlatRunHeap::m_pUnk08` | `m_pSkyNode` |
| `CFlatRunHeap::m_pUnk0C` | `m_pFlatHead` |
| `CFlatRunHeap::m_pUnk10` | `m_pFlatTail` |
| `CFlatRunHeap::m_dwUnk14` | `m_dwFlatNodes` |
| `CFlatRunHeap::m_pUnk18` | `m_pHeapBase` |
| `CFlatRunHeap::m_pUnk1C` | `m_pHeapCursor` |
| `CSpanRasterizer::Sub_406090` | `RenderWorld` |
| `CSpanRasterizer::Sub_419440` | `CreateTargetDIB` |
| `CSpanRasterizer::Sub_4192F0` | `AllocBuffers` |
| `CSpanRasterizer::Sub_405B90` | `IsSegmentVisible` |
| `CSpanRasterizer::Sub_405CE0` | `SubmitSegment` |
| `CSpanRasterizer::Sub_405FA0` | `WalkBSP` |
| `CSpanRasterizer::Sub_402E00` | `RenderColumn` |
| `CSpanRasterizer::Sub_4030F0` | `LinkFlatEdges` |
| `CSpanRasterizer::Sub_402630` | `DrawWallWithFlats` |
| `CSpanRasterizer::Sub_402990` | `DrawWallRun` |
| `CSpanRasterizer::Sub_402B10` | `DrawFlatRow` |
| `CSpanRasterizer::Sub_404030` | `CoverRows` |
| `CSpanRasterizer::Sub_4065F0` | `ClipBottomFlat` |
| `CSpanRasterizer::Sub_4066B0` | `DrawMaskedRun` |
| `CSpanRasterizer::Sub_406740` | `DrawSkyRow` |
| `CSpanRasterizer::m_pUnk00` | `m_pRunHeap` |
| `CSpanRasterizer::m_dwUnk04` | `m_dwBits` |
| `CSpanRasterizer::m_dwUnk1C` | `m_dwRowDir` |
| `CSpanRasterizer::m_pUnk20` | `m_pWorld` |
| `CSpanRasterizer::m_dwUnk24` | `m_bMipMapping` |
| `CSpanRasterizer::m_dwUnk28` | `m_bRelink` |
| `CSpanRasterizer::m_dwUnk30` | `m_dwColumnsPerAngle` |
| `CSpanRasterizer::m_dwUnk34` | `m_dwLeftEdgeAngle` |
| `CSpanRasterizer::m_pUnk3C` | `m_pLinkPool` |
| `CSpanRasterizer::m_pUnk40` | `m_pColumnLists` |
| `CSpanRasterizer::m_pUnk44` | `m_pRenderObjPool` |

## Span drawers (2026-10-14; scheme at the top of spandraw.h)

| Old | New |
|---|---|
| `Hover_sub_40D600` | `Column4x2LitReg` |
| `Hover_sub_40D680` | `Sky4x2Reg` |
| `Hover_sub_40D900` | `Span4x2Lit_S10` |
| `Hover_sub_40D9A0` | `Span4x2Lit_S11` |
| `Hover_sub_40DA40` | `Span4x2Lit_S12` |
| `Hover_sub_40DAE0` | `Span4x2Lit_S13` |
| `Hover_sub_40DB80` | `Span4x2Lit_S14` |
| `Hover_sub_40DDA0` | `Column2x2LitReg` |
| `Hover_sub_40DE20` | `Sky2x2Reg` |
| `Hover_sub_40DF80` | `Span2x2Lit_S10` |
| `Hover_sub_40E020` | `Span2x2Lit_S11` |
| `Hover_sub_40E0C0` | `Span2x2Lit_S12` |
| `Hover_sub_40E160` | `Span2x2Lit_S13` |
| `Hover_sub_40E200` | `Span2x2Lit_S14` |
| `Hover_sub_40E5A0` | `Column2x1LitReg` |
| `Hover_sub_40E600` | `Sky2x1Reg` |
| `Hover_sub_40E850` | `Span2x1Lit_S11` |
| `Hover_sub_40E910` | `Span2x1Lit_S12` |
| `Hover_sub_40EAC0` | `Span2x1Lit_S13` |
| `Hover_sub_40EDD0` | `Span2x1Lit_S14` |
| `Hover_sub_40F540` | `Span2x1Lit_S9` |
| `Hover_sub_40F5D0` | `Span2x1Lit_S10` |
| `Hover_sub_40F6B0` | `Column1x1LitReg` |
| `Hover_sub_40F710` | `Sky1x1` |
| `Hover_sub_40FA90` | `Span1x1Lit_S9` |
| `Hover_sub_40FB40` | `Span1x1Lit_S10` |
| `Hover_sub_40FC70` | `Span1x1Lit_S11` |
| `Hover_sub_40FDD0` | `Span1x1Lit_S12` |
| `Hover_sub_40FE70` | `Span1x1Lit_S13` |
| `Hover_sub_4101E0` | `Span1x1Lit_S14` |
| `Hover_sub_4114D0` | `Column4x4LitReg` |
| `Hover_sub_411940` | `Span4x4Lit_S11` |
| `Hover_sub_411FE0` | `Span4x4Lit_S12` |
| `Hover_sub_4120A0` | `Span4x4Lit_S13` |
| `Hover_sub_412160` | `Span4x4Lit_S14` |
| `Hover_sub_412EF0` | `Column4x1LitReg` |
| `Hover_sub_412F50` | `Sky4x1Reg` |
| `Hover_sub_412FA0` | `Sky4x4Reg` |
| `Hover_sub_413350` | `Span4x1Lit_S9` |
| `Hover_sub_4133F0` | `Span4x1Lit_S10` |
| `Hover_sub_413490` | `Span4x1Lit_S11` |
| `Hover_sub_413530` | `Span4x1Lit_S12` |
| `Hover_sub_4135D0` | `Span4x1Lit_S13` |
| `Hover_sub_413670` | `Span4x1Lit_S14` |
| `Hover_sub_414220` | `Span2x2Lit_S8` |
| `Hover_sub_424FB0` | `Span4x1Lit_S8` |
| `Hover_sub_429960` | `Column1x1Lit` |
| `Hover_sub_4299D0` | `Column1x1` |
| `Hover_sub_429A30` | `Column1x1Reg` |
| `Hover_sub_429A80` | `Column2x1` |
| `Hover_sub_429AF0` | `Column2x1Lit` |
| `Hover_sub_429B70` | `Column2x1Reg` |
| `Hover_sub_429BD0` | `Column4x1` |
| `Hover_sub_429C40` | `Column4x1Lit` |
| `Hover_sub_429CC0` | `Column4x1Reg` |
| `Hover_sub_429D20` | `Column2x2` |
| `Hover_sub_429DC0` | `Column2x2Lit` |
| `Hover_sub_429E60` | `Column2x2Reg` |
| `Hover_sub_429EE0` | `Column4x2` |
| `Hover_sub_429F80` | `Column4x2Lit` |
| `Hover_sub_42A030` | `Column4x2Reg` |
| `Hover_sub_42A0B0` | `Column4x4` |
| `Hover_sub_42A170` | `Column4x4Lit` |
| `Hover_sub_42A240` | `Column4x4Reg` |
| `Hover_sub_42A2D0` | `Sky2x1` |
| `Hover_sub_42A330` | `Sky4x1` |
| `Hover_sub_42A390` | `Sky2x2` |
| `Hover_sub_42A400` | `Sky4x2` |
| `Hover_sub_42A470` | `Sky4x4` |
| `Hover_sub_434470` | `Span1x1_Var` |
| `Hover_sub_434500` | `Span1x1Lit_Var` |
| `Hover_sub_434590` | `Span1x1Lit_S6` |
| `Hover_sub_434600` | `Span1x1Lit_S7` |
| `Hover_sub_434670` | `Span1x1Lit_S8` |
| `Hover_sub_4346E0` | `Span1x1Lit_S15` |
| `Hover_sub_434750` | `Span1x1_S6` |
| `Hover_sub_4347B0` | `Span1x1_S7` |
| `Hover_sub_434810` | `Span1x1_S8` |
| `Hover_sub_434870` | `Span1x1_S9` |
| `Hover_sub_4348D0` | `Span1x1_S10` |
| `Hover_sub_434930` | `Span1x1_S11` |
| `Hover_sub_434990` | `Span1x1_S12` |
| `Hover_sub_4349F0` | `Span1x1_S13` |
| `Hover_sub_434A50` | `Span1x1_S14` |
| `Hover_sub_434AB0` | `Span1x1_S15` |
| `Hover_sub_434B10` | `Span2x1_Var` |
| `Hover_sub_434BB0` | `Span2x1Lit_Var` |
| `Hover_sub_434C60` | `Span2x1Lit_S6` |
| `Hover_sub_434CF0` | `Span2x1Lit_S7` |
| `Hover_sub_434D80` | `Span2x1Lit_S8` |
| `Hover_sub_434E10` | `Span2x1Lit_S15` |
| `Hover_sub_434EA0` | `Span2x1_S6` |
| `Hover_sub_434F30` | `Span2x1_S7` |
| `Hover_sub_434FC0` | `Span2x1_S8` |
| `Hover_sub_435050` | `Span2x1_S9` |
| `Hover_sub_4350E0` | `Span2x1_S10` |
| `Hover_sub_435170` | `Span2x1_S11` |
| `Hover_sub_435200` | `Span2x1_S12` |
| `Hover_sub_435290` | `Span2x1_S13` |
| `Hover_sub_435320` | `Span2x1_S14` |
| `Hover_sub_4353B0` | `Span2x1_S15` |
| `Hover_sub_435440` | `Span4x1_Var` |
| `Hover_sub_4354F0` | `Span4x1Lit_Var` |
| `Hover_sub_4355B0` | `Span4x1Lit_S6` |
| `Hover_sub_435650` | `Span4x1Lit_S7` |
| `Hover_sub_4356F0` | `Span4x1Lit_S15` |
| `Hover_sub_435790` | `Span4x1_S6` |
| `Hover_sub_435820` | `Span4x1_S7` |
| `Hover_sub_4358B0` | `Span4x1_S8` |
| `Hover_sub_435940` | `Span4x1_S9` |
| `Hover_sub_4359D0` | `Span4x1_S10` |
| `Hover_sub_435A60` | `Span4x1_S11` |
| `Hover_sub_435AF0` | `Span4x1_S12` |
| `Hover_sub_435B80` | `Span4x1_S13` |
| `Hover_sub_435C10` | `Span4x1_S14` |
| `Hover_sub_435CA0` | `Span4x1_S15` |

## WaveMix 2.3 (statically linked) and the game's sound thread (2026-10-14)

| Old | New |
|---|---|
| `SOUND_DRIVER_CFG` | `MIXCONFIG` |
| `tagSOUND_DRIVER_CFG` | `tagMIXCONFIG` |
| `SOUNDENGINE` | `MIXSESSION` |
| `tagSOUNDENGINE` | `tagMIXSESSION` |
| `MIXPLAYPARAM` | `MIXPLAYPARAMS` |
| `tagMIXPLAYPARAM` | `tagMIXPLAYPARAMS` |
| `Hover_sub_43D880` | `WaveMixConfigureInit` |
| `Hover_sub_4393A0` | `WaveMixOpenChannel` |
| `Hover_sub_43A760` | `WaveMixCloseChannel` |
| `Hover_sub_43DAB0` | `WaveMixCloseSession` |
| `Hover_sub_439180` | `WaveMixActivate` |
| `Hover_sub_43A5A0` | `WaveMixFlushChannel` |
| `Hover_sub_439A80` | `WaveMixPump` |
| `Hover_sub_439E80` | `WaveMixPlay` |
| `Hover_sub_43B150` | `WaveMixOpenWave` |
| `Hover_WaveMixFreeWave` | `WaveMixFreeWave` |
| `Hover_sub_43C8F0` | `WaveMixConfigure` |
| `Hover_sub_43CF50` | `WaveMixGetConfig` |
| `Hover_sub_438820` | `WaveMix_DebugOut` |
| `Hover_sub_43D3B0` | `WaveMix_ReadConfigSettings` |
| `Hover_sub_43B8F0` | `WaveMix_ShowWaveOutDevices` |
| `Hover_sub_438840` | `WaveMix_GetWaveOutPosition` |
| `Hover_sub_4388C0` | `WaveMix_AddFactor` |
| `Hover_sub_4388E0` | `WaveMix_SubFactor` |
| `Hover_sub_438900` | `WaveMix_SetWaveOutPosition` |
| `Hover_sub_4389F0` | `WaveMix_CMixit` |
| `Hover_sub_438CA0` | `WaveMix_InitChannelNodes` |
| `Hover_sub_438F50` | `WaveMix_FreeWaveOut` |
| `Hover_sub_439900` | `WaveMix_FreePlayedChannelNodes` |
| `Hover_sub_439010` | `WaveMix_OpenWaveOut` |
| `Hover_sub_439560` | `WaveMix_MixerPlay` |
| `Hover_sub_43B880` | `WaveMix_RemoveInvalidIniNameCharacters` |
| `Hover_sub_438D60` | `WaveMix_AllocWaveBlocks` |
| `Hover_sub_438D00` | `WaveMix_FreeWaveBlocks` |
| `Hover_sub_438E90` | `WaveMix_RemoveFromPlayingQueue` |
| `Hover_sub_43B810` | `WaveMix_GetWaveBlockLen` |
| `Hover_sub_43B9C0` | `WaveMix_GetOperatingSystemPrefix` |
| `Hover_sub_43BA10` | `WaveMix_ReadRegistryToGetMachineSpecificInfSection` |
| `Hover_sub_43BAD0` | `WaveMix_ReadRegistryInt` |
| `Hover_sub_43BB20` | `WaveMix_ReadRegistryForAppSpecificConfigs` |
| `Hover_sub_43BCF0` | `WaveMix_ShowCurrentSettings` |
| `Hover_sub_43BDF0` | `WaveMix_SettingsOnInitDialog` |
| `Hover_sub_43C1C0` | `WaveMix_SettingsOnCommand` |
| `Hover_sub_43C3C0` | `WaveMix_SettingsDlgProc` |
| `Hover_sub_43C420` | `WaveMix_MakeDlgTemplate` |
| `Hover_sub_43C4C0` | `WaveMix_AddDlgControl` |
| `Hover_sub_43C5A0` | `WaveMix_MakeSettingsDlgTemplate` |
| `Hover_sub_43C8C0` | `WaveMix_DestroySettingsDlgTemplate` |
| `Hover_sub_43D100` | `WaveMix_SaveConfigSettings` |
| `Hover_sub_43DB90` | `WaveMix_SetIniFileName` |
| `Hover_sub_43DBE0` | `WaveMix_InitVolumeTable` |
| `Hover_sub_43DC40` | `WaveMix_Startup` |
| `Hover_sub_439BF0` | `WaveMix_NoResetRemix` |
| `Hover_sub_439C20` | `WaveMix_ResetRemix` |
| `Hover_sub_439B60` | `WaveMix_WndProc` |
| `Hover_sub_438CD0` | `WaveMix_GetChannelNode` |
| `Hover_sub_439E40` | `WaveMix_ResetWavePosIfNoChannelData` |
| `Hover_sub_43AA70` | `WaveMix_BitsPerSampleAlign` |
| `Hover_sub_43ABC0` | `WaveMix_ChannelAlign` |
| `Hover_sub_43AD40` | `WaveMix_AvgSample` |
| `Hover_sub_43ADF0` | `WaveMix_RepSample` |
| `Hover_sub_43AEC0` | `WaveMix_SamplesPerSecAlign` |
| `Hover_sub_43B0A0` | `WaveMix_ConvertWaveFormat` |
| `Hover_wavemixDestroyQueue` | `WaveMix_DestroyPlayQueue` |
| `Hover_wavemixSwapBlocks` | `WaveMix_SwapWaveBlocks` |
| `Hover_wavemixGetBlock` | `WaveMix_GetWaveBlock` |
| `Hover_wavemixQueueBlock` | `WaveMix_AddToPlayingQueue` |
| `Hover_sub_42D6A0` | `Hover_SoundThreadProc` |
| `Hover_sub_42D760` | `Hover_OnWaveMixDone` |
| `g_csUnk_4A4808` | `g_csSoundQueue` |
| `g_pUnk_4C56DC` | `g_pMixSession` |
| `g_pThreadUnk_4C56E0` | `g_pSoundThread` |
| `g_dwUnk_4C56E4` | `g_bSoundQueueCsInit` |
| `g_dwUnk_4C56E8` | `g_bSoundThreadQuit` |
| `g_hUnk_4C56F4` | `g_hSoundWakeEvent` |
| `g_hUnk_4C56F8` | `g_hSoundAckEvent` |
| `g_dwUnk_4C56FC` | `g_bWaveBusyMsgShown` |
| `g_dwUnk_4C5AD4` | `g_dwPlayNesting` |
| `g_pvUnk_4C56EC` | `g_pvSoundOnce` |
| `g_pvUnk_4C56F0` | `g_pvSoundLoop` |
| `g_pUnk_4B6248` | `g_pCurSession` |
| `g_dwUnk_4B6F14` | `g_dwDefaultMixit` |
| `g_pUnk_4B6F18` | `g_pActiveSession` |
| `g_pUnk_4B6D50` | `g_pFreeChannelNodes` |
| `g_adwUnk_4B6E70` | `g_adwWaveBlocksA` |
| `g_adwUnk_4B6EC0` | `g_adwWaveBlocksB` |
| `g_hUnk_4B6F10` | `g_hWaveMixInst` |
| `g_apUnk_4B61C8` | `g_apMixChannelNodes` |
| `g_apUnk_4B6208` | `g_apMixSources` |
| `g_awUnk_4B6F20` | `g_awMixVolumes` |
| `g_adwUnk_4B4A58` | `g_adwChannelNodePool` |
| `g_aUnk_4B6250` | `g_abVolumeTable` |
| `g_szUnk_4B6D58` | `g_szIniFileName` |
| `g_dwUnk_4C59B8` | `g_bShowDebugDialogs` |
| `g_dwUnk_4C59BC` | `g_dwDebugLevel` |
| `g_pUnk_4C59C0` | `g_pPlayQueueHead` |
| `g_pUnk_4C59C4` | `g_pPlayQueueTail` |
| `g_dwUnk_4C59C8` | `g_bWaveMixInitialized` |
| `g_szUnk_4C59D0` | `g_szStringBuffer` |
| `g_dwUnk_4C56D0` | `g_bMixSessionOpen` |
| `g_dwUnk_4C56D4` | `g_bWaveOutAvailable` |
| `g_wfxUnk_4BFE30` | `g_wfxDefaultMix` |
| `g_mapUnk_4A4820` | `g_mapOpenWaveToSlot` |
| `tagMIXCONFIG::wUnk00` | `wSize` |
| `tagMIXCONFIG::dwUnk02` | `dwFlags` |
| `tagMIXCONFIG::wUnk06` | `wChannels` |
| `tagMIXCONFIG::wUnk08` | `wSamplingRate` |
| `tagMIXCONFIG::wUnk0A` | `wWaveBlockCount` |
| `tagMIXCONFIG::wUnk0C` | `wWaveBlockLen` |
| `tagMIXCONFIG::wUnk0E` | `wCmixPtrDefaultFlag` |
| `tagMIXCONFIG::wUnk10` | `wResetMixDefaultFlag` |
| `tagMIXCONFIG::wUnk12` | `wGoodWavePos` |
| `tagMIXCONFIG::wUnk14` | `wDeviceID` |
| `tagMIXCONFIG::wUnk16` | `wPauseBlocks` |
| `tagMIXCONFIG::wUnk18` | `wShowDebugDialogs` |
| `tagMIXCONFIG::dwUnk1A` | `dwRegistryKey` |
| `tagMIXSESSION::wMagic00` | `wMagic1` |
| `tagMIXSESSION::hwndUnk04` | `hWndCallback` |
| `tagMIXSESSION::dwUnk0C` | `dwWaveOut` |
| `tagMIXSESSION::dwUnk10` | `fActive` |
| `tagMIXSESSION::dwUnk14` | `fSettingsDialogActive` |
| `tagMIXSESSION::dwUnk18` | `dwDeviceID` |
| `tagMIXSESSION::szUnk1C` | `szDevicePName` |
| `tagMIXSESSION::capsUnk7C` | `woCaps` |
| `tagMIXSESSION::wUnkB0` | `wDefaultVolumeL` |
| `tagMIXSESSION::wUnkB2` | `wDefaultVolumeR` |
| `tagMIXSESSION::awUnkB4` | `awChannelVolume` |
| `tagMIXSESSION::adwUnkF4` | `adwChannel` |
| `tagMIXSESSION::dwUnk134` | `dwOpenChannels` |
| `tagMIXSESSION::adwUnk138` | `adwChannelMRU` |
| `tagMIXSESSION::dwUnk178` | `dwMRU` |
| `tagMIXSESSION::wfxUnk17C` | `pcm` |
| `tagMIXSESSION::dwUnk18C` | `dwWaveBlockLen` |
| `tagMIXSESSION::dwUnk190` | `dwWaveBlockCount` |
| `tagMIXSESSION::dwUnk194` | `dwPauseBlocks` |
| `tagMIXSESSION::dwUnk198` | `dwWaveBlockArray` |
| `tagMIXSESSION::dwUnk19C` | `dwCurrentSample` |
| `tagMIXSESSION::dwUnk1A0` | `dwBaseTime` |
| `tagMIXSESSION::dwUnk1A4` | `fGoodGetPos` |
| `tagMIXSESSION::dwUnk1A8` | `dwWaveOutPosDelta` |
| `tagMIXSESSION::dwUnk1AC` | `dwMixitFn` |
| `tagMIXSESSION::dwUnk1B0` | `dwRemixFn` |
| `tagMIXSESSION::dwUnk1B4` | `dwPosAdjustFn` |
| `tagMIXSESSION::dwUnk1B8` | `dwWaitList` |
| `tagMIXSESSION::wMagic1BC` | `wMagic2` |
| `tagMIXPLAYPARAMS::wVolume1` | `wVolumeL` |
| `tagMIXPLAYPARAMS::wVolume2` | `wVolumeR` |

## Step 4 retype: game-object pointers held in DWORDs (2026-10-14)

| Old | New | Type |
|---|---|---|
| `CPodObj::m_dwOwner` | `m_pOwner` | `CPlayer*` |
| `CPodObj::m_dwInner` | `m_pInner` | `CPodObj*` |
| `CPodObj::m_dwForceField` | `m_pForceField` | `CMerlinDynamic*` |
| `CPodObj::m_dwSpriteTexture` | `m_pSpriteTexture` | `CMerlinTexture*` |
| `CPodObj::m_dwNextFree` | `m_pNextFree` | `CPodObj*` |
| `CBumperDoc::m_dwCollider` | `m_pCollider` | `CCollider*` |
| `CBumperDoc::m_dwWorld` | `m_pWorld` | `CMerlinWorld*` |
| `CGameObj::m_dwMerlin` | `m_pMerlin` | `CMerlinLine*` |
| `CGameObj::m_dwRegionNodes` | `m_pRegionNodes` | `REGIONNODE*` |
| `CGameObj::m_dwNextStart` | `m_pNextStart` | `CGameObj*` |
| `CGameObj::m_dwNextEnd` | `m_pNextEnd` | `CGameObj*` |
| `CRobotPlayer::m_dwTarget` | `m_pTarget` | `CGameObj*` |
| `CRobotPlayer::m_adwRecent` | `m_apRecent` | `CGameObj*` |
| `CPlayer::m_dwInvinciblePod` | `m_pInvinciblePod` | `CInviciblePod*` |
| `CPlayer::m_dwSpeedPod` | `m_pSpeedPod` | `CSpeedPod*` |
| `CPlayer::m_dwSlowPod` | `m_pSlowPod` | `CSlowPod*` |
| `CPlayer::m_dwTempWallPod` | `m_pTempWallPod` | `CTempWallPod*` |
| `CPlayer::m_dwSmokeBombPod` | `m_pSmokeBombPod` | `CSmokeBombPod*` |
| `CPlayer::m_dwJumpPod` | `m_pJumpPod` | `CJumpPod*` |
| `CPlayer::m_dwHeadingFrame` | `m_pHeadingFrame` | `CMerlinTexture*` |
| `CPlayer::m_dwFrame` | `m_pFrame` | `CMerlinTexture*` |
| `CWall::m_dwNextFree` | `m_pNextFree` | `CWall*` |
| `CFloorObj::m_dwNextFree` | `m_pNextFree` | `CFloorObj*` |
| `CFloorObj::m_dwHitEdge` | `m_pHitEdge` | `CWall*` |
| `CBeacon::m_dwNextFree` | `m_pNextFree` | `CBeacon*` |
| `CMerlinDynamic::m_dwNextFree` | `m_pNextFree` | `CMerlinDynamic*` |
| `g_dwPodObjFree` | `g_pPodObjFree` | `CPodObj*` |
| `g_dwFloorObjFree` | `g_pFloorObjFree` | `CFloorObj*` |
| `g_dwWallFree` | `g_pWallFree` | `CWall*` |
| `g_dwRobotFree` | `g_pRobotFree` | `CRobotPlayer*` |
| `g_dwMerlinDynamicFree` | `g_pMerlinDynamicFree` | `CMerlinDynamic*` |
| `g_dwBeaconFree` | `g_pBeaconFree` | `CBeacon*` |
| `g_dwRegionNodeFree` | `g_pRegionNodeFree` | `REGIONNODE*` |
| `g_dwDirtyObjList` | `g_pDirtyObjList` | `CGameObj*` |
| `g_dwDrawObjList` | `g_pDrawObjList` | `CGameObj*` |

## Step 4 retype: view and renderer (2026-10-14)

| Old | New | Type |
|---|---|---|
| `CVelocityIndicator::m_dwBits` | `m_pBits` | `BYTE*` |
| `CSpanRasterizer::m_dwBits` | `m_pBits` | `BYTE*` |
| `CSegmentGauge::m_dwBrushOff` | `m_pBrushOff` | `CBrush*` |
| `CSegmentGauge::m_dwBrushOn` | `m_pBrushOn` | `CBrush*` |
| `CSegmentGauge::m_dwPen` | `m_pPen` | `CPen*` |
| `CBumperView::m_dwRenderThread` | `m_pRenderThread` | `CWinThread*` |
| `CBumperView::m_dwEvtCommand` | `m_hEvtCommand` | `HANDLE` |
| `CBumperView::m_dwOldBitmap` | `m_hOldBitmap` | `HBITMAP` |
| `CRenderContext::m_dwScanOrigin` | `m_pScanOrigin` | `BYTE*` |
| `tagRENDEROBJ::m_dwTopFlatTex` | `m_pTopFlatTex` | `CMerlinTexture*` |
| `tagRENDEROBJ::m_dwBottomFlatTex` | `m_pBottomFlatTex` | `CMerlinTexture*` |
| `tagFLATNODE::m_dwTex` | `m_pTex` | `CMerlinTexture*` |
| `tagSPANLINK::m_dwObj` | `m_pObj` | `CMerlinLine*` |
| `tagSPANLINK::m_dwSegment` | `m_pSegment` | `CMerlinLine*` |
| `tagMERLIN_TEXFRAME::m_dwPixels` | `m_pPixels` | `BYTE*` |
| `tagMERLIN_TEXFRAME::m_dwColumns` | `m_pColumns` | `struct tagTEXCOLUMN*` |
| `tagTEXRUN::m_dwPixels` | `m_pPixels` | `BYTE*` |
| `tagSPANENTRY::m_dwPixels` | `m_pPixels` | `BYTE*` |
| `CDigitDisplay::m_pStripBits` | `m_pStripBits` | `BYTE*` |
| `CSegmentGauge::m_pBrushOff` (named in this PR) | `m_pBrushFilled` | `CBrush*` (+0x10 draws the segments below the level) |
| `CSegmentGauge::m_pBrushOn` (named in this PR) | `m_pBrushEmpty` | `CBrush*` |
| `RENDEROBJ::m_dwSubKey` | `m_pSegment` | `CMerlinLine*` |
| `EDGENODE::m_pFlatTex` | `m_pFlatTex` | `CMerlinTexture*` (was `void*`) |

## Step 4 retype: app and sound handles (2026-10-14)

| Old | New | Type |
|---|---|---|
| `CHoverApp::m_dwDisplayDibWnd` | `m_hwndDisplayDib` | `HWND` |
| `CMainFrame::m_dwUnk00C4` | `m_hLibrary` | `HINSTANCE` |
| `CMidiStream::m_dwFile` | `m_hFile` | `HANDLE` |
| `CMidiStream::m_dwFreeHead` | `m_pFreeHead` | `MIDIHDR*` |
| `CMidiStream::m_dwFreeTail` | `m_pFreeTail` | `MIDIHDR*` |
| `CMidiStream::m_dwStream` | `m_hStream` | `HMIDISTRM` |
| `tagMIXSESSION::dwWaveOut` | `hWaveOut` | `HWAVEOUT` |
| `tagMIXSESSION::dwMixitFn` | `pfnMixit` | `MIXITFN` |
| `tagMIXSESSION::dwRemixFn` | `pfnRemix` | `REMIXFN` |
| `tagMIXSESSION::dwPosAdjustFn` | `pfnPosAdjust` | `POSADJUSTFN` |
| `tagMIXCONFIG::dwRegistryKey` | `hRegistryKey` | `HKEY` |
| `g_pvSoundOnce` | `g_pSoundOnce` | `CSoundSlot*` |
| `g_pvSoundLoop` | `g_pSoundLoop` | `CSoundSlot*` |
| `g_dwDefaultMixit` | `g_pfnDefaultMixit` | `MIXITFN` |
| `CSoundSlot::m_pMixWave` | `m_pMixWave` | `MIXWAVE*` |

## Step 4 struct merges: map points, map renderer rects and fixed fields, live padding (2026-10-14)

| Old | New |
|---|---|
| `CMapRenderer::m_dwUnk00/04/08/0C` | `RECT m_rcClip` |
| `CMapRenderer::m_dwUnk34 .. m_dwUnk58` | `m_bLeftAligned`, `m_bRightAligned`, `m_dwTopOffsetDw`, `m_dwTopOffset`, `m_dwFlipBottom`, `m_dwFlipTop`, `m_dwLeftEdge`, `m_dwRightEdge4`, `m_dwLeftAligned`, `m_dwRightAligned` |
| `CMapRenderer::m_dwUnk5C/60/64/68` | `RECT m_rcView` |
| `CMapRenderer::m_awUnk006C[0..9]` | `m_fxHeight`, `m_fxWidth`, `m_fxHalfHeight`, `m_fxHalfWidth`, `m_fxProjBase` (16.16 unions) |
| `CMapRenderer::m_wUnk0080/0082` | `m_fxScale` |
| `CMapRenderer::SetClipRect(dwBottom, dwRight, dwTop, dwRight2)` | `SetClipRect(dwLeft, dwTop, dwRight, dwBottom)` |
| `CBumperView::m_dwUnk0094..00A0` (union arm) | `m_rcMessage` only |
| `CPlayer::m_pad00A8[0/4/8]` | `m_bAirborne`, `m_dwSpawnHeight1`, `m_dwSpawnHeight2` |
| `CPlayer::m_pad0160` | `m_dwSledAngle` |
| `CPlayer::m_dwUnk0164..0178` | `POINT m_aptMapTri[3]` |
| `CPodObj::m_dwUnkC0/C4` | `POINT m_ptMap` |
| `CPodObj::m_dwUnkF0..FC` | `POINT m_ptMapWall1`, `m_ptMapWall2` |
| `CWall::m_dwUnk84..90` | `POINT m_ptMap1`, `m_ptMap2` |
| `CBeacon::m_dwUnk9C/A0` | `POINT m_ptMap` |

## Step 4 struct merges: beacon search frames, robot target heading, player probe (2026-10-14)

| Old | New |
|---|---|
| `CBeaconSearch::m_adwFrames[10]` | `BEACONFRAME m_aFrames[5]` (`{CBeacon* pBeacon; int nChild;}`) |
| `CBeaconSearch::m_pTop` (`DWORD*`) | `BEACONFRAME* m_pTop` |
| `CRobotPlayer::m_wUnk0200/0202` | `m_fxTargetHeading` (16.16 union; halves `m_wTargetHeadingFrac/Int`) |
| `CPlayer::m_dwUnk0180/0184` | `m_probe.ptFront` |
| `CPlayer::m_dwUnk0188/018C` | `m_probe.ptBack` |
| `CPlayer::m_dwProbeRadius` | `m_probe.lRadius` |
| `CPlayer::GetProbe()` (`DWORD*`) | `PLAYERPROBE* GetProbe()` |
| `CSegment::DistanceTo(const DWORD* pdwPt, ...)` | `DistanceTo(const POINT* pPt, ...)` |
| `CSegment::SideOf(const long* plPt)` | `SideOf(const POINT* pPt)` |

## Step 4 struct merges: WaveMix wave blocks and channel nodes (2026-10-14)

| Old | New |
|---|---|
| 0x2030-byte wave blocks as `DWORD*` | `XWAVEHDR` (`WAVEHDR wh`, `fAvailable`, `dwWavePos`, `pSession`, `pQNext`, `abData[0x2000]`) |
| 0x3C-byte channel nodes as `DWORD*` | `CHANNELNODE` (`pNext`, `PlayParams`, `lpMixWave`, `dwNumSamples`, `dwStartPos`, `dwEndPos`, `lpPos`, `lpEnd`, `wVolumeL/R`) |
| `tagMIXSESSION::adwChannel[0x10]` | `apChannel[0x10]` (`CHANNELNODE*`; `WMIX_CHANNEL_CLOSED` = -1) |
| `tagMIXSESSION::dwWaveBlockArray` | `ppWaveBlocks` (`XWAVEHDR**`) |
| `tagMIXSESSION::dwWaitList` | `pWaitList` (`CHANNELNODE*`) |
| `tagMIXWAVE::pData/cbData/dwUnk18..2C` | `WAVEHDR wh` (`lpData`, `dwBufferLength`, `dwUser`, `dwFlags`, `dwLoops`) |
| `g_adwWaveBlocksA/B` | `g_apWaveBlocksA/B` (`XWAVEHDR*[0x14]`) |
| `g_adwChannelNodePool` | `g_aChannelNodePool` (`CHANNELNODE[100]`) |
| `g_pFreeChannelNodes`, `g_apMixChannelNodes` | (unchanged names) `CHANNELNODE*` |
| `g_pPlayQueueHead/Tail` | (unchanged names) `XWAVEHDR*` |
| `WaveMix_MixerPlay(DWORD* pNode, ...)` | `WaveMix_MixerPlay(XWAVEHDR* pXWH, ...)` |
| `WaveMix_AllocWaveBlocks/FreeWaveBlocks(..., DWORD* pBlocks)` | `(..., XWAVEHDR** ppBlocks)` |
| `void* WaveMix_RemoveFromPlayingQueue(void*)` | `XWAVEHDR* WaveMix_RemoveFromPlayingQueue(XWAVEHDR*)` |
| `void* WaveMix_GetChannelNode()` | `CHANNELNODE* WaveMix_GetChannelNode()` |

## Step 5: stack-adjacent arguments and literal strides (2026-10-14)

| Old | New |
|---|---|
| `IsNear(DWORD dwA, DWORD dwB, DWORD dwC)` (5 overrides) | `IsNear(POINT pt, long lRadius)` |
| `CMapRenderer::SetClipRect(DWORD dwLeft, dwTop, dwRight, dwBottom)` | `SetClipRect(RECT rc)` |
| `CMapRenderer::SetViewport(..., DWORD dwLeft, dwTop, dwRight, dwBottom)` | `SetViewport(..., RECT rc)` |
| `CVelocityIndicator::CreateTile(..., int nLeft, nTop, nRight, DWORD nBottom)` | `CreateTile(..., RECT rc)` |
| `CRegionMatrix::m_pCells/m_pCellsEnd` (`DWORD*`) | `REGIONNODE**` |
| `CSpanRasterizer::m_pLinkPool` (`DWORD*`) | `SPANLINK*` |
| `CSpanRasterizer::m_pColumnLists` (`DWORD*`) | `SPANLINK**` |
| `g_pdwSpanLinkCursor` (`DWORD*`) | `g_pSpanLinkCursor` (`SPANLINK*`) |
| `CRenderContext::m_pRayDirs` (`DWORD*`, walked in pairs) | `SINCOSENTRY*` (struct moved to texheap.h) |
| `0x4A40A8 - 0x4A1D80` | `SPAN_ARENA_SIZE` |
| `0x7E9`, `0x1FA4` | `ROWDIST_ENTRIES`, `ROWDIST_ENTRIES * sizeof(DWORD)` |
| `0x428` (BITMAPINFO allocations) | `BMI8_SIZE` |
| `DWORD adwMsg[3]` (BeginDispDib) | `COPYDATASTRUCT cds` |

## Audit leftovers: readability (2026-10-14)

| Old | New |
|---|---|
| `CHoverApp::m_adwUnk0378[13]` | `JOYINFOEX m_joyInfo` (`dwSize`, `dwFlags`, `dwXpos`, `dwYpos`, `dwButtons`) |
| `0x408` (LOGPALETTE allocations) | `LOGPAL256_SIZE` |
| `g_szIniFileName[0x100]` | `g_szIniFileName[MAX_PATH]` (GetModuleFileName is told MAX_PATH) |
| `0x104` path lengths | `MAX_PATH` |
| Robot steering locals `f.dwThrottle` / `f.dwSteerVal` / `f.dwNegSteer` (swapped) | `f.dwTurnCtl` / `f.dwThrottle` / `f.dwNegThrottle` |
| 84 stale placeholder names in comments (`m_dwUnkXX`, `Sub_XXXXXX`, `CSubXXXXXX`) | current names; the 5 left are deliberate "formerly called" notes |
