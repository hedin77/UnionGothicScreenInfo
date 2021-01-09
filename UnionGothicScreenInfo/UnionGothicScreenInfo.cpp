// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

	// Add your code here . . .
	#define playerIsDead player->attribute[NPC_ATR_HITPOINTS] <= 0

	struct DamageText { 
		string damage;
		int currentTime;
		oCNpc* targetEnemy;
		int lastY; 
		int correctionShiftByY;
		int startPosX;
	};

	struct BarParams {
		int x; int y;  int h; int w;
	};

	enum DamageMode
	{
		DAMAGE_ABOVE_ENEMY, DAMAGE_ON_SCREEN
	};

	static Array<DamageText> damages;
	static Timer mainTimer;
	static int bNeedShowEnemyNameAbobeBar;
	static bool needShowEnemyName = false;

	const int AST_TIMER_FPS_SHOW = 0;
	const int AST_TIMER_TIME_SHOW = 1;
	const int AST_TIMER_QE = 2;
	const int AST_TIMER_TIMER = 3;
	const int DAMAGE_TIMER = 4;
	const float enemyBarScaleMin = 0.5;
	const float enemyBarScaleMax = 1.0;
	const int damageTotalTime = 2000;
	const int delay = 50;

	BarParams playerHealth; 
	BarParams playerMana;
	zCView* screenMiscShowDate = NULL;
	zCView* screenSpeedInfo = NULL;
	zCView* screenAddInfo = NULL;
	zCView* screenMunitionInfo = NULL;
	int bshowEnemyHealth;
	int bshowPlayerHealthAndMana;
	int bshowDate;
	string sDay;
	string playerHealthName;
	string playerManaName;
	string playerHealthBarParams;
	string playerManaBarParams;
	int playerHealthNamePos;
	int playerManaNamePos;
	int bNeedShowBarAboveEnemy; 
	int bMunitionInfo;
	int xBarEnemy; 
	int yBarEnemy;
	zCOLOR enemyTextColor;
	int bNeedSpeedMode;
	Array<double> speedModeMults;
	int currentIndexSpeedMult; 
	zCOLOR mutionColorInfo;
	int munitionPosX;
	int munitionPosY;
	int bNeedShowDamageInfo;
	zCView* damageView;	
	int damagePosX;
	int damagePosY;
	zCOLOR damageColorInfo;	
	int speedWorldKeyId;
	bool damageShowZero; 
	int barSizeOriginalX = 0;
	int barSizeOriginalY = 0;
	float baseScaleEnemyBar = 1; 	
	static DamageMode damageMode;
	static int damageStartPosX;
	static int damageStartPosY;
	static const int POSSIBLE_SHIFT_BY_X = 350; 
	static const int POSSIBLE_SHIFT_BY_Y = 150;

	void MultParamToDoubleArray(zSTRING str, Array<double> arraysMult) {
		bool oneFound = false; 
		Array<string> arraysString = CStringA(str).Split(",");
		int len = arraysString.GetNum();
		for (int ii = 0; ii < len; ii++) {
			double param = atof(arraysString.GetSafe(ii)->ToChar());
			arraysMult.InsertSorted(param);
			if (param == 1.0) {
				oneFound = true;
			}
		}
		if (!oneFound) {
			arraysMult.InsertSorted(1.0);
		}
	}

	void MultParamToIntArray(zSTRING str, Array<int> arraysMult) {
		Array<string> arraysString = CStringA(str).Split(",");
		int len = arraysString.GetNum();
		for (int ii = 0; ii < len; ii++) {
			int param = atoi(arraysString.GetSafe(ii)->ToChar());
			arraysMult.InsertEnd(param);
		}
	}


	zSTRING getBarParams(oCViewStatusBar* bar) {
		return Z(bar->vposx) + "," + Z(bar->vposy) + "," + Z(bar->vsizex) + "," + Z(bar->vsizey);
	}

	void initOptions() {		
		zSTRING defaultHealthName = "HP";
		zSTRING defaultManaName = "MP";

		switch (Union.GetSystemLanguage())
		{
			case Lang_Rus: defaultHealthName = "Жизнь"; defaultManaName = "Мана"; break;
			case Lang_Eng: defaultHealthName = "Health"; defaultManaName = "Mana";  break;
			case Lang_Ger: defaultHealthName = "Gesundheit"; defaultManaName = "Mana";  break;
			case Lang_Pol: defaultHealthName = "Zdrowie"; defaultManaName = "Mana";  break;
			default: break;
		}

		zSTRING speedWorldKey = zoptions->ReadString("show_additional_info", "speedWorldKey", "Z");
		if (speedWorldKey.Compare("F")) {
			speedWorldKeyId = KEY_F;
		}
		else {
			speedWorldKeyId = KEY_Z;
		}

		bNeedSpeedMode = zoptions->ReadInt("show_additional_info", "bNeedSpeedMode", TRUE);
		zSTRING param = zoptions->ReadString("show_additional_info", "speedModeMult", "1,4");
		MultParamToDoubleArray(param, speedModeMults);

		bshowDate = zoptions->ReadInt("show_additional_info", "bShowDate", TRUE);
		sDay = A zoptions->ReadString("show_additional_info", "sDay", "День:");

		bMunitionInfo = zoptions->ReadInt("show_additional_info", "bMunitionInfo", TRUE);	
		int munitionColorR = zoptions->ReadInt("show_additional_info", "MunitionColorR", 51);
		int munitionColorG = zoptions->ReadInt("show_additional_info", "MunitionColorG", 204);
		int munitionColorB = zoptions->ReadInt("show_additional_info", "MunitionColorB", 51);
		mutionColorInfo = zCOLOR(munitionColorR, munitionColorG, munitionColorB);
		munitionPosX = zoptions->ReadInt("show_additional_info", "MunitionPosX", 1);
		munitionPosY = zoptions->ReadInt("show_additional_info", "MunitionPosY", 750);


		bNeedShowDamageInfo = zoptions->ReadInt("show_additional_info", "bNeedShowDamageInfo", TRUE);
		int damageColorR = zoptions->ReadInt("show_additional_info", "damageColorR", 255);
		int damageColorG = zoptions->ReadInt("show_additional_info", "damageColorG", 105);
		int damageColorB = zoptions->ReadInt("show_additional_info", "damageColorB", 0);
		damageColorInfo = zCOLOR(damageColorR, damageColorG, damageColorB);
		damageShowZero = zoptions->ReadInt("show_additional_info", "damageShowZero", TRUE);
		int dmode = zoptions->ReadInt("show_additional_info", "damageMode", 0);
		damageMode = dmode == 0 ? DAMAGE_ABOVE_ENEMY : DAMAGE_ON_SCREEN;
		damageStartPosX = zoptions->ReadInt("show_additional_info", "damageStartPosX", 5500);
		damageStartPosY = zoptions->ReadInt("show_additional_info", "damageStartPosY", 4000);


		currentIndexSpeedMult = 0; 
		bNeedShowBarAboveEnemy = zoptions->ReadInt("show_additional_info", "bNeedShowBarAboveEnemy", TRUE);
		bshowEnemyHealth = zoptions->ReadInt("show_additional_info", "bShowEnemyHealth", TRUE);

		bshowPlayerHealthAndMana = zoptions->ReadInt("show_additional_info", "bshowPlayerHealthAndMana", TRUE);
		playerHealthName = A zoptions->ReadString("show_additional_info", "playerHealthName", defaultHealthName);
		playerManaName = A zoptions->ReadString("show_additional_info", "playerManaName", defaultManaName);
		playerHealthNamePos = zoptions->ReadInt("show_additional_info", "playerHealthNamePosMode", 0);
		playerManaNamePos = zoptions->ReadInt("show_additional_info", "playerManaNamePosMode", 0);

		baseScaleEnemyBar = zoptions->ReadReal("show_additional_info", "baseScaleEnemyBar", 1.0);
		baseScaleEnemyBar = max(0.5, baseScaleEnemyBar);
		baseScaleEnemyBar = min(2.0, baseScaleEnemyBar);
		bNeedShowEnemyNameAbobeBar = zoptions->ReadInt("show_additional_info", "bNeedShowEnemyNameAbobeBar", FALSE);


	}

	void ShowZTime() {
		if (bshowDate && !(playerIsDead)) {
				if (mainTimer[AST_TIMER_TIME_SHOW].Await(100)) {
					int day, hour, min, nFont;
					ogame->GetTime(day, hour, min);
					nFont = screenMiscShowDate->FontY();
					zCOLOR textColor = zCOLOR(245, 247, 225);
					screenMiscShowDate->SetFontColor(textColor);
					if (min >= 10) {
						screenMiscShowDate->Print(1, nFont + 100, sDay + " " + A(day + 1) + " " + A(hour) + ":" + A(min));
					}
					else {
						screenMiscShowDate->Print(1, nFont + 100, sDay + " " + A(day + 1) + " " + +A(hour) + ":0" + A(min));
					}
				}
		}

	}

	
	float getScaleMult() {
		if (player->GetFocusNpc()) {
			zCVob* focusVob = player->GetFocusVob();
			float dist = player->GetDistanceToVob(*focusVob);
			float scaleMult = 1 - (dist * (enemyBarScaleMin / 3000));
			scaleMult = max(enemyBarScaleMin, scaleMult);
			scaleMult = min(enemyBarScaleMax, scaleMult);
			return scaleMult;
		}
		else { 
			return 1;
		}
	}

	void MunitionLoop() {
		if (!bMunitionInfo)
			return;

		if (!player || playerIsDead) {
			screenMunitionInfo->ClrPrintwin();
			return;
		}

		oCItem* gun = player->GetEquippedRangedWeapon();

		if (!gun)
			gun = player->GetWeapon();

		if (!gun || ((gun->mainflag & ITM_CAT_FF) != ITM_CAT_FF) || gun->munition <= 0)
			return;

		player->inventory2.UnpackAllItems();
		oCItem* munition = player->IsInInv(gun->munition, 1);

		zSTRING text = munition ? (munition->description + Z": " + Z munition->amount) : Z"No munition available";
		screenMunitionInfo->Print(munitionPosX, munitionPosY, text);
	}


	void showBar(oCViewStatusBar* bar, int mode, zSTRING name) {
		int sizeX = bar->vsizex;
		int sizeY = bar->vsizey;
		int posX = bar->vposx;
		int posY = bar->vposy;
		int addByX = (sizeX / 2 - (screenAddInfo->FontSize(name) / 2));
		int addByY = screenAddInfo->FontY();
		int x;
		int y;

		if (mode == 0) { // above
			x = posX + addByX;
			y = posY - sizeY;
		}
		else if (mode == 1) {//under
			x = posX + addByX;
			y = posY + sizeY;
		}
		else if (mode == 2) { //right
			x = posX + sizeX;
			y = posY + sizeY/2 - addByY/2;
		}
		else if (mode == 3) { //left
			x = posX - screenAddInfo->FontSize(name);
			y = posY + sizeY / 2 - addByY / 2;
		}

		// default
		if (x < 0 || x > 8128 || y < 0 || y > 8128) {
			x = posX + addByX; 
			y = posY - addByY;
		}
		screenAddInfo->Print(x, y, name);
	}

	void ResetEnemyBarData(oCViewStatusBar* focusBar) {
		xBarEnemy = -500;
		yBarEnemy = -500;
		if (focusBar) {
			focusBar->vposx = -500;
			focusBar->vposy = -500;
		}
	}

	void ShowEnemyData() {
		needShowEnemyName = false;
		const int fSize = screenAddInfo->FontY();
		const int fSizeHalf = screenAddInfo->FontY() / 2;

		if (bshowEnemyHealth) {
			oCViewStatusBar* focusBar = ogame->focusBar;
			oCNpc* focusNpc = player->GetFocusNpc();

			if (bNeedShowBarAboveEnemy && focusBar) {
				if (focusBar) {
					focusBar->vposx = -500;
					focusBar->vposy = -500;
				}

				if (!focusNpc) {
					ResetEnemyBarData(focusBar);
					return;
				}
				else {
					zVEC3 npcPosition = focusNpc->GetPositionWorld() + zVEC3(0.0f, 0.0f, 0.0f);
					zCCamera* cam = ogame->GetCamera();
					zVEC3 viewPos = cam->GetTransform(zTCamTrafoType::zCAM_TRAFO_VIEW) * npcPosition;
					int x, y;
					cam->Project(&viewPos, x, y);
					if (viewPos[2] <= cam->nearClipZ) {
						ResetEnemyBarData(focusBar);
						return;
					}
				}

				float normScale = getScaleMult();
				float scaleMult = normScale * baseScaleEnemyBar;
				if (barSizeOriginalX == 0) {
					barSizeOriginalX = focusBar->vsizex;
					barSizeOriginalY = focusBar->vsizey;
				}			
				focusBar->vsizex = barSizeOriginalX * scaleMult;
				focusBar->vsizey = barSizeOriginalY * scaleMult;
				focusBar->vposx = xBarEnemy + (screen->FontSize(focusNpc->name[0]) / 2) - (focusBar->vsizex) / 2;
				focusBar->vposy = bNeedShowEnemyNameAbobeBar ? yBarEnemy - fSize : yBarEnemy - 2 * fSize;
			}

			if (focusBar) {
				int focusX = focusBar->vposx;
				int focusY = focusBar->vposy;	
				int focusSize = focusBar->vsizey;
				int y1;
				int y2;

				if (bNeedShowBarAboveEnemy) {
					if (bNeedShowEnemyNameAbobeBar) {
						y1 = focusY - fSize - fSizeHalf;
						y2 = focusY - 2 * fSize - fSizeHalf;
					}
					else {
						y1 = focusY - fSize - fSizeHalf;
						y2 = focusY + focusSize/2 + fSize;
					}
				}
				else {
					y1 = focusY +  focusSize + screenAddInfo->FontY() / 2; 
				}

				if (focusNpc && focusNpc->attribute[NPC_ATR_HITPOINTS] > 0) {
					zSTRING npcName = focusNpc->name[0];
					int hp = focusNpc->attribute[NPC_ATR_HITPOINTS];
					int hpMax = focusNpc->attribute[NPC_ATR_HITPOINTSMAX];
					zSTRING str = zSTRING(hp) + "/" + zSTRING(hpMax);
					int x = focusX + (focusBar->vsizex) / 2 - (screenAddInfo->FontSize(str) / 2);
  					screenAddInfo->Print(
						x, 
						bNeedShowEnemyNameAbobeBar ? y2 : y1, 
						str
					);

					if (bNeedShowBarAboveEnemy) {
						zCOLOR oldColor = screenAddInfo->fontColor;
						screenAddInfo->fontColor = enemyTextColor;
						needShowEnemyName = true;
						screenAddInfo->Print(
							x + (screen->FontSize(str) / 2) - (screen->FontSize(npcName)) / 2,
							bNeedShowEnemyNameAbobeBar ? y1 : y2,
							npcName
						);
						needShowEnemyName = false;
						screenAddInfo->fontColor = oldColor;
					}
				}
			}
		}
	}

	void ShowPlayerData() {

		if (bshowPlayerHealthAndMana) {
			oCViewStatusBar* hpBar = ogame->hpBar;
			oCViewStatusBar* manaBar = ogame->manaBar;
			if (hpBar && manaBar) {
				zSTRING hp = playerHealthName + ": " + A player->attribute[NPC_ATR_HITPOINTS] + "/" +  A player->attribute[NPC_ATR_HITPOINTSMAX];
				zSTRING mana = playerManaName + ": " + A player->attribute[NPC_ATR_MANA] + "/" + A player->attribute[NPC_ATR_MANAMAX];		
				showBar(hpBar, playerHealthNamePos, hp);
				showBar(manaBar, playerManaNamePos, mana);
			}
		}
	}


	void checkSpeedMode() {
		if (bNeedSpeedMode) {
			if(zinput->KeyToggled(speedWorldKeyId)){
				currentIndexSpeedMult = (currentIndexSpeedMult + 1) % speedModeMults.GetNum();
				int nFont = screenSpeedInfo->FontY();
				if (speedModeMults[currentIndexSpeedMult] != 1.0) {
					screenSpeedInfo->Print(1, nFont * 2 + 200, "Speed mode:  x" 
						+ zSTRING(speedModeMults[currentIndexSpeedMult]));
				}
				else {
					screenSpeedInfo->ClrPrintwin();
				}
			}
			ztimer->factorMotion = (float) speedModeMults[currentIndexSpeedMult];
		}
	}



	void ToolsLoadEnd() {	

		if (screenSpeedInfo) {
			screenSpeedInfo->ClrPrintwin();
		}
		if (ztimer){
			ztimer->factorMotion = 1.0;
		}
		
		if (!damages.IsEmpty()) {
			damages.Clear();			
		}

	}

	void checkViews() {
		if (!screenMiscShowDate) {
			screenMiscShowDate = zNEW(zCView)(0, 0, 8192, 8192);
			screen->InsertItem(screenMiscShowDate);
		}

		if (!screenSpeedInfo) {
			screenSpeedInfo = zNEW(zCView)(0, 0, 8192, 8192);
			screen->InsertItem(screenSpeedInfo);
		}
		
		if (!screenAddInfo) {
			screenAddInfo = zNEW(zCView)(0, 0, 8192, 8192);
			screen->InsertItem(screenAddInfo);
		}

		if (!screenMunitionInfo) {
			screenMunitionInfo = zNEW(zCView)(0, 0, 8192, 8192);
			screenMunitionInfo->SetFontColor(mutionColorInfo);
			screen->InsertItem(screenMunitionInfo);
		}

		if (!damageView) {
			damageView = zNEW(zCView)(0, 0, 8192, 8192);
			damageView->SetFontColor(damageColorInfo);
			screen->InsertItem(damageView);
		}

		screenAddInfo->ClrPrintwin();
		damageView->ClrPrintwin();
	}

	int getFreePosX() {
		int res = damageStartPosX;
		int cnt = 0;
		bool freePosFound; 
		int shiftXRight = 0;
		int shiftXLeft = 0;
		int currentShift; 

		while (cnt++ < 500) {
			freePosFound = true; 
			if (cnt % 2 == 0) {
				currentShift = shiftXRight;
				shiftXRight += POSSIBLE_SHIFT_BY_X;
			}
			else {
				currentShift = shiftXLeft;
				shiftXLeft -= POSSIBLE_SHIFT_BY_X;
			}

			int len = damages.GetNum();
			for (uint i = 0; i < len; i++) {
				DamageText* dmg = damages.GetSafe(i);
				int currentPos = damageStartPosX + currentShift; 
				bool allowByX = dmg->startPosX <= currentPos - POSSIBLE_SHIFT_BY_X 	||  dmg->startPosX >= currentPos + POSSIBLE_SHIFT_BY_X;
				if ((damageStartPosY - dmg->lastY < POSSIBLE_SHIFT_BY_Y) && !allowByX) {
					freePosFound = false;
				}
			}

			if (freePosFound) {
				res = damageStartPosX + currentShift;
				break;
			}
		}
		return res; 	
	}

	void  DamageLoop()
	{
		if (bNeedShowDamageInfo && damages.GetNum() > 0) {

			int tick; 
			damageView->ClrPrintwin();
			
			if (mainTimer[DAMAGE_TIMER].Await(10)) {
				for (uint i = 0; i < damages.GetNum(); i++) {
					DamageText* dt = damages.GetSafe(i);
					dt->currentTime += 10;
					if (dt->currentTime >= damageTotalTime) {
						damages.RemoveAt(i);
						i--;
					}
				}
			}

			int cntDamages = damages.GetNum();

			for (uint i = 0; i < damages.GetNum(); i++) {
				DamageText* dmg = damages.GetSafe(i);
				oCNpc* foundNpc = dmg->targetEnemy;
				tick = 1.7 * dmg->currentTime;
				if(foundNpc && damageMode == DAMAGE_ABOVE_ENEMY) {
					int shiftX = min(tick / 3, 80);
					zVEC3 npcPosition = foundNpc->GetPositionWorld() + zVEC3(0.0f, 130.0f, 0.0f);
					zCCamera* cam = ogame->GetCamera();
					zVEC3 viewPos = cam->GetTransform(zTCamTrafoType::zCAM_TRAFO_VIEW) * npcPosition;
					int x, y;
					cam->Project(&viewPos, x, y);
					if (viewPos[2] > cam->nearClipZ) {
						if ((y - dmg->lastY) > 0) {
							dmg->correctionShiftByY = dmg->correctionShiftByY + y - dmg->lastY;
						}
						int currentX = screen->anx(x + 0.5f) + shiftX;
						int currentY = screen->any(y + 0.5f - dmg->correctionShiftByY) - tick;
						damageView->Print(currentX, currentY, dmg->damage);
						dmg->lastY = y;
					}
				} else if(damageMode == DAMAGE_ON_SCREEN) {
					dmg->lastY = damageStartPosY - tick;
					damageView->Print(dmg->startPosX, dmg->lastY, dmg->damage);
				}
			}
		}

	}

	void updateBarParams(BarParams* barparam, zSTRING params) {		
		Array<int> paramsArray;
		MultParamToIntArray(params, paramsArray);
		barparam->x =  *paramsArray.GetSafe(0);
		barparam->y =  *paramsArray.GetSafe(1);
		barparam->w =  *paramsArray.GetSafe(2);
		barparam->h =  *paramsArray.GetSafe(3);
	}

	void updateBar(oCViewStatusBar* bar, BarParams barparam) {
		bar->vposx = barparam.x;
		bar->vposy = barparam.y;
		bar->vsizex = barparam.w;
		bar->vsizey = barparam.h;
	}

	void MovePlayerBars() {
		oCViewStatusBar* hpBar = ogame->hpBar;
		oCViewStatusBar* manaBar = ogame->manaBar;
		if (hpBar) {
			if (playerHealthBarParams.IsEmpty()) {
				playerHealthBarParams = A zoptions->ReadString("show_additional_info", "playerHealthBarParams", getBarParams(hpBar));
				updateBarParams(&playerHealth, playerHealthBarParams);
			}
			updateBar(hpBar, playerHealth);
		}
		
		
		if (manaBar) {
			if (playerManaBarParams.IsEmpty()) {
				playerManaBarParams = A zoptions->ReadString("show_additional_info", "playerManaBarParams", getBarParams(manaBar));
				updateBarParams(&playerMana, playerManaBarParams);
			}
			updateBar(manaBar, playerMana);
		}

	}

	void MainLoop() {		
		checkViews();

		if (player) {
			MovePlayerBars();
		}

		if (!ogame || !player || playerIsDead
			|| !oCInformationManager::GetInformationManager().HasFinished() 
			|| ogame->pause_screen || ogame->singleStep) {


			oCViewStatusBar* focusBar = ogame->focusBar;
			if (bNeedShowBarAboveEnemy && focusBar) {
				ResetEnemyBarData(focusBar);
			}

			if (bNeedSpeedMode) {
				for (uint ii = 0; ii < speedModeMults.GetNum(); ii++) {
					if (speedModeMults[ii] == 1.0) {
						currentIndexSpeedMult = ii;
						ztimer->factorMotion = 1.0;
						screenSpeedInfo->ClrPrintwin();
					}
				}
			}
			return;
		}
	
		MunitionLoop();
		ShowPlayerData();
		ShowEnemyData();
		checkSpeedMode();
		ShowZTime();	
		DamageLoop();
		return;	
	}
}