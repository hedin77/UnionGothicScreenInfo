// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

	// Add your code here . . .
	#define playerIsDead player->attribute[NPC_ATR_HITPOINTS] <= 0

	struct DamageText { 
		zSTRING damage;
		int currentTime;
		oCNpc* targetEnemy;
	};

	struct BarParams {
		int x; int y;  int h;int w;
	};

	BarParams playerHealth; 
	BarParams playerMana;

	static int AST_TIMER_FPS_SHOW = 0;
	static int AST_TIMER_TIME_SHOW = 1;
	static int AST_TIMER_QE = 2;
	static int AST_TIMER_TIMER = 3;
	static int DAMAGE_TIMER = 4;
	static zCView* screenMiscShowDate = NULL;
	static zCView* screenSpeedInfo = NULL;
	static zCView* screenAddInfo = NULL;
	static zCView* screenMunitionInfo = NULL;
	static Timer mainTimer;
	static int bshowEnemyHealth;
	static int bshowPlayerHealthAndMana;
	static int bshowDate;
	static zSTRING sDay;
	static zSTRING playerHealthName;
	static zSTRING playerManaName;
	static zSTRING playerHealthBarParams;
	static zSTRING playerManaBarParams;
	static int playerHealthNamePos;
	static int playerManaNamePos;
	static int bNeedShowBarAboveEnemy; 
	static int bMunitionInfo;
	static int xBarEnemy; 
	static int yBarEnemy;
	static int bNeedSpeedMode;
	static Array<double> speedModeMults;
	static int currentIndexSpeedMult; 
	static zCOLOR mutionColorInfo;
	static int munitionPosX;
	static int munitionPosY;
	
	static int bNeedShowDamageInfo;
	zCView* damageView;	
	static 	int damagePosX;
	static 	int damagePosY;
	static zCOLOR damageColorInfo;
	static Array<DamageText> damages;
	static int damageTotalTime = 2000;
	static int delay = 50;
	static int speedWorldKeyId;
	static bool damageShowZero; 
	static int barSizeOriginalX = -1;
	static int barSizeOriginalY = -1;
	const float enemyBarScaleMin = 0.5;
	const float enemyBarScaleMax = 1.0;

	void splitToStringArray(zSTRING str, Array<string> arraysMult, bool needSort) {
		char delimiter = ',';
		int idx = 0;
		bool oneFound = false;
		char* strChars = str.ToChar();
		int len = strlen(strChars);
		string tempNumber = string();

		for (int ii = 0; ii < len; ii++) {
			char current = strChars[ii];
			if (current == delimiter) {
				if(needSort)
					arraysMult.InsertSorted(tempNumber);
				else
					arraysMult.InsertEnd(tempNumber);
				tempNumber = string();
			}
			else {
				tempNumber = tempNumber + string(strChars[ii]);
				if (ii == len - 1) {
					if(needSort)
						arraysMult.InsertSorted(atof(tempNumber));
					else 
						arraysMult.InsertEnd(atof(tempNumber));
				}
			}
		}
	}

	void MultParamToDoubleArray(zSTRING str, Array<double> arraysMult) {
		bool oneFound = false; 
		Array<string> arraysString; 
		splitToStringArray(str, arraysString, true);
		int len = arraysString.GetNum();
		for (int ii = 0; ii < len; ii++) {
			double param = atof(arraysString.GetSafe(ii)->ToChar());
			arraysMult.InsertSorted(param);
			if (param == 1.0) {
				oneFound = true;
			}
		}
		if (!oneFound) {
			arraysMult.Insert(1.0);
		}
	}

	void MultParamToIntArray(zSTRING str, Array<int> arraysMult) {
		Array<string> arraysString;
		splitToStringArray(str, arraysString, false);
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
		sDay = zoptions->ReadString("show_additional_info", "sDay", "День:");

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

		currentIndexSpeedMult = 0; 

		bNeedShowBarAboveEnemy = zoptions->ReadInt("show_additional_info", "bNeedShowBarAboveEnemy", TRUE);
		bshowEnemyHealth = zoptions->ReadInt("show_additional_info", "bShowEnemyHealth", TRUE);

		bshowPlayerHealthAndMana = zoptions->ReadInt("show_additional_info", "bshowPlayerHealthAndMana", TRUE);
		playerHealthName = zoptions->ReadString("show_additional_info", "playerHealthName", "Жизнь");
		playerManaName = zoptions->ReadString("show_additional_info", "playerManaName", "Мана");
		playerHealthNamePos = zoptions->ReadInt("show_additional_info", "playerHealthNamePosMode", 0);
		playerManaNamePos = zoptions->ReadInt("show_additional_info", "playerManaNamePosMode", 0);

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
						screenMiscShowDate->Print(1, nFont + 100, sDay + " " + zSTRING(day + 1) + " " + zSTRING(hour) + ":" + zSTRING(min));
					}
					else {
						screenMiscShowDate->Print(1, nFont + 100, sDay + " " + zSTRING(day + 1) + " " + +zSTRING(hour) + ":0" + zSTRING(min));
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

	void MoveEnemyBar() {
		oCNpc* focusNpc = player->GetFocusNpc();
		if (focusNpc)
		{

			oCViewStatusBar* focusBar = ogame->focusBar;
			if (barSizeOriginalX == -1) {
				barSizeOriginalX = focusBar->vsizex;
				barSizeOriginalY = focusBar->vsizey;				
			}
			
			float scaleMult = getScaleMult();
			focusBar->vposx = xBarEnemy + (screen->FontSize(focusNpc->name[0]) / 2) - (focusBar->vsizex) / 2;
			focusBar->vposy = yBarEnemy - 250;
			focusBar->vsizex = barSizeOriginalX * scaleMult;
			focusBar->vsizey = barSizeOriginalY * scaleMult;

		}
		else {
			xBarEnemy = -500;
			yBarEnemy = -500;
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

	void ShowEnemyAndHealthAndMana() {
		if (bshowEnemyHealth && bNeedShowBarAboveEnemy) {
			MoveEnemyBar();
		}

		if (bshowPlayerHealthAndMana) {
			oCViewStatusBar* hpBar = ogame->hpBar;
			oCViewStatusBar* manaBar = ogame->manaBar;
			if (hpBar && manaBar) {
				zSTRING hp = playerHealthName + ": " + (zSTRING)player->attribute[NPC_ATR_HITPOINTS] + "/" + (zSTRING)player->attribute[NPC_ATR_HITPOINTSMAX];
				zSTRING mana = playerManaName + ": " + (zSTRING)player->attribute[NPC_ATR_MANA] + "/" + (zSTRING)player->attribute[NPC_ATR_MANAMAX];		
				showBar(hpBar, playerHealthNamePos, hp);
				showBar(manaBar, playerManaNamePos, mana);
			}
		}

		if (bshowEnemyHealth) {
			oCViewStatusBar* focusBar = ogame->focusBar;
			if (focusBar) {
				int focusX = focusBar->vposx;
				int focusY = focusBar->vposy;
				int focusSize = focusBar->vsizey;
				int  focusPSizeX = focusBar->psizex;

				float scaleMult = getScaleMult();
				float addY = (1.5 * enemyBarScaleMin / scaleMult);

				int y = bNeedShowBarAboveEnemy ? focusY - (focusSize * (0.3 + addY)) : focusY + (focusSize * 1.4);


				oCNpc* npc = player->GetFocusNpc();
				if (npc && !(playerIsDead)) {
					zSTRING npcName = npc->name[0];
					int hp = npc->attribute[NPC_ATR_HITPOINTS];
					int hpMax = npc->attribute[NPC_ATR_HITPOINTSMAX];
					zSTRING str = zSTRING(hp) + "/" + zSTRING(hpMax);
					int x = focusX + (focusBar->vsizex) / 2 - (screenAddInfo->FontSize(str) / 2);
					screenAddInfo->Print(x, y, str);
				}
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
				tick = 1.7 * dmg->currentTime;
				int shiftY = min(tick/3, 80);

				oCNpc* foundNpc = dmg->targetEnemy;

				if (foundNpc) {
					zVEC3 npcPosition = foundNpc->GetPositionWorld() + zVEC3(0.0f, 130.0f, 0.0f);
					zCCamera* cam = ogame->GetCamera();
					zVEC3 viewPos = cam->GetTransform(zTCamTrafoType::zCAM_TRAFO_VIEW) * npcPosition;
					int x, y;
					cam->Project(&viewPos, x, y);

					if (viewPos[2] > cam->nearClipZ) {
						int currentX = screen->anx(x + 0.5f) +  shiftY;
						int currentY = screen->any(y + 0.5f) - tick;
						damageView->Print(currentX, currentY, dmg->damage);
					}
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
				playerHealthBarParams = zoptions->ReadString("show_additional_info", "playerHealthBarParams", getBarParams(hpBar));
				updateBarParams(&playerHealth, playerHealthBarParams);
			}
			updateBar(hpBar, playerHealth);
		}
		
		
		if (manaBar) {
			if (playerManaBarParams.IsEmpty()) {
				playerManaBarParams = zoptions->ReadString("show_additional_info", "playerManaBarParams", getBarParams(manaBar));
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
		ShowEnemyAndHealthAndMana();		
		checkSpeedMode();
		ShowZTime();	
		DamageLoop();
		return;	
	}
}