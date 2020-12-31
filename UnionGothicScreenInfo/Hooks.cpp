// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {
	// Add your code here . . .

    void __fastcall View_Print(zCView* _this, void* vt, int x, int y, zSTRING& text);
    CInvoke <void(__thiscall*) (zCView*, int, int, zSTRING&)> pView_Print(ZenDef<TInstance>(0x006FFEB0, 0x0073A4B0, 0x00749F40, 0x007A9A40),
        &View_Print, IvkEnabled(ENGINE));
    void __fastcall View_Print(zCView* _this, void* vt, int x, int y, zSTRING& text)
    {

        if (player)
        {
            oCNpc* focusNpc = player->GetFocusNpc();
            if (focusNpc) {
                 string s1 = (A text).Replace(A"\n", A"").Shrink();
                 string s2 = (A focusNpc->name[0]).Replace(A"\n", A"").Shrink();

                 if (bNeedShowBarAboveEnemy && (s1.Compare(s2))) {
                    xBarEnemy = x;
                    if (y < 500) {
                        y = 500;
                    }
                    yBarEnemy = y;
                }
            }
        }
        pView_Print(_this, x, y, text);
    }

    
    void __fastcall OnDamage_Hit(oCNpc*, void*, oCNpc::oSDamageDescriptor&);
    CInvoke <void(__thiscall*) (oCNpc*, oCNpc::oSDamageDescriptor& damage)> pOnDamage_Hit(ZenDef<TInstance>(0x00731410, 0x007700A0, 0x0077D390, 0x00666610)
        , &OnDamage_Hit, IvkEnabled(ENGINE));

    void __fastcall OnDamage_Hit(oCNpc* target, void*, struct oCNpc::oSDamageDescriptor& damage) {
        bool playerIsDamager = damage.pNpcAttacker == player; 
        int oldTargetHP = target->GetAttribute(NPC_ATR_HITPOINTS);
        pOnDamage_Hit(target, damage);

        if (bNeedShowDamageInfo) {
            int realDamage = oldTargetHP - target->GetAttribute(NPC_ATR_HITPOINTS);
            if (playerIsDamager && (realDamage > 0 || damageShowZero)) {
                DamageText dmg;
                dmg.currentTime = 0;
                dmg.damage = Z(realDamage);
                dmg.targetEnemy = target;
                damages.Insert(dmg);
            }
        }
    }
}