// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
	// Add your code here . . .

    HOOK Ivk_Print_Hedin   PATCH_IF(&zCView::Print, &zCView::Print_Hedin, true);
    void zCView::Print_Hedin(int x, int y, const zSTRING& text) {
        if (player)
        {
            oCNpc* focusNpc = player->GetFocusNpc();
            if (focusNpc && infoFinished) {
                if (focusNpc->attribute[NPC_ATR_HITPOINTS] <= 0) {
                    THISCALL(Ivk_Print_Hedin)(x, y, text);
                    return;
                }
                string s1 = (A text).Replace(A"\n", A"").Shrink();
                string s2 = (A focusNpc->name[0]).Replace(A"\n", A"").Shrink();

                if (bNeedShowBarAboveEnemy && (s1.Compare(s2))) {
                    if (!needShowEnemyName) {
                        if (y < 600) {
                            y = 600;
                        }
                        yBarEnemy = y;
                        xBarEnemy = x;
                        enemyTextColor = this->fontColor;
                    }

                    if (needShowEnemyName)
                        THISCALL(Ivk_Print_Hedin)(x, y, text);
                    return;
                }
            }
        }
        THISCALL(Ivk_Print_Hedin)(x, y, text);
    }
  

    struct NpcHP
    {
        int hp; 
        oCNpc* npc; 
    };


    static Array<NpcHP> newNpc;

    HOOK Ivk_OnDamage_Hit_Hedin   PATCH_IF(&oCNpc::OnDamage_Hit, &oCNpc::OnDamage_Hit_Hedin, true);
    void oCNpc::OnDamage_Hit_Hedin(oSDamageDescriptor& damageDescriptor) {
        if (bNeedShowDamageInfo) {
            bool playerIsDamager = damageDescriptor.pNpcAttacker == player;
            int oldTargetHP = this->GetAttribute(NPC_ATR_HITPOINTS);
            if (playerIsDamager) {
                NpcHP npcHP;
                npcHP.hp = oldTargetHP;
                npcHP.npc = this;
                newNpc.Insert(npcHP);
            }
        }
        THISCALL(Ivk_OnDamage_Hit_Hedin)(damageDescriptor);
    }
    
    void processNewNpc() {
        if (!bNeedShowDamageInfo)
            return;

        int cntNpc = newNpc.GetNum();
        for (int idx = 0; idx < cntNpc; idx++) {
            NpcHP* npcHP = newNpc.GetSafe(idx);
            int oldHP = npcHP->hp; 
            oCNpc* target = npcHP->npc;
            int newHP = target->GetAttribute(NPC_ATR_HITPOINTS);
            if (bNeedShowDamageInfo) {
                int realDamage = oldHP - newHP;
                if (realDamage > 0 || damageShowZero) {
                    DamageText dmg;
                    dmg.currentTime = 0;
                    dmg.damage = A(realDamage);
                    if (damageMode == DAMAGE_ABOVE_ENEMY) {
                        zVEC3 npcPosition = target->GetPositionWorld() + zVEC3(0.0f, 130.0f, 0.0f);
                        zCCamera* cam = ogame->GetCamera();
                        zVEC3 viewPos = cam->GetTransform(zTCamTrafoType::zCAM_TRAFO_VIEW) * npcPosition;
                        int x, y;
                        cam->Project(&viewPos, x, y);
                        dmg.targetEnemy = target;
                        dmg.correctionShiftByY = 0;
                        dmg.lastY = y;
                    }
                    else if (damageMode == DAMAGE_ON_SCREEN)
                    {
                        dmg.startPosX = getFreePosX();
                        dmg.lastY = damageStartPosY;
                    }
                    damages.Insert(dmg);
                }
            }
        }
        newNpc.Clear();
    }
}