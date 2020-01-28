#if defined _nexd_included_ 
  #endinput 
#endif 
#define _nexd_included_
#define PREFIX "\x01[\x0BRendszer\x01]" //Used for my things, change it if you want
#define empty "\0"
#define m_iMaxVelocity 9999999.0
#define MAX_WEAPONS 48
#define INTEGER_STRING_LENGTH 20
#define GAMEUNITS_TO_METERS 0.01905
#define LoopValidClients(%1) for(int %1 = 1; %1 <= MaxClients; %1++) if(IsValidClient(%1))
#define EF_BONEMERGE                (1 << 0)
#define EF_NOSHADOW                 (1 << 4)
#define EF_NORECEIVESHADOW          (1 << 6)
#define EPOOL "ABCDEFGHIJKLMNOPQRSTXYZVW"

#include <clients>
#include <sdktools>
#include <cstrike>
#include <clientprefs>
#undef REQUIRE_PLUGIN
#include <crypt>
#include <progressbar>
#define REQUIRE_PLUGIN

#pragma tabsize 0;

stock float[] GetConVar2DVector(ConVar convar)
{
	float vectors[2] = {0.0, 0.0};

	char sBuffer[128];
	convar.GetString(sBuffer, sizeof(sBuffer));

	if (strlen(sBuffer) == 0)
		return vectors;

	char sPart[2][12];
	int iReturned = ExplodeString(sBuffer, StrContains(sBuffer, ", ") != -1 ? ", " : " ", sPart, 2, sizeof(sPart[]));

	for (int i = 0; i < iReturned; i++)
		vectors[i] = StringToFloat(sPart[i]);

	return vectors;
}

stock float FloatDivider(float value, float multiplier)
{
	return value *= 1.0 - multiplier;
}

stock float GetEntitiesDistance(int entity1, int entity2)
{
	if(!IsValidEntity(entity1) || !IsValidEntity(entity2)) return 100.0;
	if(!HasEntProp(entity1, Prop_Send, "m_vecOrigin") || !HasEntProp(entity2, Prop_Send, "m_vecOrigin")) return 100.0;
	
	float fOrigin1[3];
	GetEntPropVector(entity1, Prop_Send, "m_vecOrigin", fOrigin1);

	float fOrigin2[3];
	GetEntPropVector(entity2, Prop_Send, "m_vecOrigin", fOrigin2);
	
	return GetVectorDistance(fOrigin1, fOrigin2);
}

stock bool IsClientCrouching(int client)
{
	if(GetEntityFlags(client) & FL_DUCKING)
		return true;

	return false;
}

stock bool IsClientIsOnGround(int client)
{
	if(GetEntityFlags(client) & FL_ONGROUND)
		return true;

	return false;
}

stock int GetClientFromSteamID(const char[] id)
{
	char steamid[20];
	for (int i = 1; i <= MaxClients; ++i)
	{
		if(!IsValidClient(i))
			continue;
		
		GetClientAuthId(i, AuthId_Steam2, steamid, sizeof(steamid));
		if(StrEqual(steamid, id))
			return i;
	}

	return -1;
}

stock void RemovePlayerKnife(int client) {
    int fegya;
    for (int i = 0; i < 6; i++)
    {
		fegya = GetPlayerWeaponSlot(client, i);
		if(IsValidEntity(fegya) && i == 2)
		{
			RemovePlayerItem(client, fegya);
		}
	}
}

stock void RemoveAllWeapon(int client) {
    int fegya;
    for (int i = 0; i < 6; i++)
    {
		fegya = GetPlayerWeaponSlot(client, i);
		if(IsValidEntity(fegya))
		{
			RemovePlayerItem(client, fegya);
		}
	}
}

stock void RemovePlayerPistol(int client) {
    int fegya;
    for (int i = 0; i < 6; i++)
    {
		fegya = GetPlayerWeaponSlot(client, i);
		if(IsValidEntity(fegya) && i == 1)
		{
			RemovePlayerItem(client, fegya);
		}
	}
}

stock void RemovePlayerPrimary(int client) {
    int fegya;
    for (int i = 0; i < 6; i++)
    {
		fegya = GetPlayerWeaponSlot(client, i);
		if(IsValidEntity(fegya) && i == 0)
		{
			RemovePlayerItem(client, fegya);
		}
	}
}

stock GetLegacyAuthString(int client, char[] out, int maxlen, bool validate = true)
{
	char m_szSteamID[32];
	bool success = GetClientAuthId(client, AuthId_Steam2, m_szSteamID, sizeof(m_szSteamID), validate);

	if(m_szSteamID[0]=='[')
	{
		new m_unAccountID = StringToInt(m_szSteamID[5]);
		new m_unMod = m_unAccountID % 2;
		Format(out, maxlen, "STEAM_0:%d:%d", m_unMod, (m_unAccountID-m_unMod)/2);
	} else {
		strcopy(out, maxlen, m_szSteamID);
	}

	return success;
}

stock bool IsInteger(const char[] buffer)
{
    for (int i = 0; i < strlen(buffer); i++)
    {
        if ( !IsCharNumeric(buffer[i]) )
            return false;
    }

    return true;    
}

stock char IntToStr(int thing)
{
	char itemindex[10];
	IntToString(thing, itemindex, sizeof(itemindex));
	return itemindex;
}

stock bool HasWeapon(int client, const char[] className)
{
	return (Client_GetWeapon(client, className) != INVALID_ENT_REFERENCE);
}

stock Client_GetWeapon(int client, const char[] className)
{
	int offset = Client_GetWeaponsOffset(client) - 4;
	int weapon = INVALID_ENT_REFERENCE;
	for (int i = 0; i < MAX_WEAPONS; i++) {
		offset += 4;

		weapon = GetEntDataEnt2(client, offset);

		if (!Weapon_IsValid(weapon)) {
			continue;
		}

		if (Entity_ClassNameMatches(weapon, className)) {
			return weapon;
		}
	}

	return INVALID_ENT_REFERENCE;
}

stock int Weapon_GetPrimaryAmmoType(int weapon)
{
	return GetEntProp(weapon, Prop_Data, "m_iPrimaryAmmoType");
}

stock int Weapon_GetSecondaryAmmoType(int weapon)
{
	return GetEntProp(weapon, Prop_Data, "m_iSecondaryAmmoType");
}

stock void DisablePlugin(char[] pluginname)
{
	char sPath[64];
	BuildPath(Path_SM, sPath, sizeof(sPath), "plugins/%s.smx", pluginname);
	if (FileExists(sPath))
	{
		char sNewPath[64];
		BuildPath(Path_SM, sNewPath, sizeof(sNewPath), "plugins/disabled/%s.smx", pluginname);

		ServerCommand("sm plugins unload %s", pluginname);

		if(FileExists(sNewPath))
		{
			DeleteFile(sNewPath);
		}
		RenameFile(sNewPath, sPath);

		LogMessage("%s was unloaded and moved to %s to avoid conflicts", sPath, sNewPath);
	}
}

stock bool Entity_IsPlayer(int entity)
{
	if (entity < 1 || entity > MaxClients) {
		return false;
	}
	
	return true;
}

stock int GetEventAttacker(Event event)
{
	return event.GetInt("attacker");
}

stock int GetEventVictim(Event event)
{
	return event.GetInt("userid");
}

stock bool Entity_Kill(kenny, killChildren=false)
{
	if (Entity_IsPlayer(kenny)) {
		ForcePlayerSuicide(kenny);
		return true;
	}
	
	if(killChildren){
		return AcceptEntityInput(kenny, "KillHierarchy");
	}
	else {
		return AcceptEntityInput(kenny, "Kill");
	}
}

stock Client_SetActiveWeapon(int client, int weapon)
{
	SetEntPropEnt(client, Prop_Data, "m_hActiveWeapon", weapon);
	ChangeEdictState(client, FindDataMapInfo(client, "m_hActiveWeapon"));
}

stock Client_SetWeaponPlayerAmmoEx(int client, int weapon, int primaryAmmo=-1, int secondaryAmmo=-1)
{
	new offset_ammo = FindDataMapInfo(client, "m_iAmmo");

	if (primaryAmmo != -1) {
		new offset = offset_ammo + (Weapon_GetPrimaryAmmoType(weapon) * 4);
		SetEntData(client, offset, primaryAmmo, 4, true);
	}

	if (secondaryAmmo != -1) {
		new offset = offset_ammo + (Weapon_GetSecondaryAmmoType(weapon) * 4);
		SetEntData(client, offset, secondaryAmmo, 4, true);
	}
}

stock Client_RemoveAllWeapons(int client, const char[] exclude="", bool clearAmmo=false)
{
	new offset = Client_GetWeaponsOffset(client) - 4;
	
	new numWeaponsRemoved = 0;
	for (new i=0; i < MAX_WEAPONS; i++) {
		offset += 4;

		new weapon = GetEntDataEnt2(client, offset);
		
		if (!Weapon_IsValid(weapon)) {
			continue;
		}
		
		if (exclude[0] != '\0' && Entity_ClassNameMatches(weapon, exclude)) {
			Client_SetActiveWeapon(client, weapon);
			continue;
		}
		
		if (clearAmmo) {
			Client_SetWeaponPlayerAmmoEx(client, weapon, 0, 0);
		}

		if (RemovePlayerItem(client, weapon)) {
			Entity_Kill(weapon);
		}

		numWeaponsRemoved++;
	}
	
	return numWeaponsRemoved;
}

stock Weapon_IsValid(int weapon)
{
	if (!IsValidEdict(weapon)) {
		return false;
	}

	return Entity_ClassNameMatches(weapon, "weapon_", true);
}

stock int Client_GetWeaponsOffset(int client)
{
	static offset = -1;

	if (offset == -1) {
		offset = FindDataMapInfo(client, "m_hMyWeapons");
	}

	return offset;
}

stock int Client_GetActiveWeaponName(int client, char[] buffer, int size)
{
	new weapon = Client_GetActiveWeapon(client);
	
	if (weapon == INVALID_ENT_REFERENCE) {
		buffer[0] = '\0';
		return INVALID_ENT_REFERENCE;
	}
	
	Entity_GetClassName(weapon, buffer, size);
	
	return weapon;
}

stock bool Weapon_IsReloading(int weapon)
{
	return view_as<bool>(GetEntProp(weapon, Prop_Data, "m_bInReload"));
}

stock Entity_IsValid(int entity)
{
	return IsValidEntity(entity);
}

stock Client_GetActiveWeapon(int client)
{
	new weapon =  GetEntPropEnt(client, Prop_Data, "m_hActiveWeapon");
	
	if (!Entity_IsValid(weapon)) {
		return INVALID_ENT_REFERENCE;
	}
	
	return weapon;
}

stock Weapon_GetOwner(int weapon)
{
	return GetEntPropEnt(weapon, Prop_Data, "m_hOwner");
}

stock bool Entity_ClassNameMatches(int entity, const char[] className, bool partialMatch = false)
{
	char entity_className[64];
	Entity_GetClassName(entity, entity_className, sizeof(entity_className));

	if (partialMatch) {
		return (StrContains(entity_className, className) != -1);
	}

	return StrEqual(entity_className, className);
}

stock Entity_GetClassName(int entity, char[] buffer, int size)
{
	return GetEntPropString(entity, Prop_Data, "m_iClassname", buffer, size);
}

stock int GetAliveClientsCount()
{
	int j = 0;
	for (int i = 1; i <= MaxClients; i++)
	{
		if(IsClientInGame(i) && IsPlayerAlive(i) && !IsFakeClient(i))
		{
			j++;
		}
	}
	return j;
}

stock int GetAliveClient()
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if(IsClientInGame(i) && IsPlayerAlive(i) && !IsFakeClient(i))
		{
			return i;
		}
	}
	return 0;
}

stock int GetAliveClientFromTeam(int team)
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if(IsClientInGame(i) && IsPlayerAlive(i) && !IsFakeClient(i) && GetClientTeam(i) == team)
		{
			return i;
		}
	}
	return 0;
}

stock bool HasAccess(int client, int flag, bool command = true)
{
	return (CheckCommandAccess(client, "dummytext", flag, command));
}

stock bool IsSilenced(int wix)
{
	return view_as<bool>(GetEntProp(wix, Prop_Send, "m_bSilencerOn"));
}

stock int GetBotFromTeam(int team)
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if(IsClientConnected(i) && IsClientInGame(i) && IsPlayerAlive(i) && IsFakeClient(i) && GetClientTeam(i) == team)
		{
			return i;
		}
	}

	return 0;
}

stock int GetBotCountTeam(int team)
{
	int count = 0;
	for (int i = 1; i <= MaxClients; i++)
	{
		if(IsClientConnected(i) && IsClientInGame(i) && IsPlayerAlive(i) && IsFakeClient(i) && GetClientTeam(i) == team)
		{
			count++;
		}
	}

	return count;
}

stock int GetAliveClientsCountTeam(int team)
{
	int j = 0;
	for (int i = 1; i <= MaxClients; i++)
	{
		if(IsClientConnected(i) && IsClientInGame(i) && GetClientTeam(i) == team && IsPlayerAlive(i) && !IsFakeClient(i))
		{
			j++;
		}
	}
	return j;
}

stock int GetClientsCountTeam(int team)
{
	int j = 0;
	for (int i = 1; i <= MaxClients; i++)
	{
		if(IsValidClient(i) && GetClientTeam(i) == team)
		{
			j++;
		}
	}
	return j;
}

stock int GetInGameClientCount()
{
	int j = 0;
	for (int i = 1; i <= MaxClients; i++)
	{
		if(IsClientInGame(i) && !IsFakeClient(i) && GetClientTeam(i) > 1)
		{
			j++;
		}
	}
	return j;
}

stock int NGetClientsCount()
{
	int j = 0;
	for (int i = 1; i <= MaxClients; i++)
	{
		if(IsClientInGame(i) && !IsFakeClient(i))
		{
			j++;
		}
	}
	return j;
}

stock int GetRealClientCount()
{
	int count = 0;
	for (int i = 1; i <= MaxClients; i++)
	{
		if(IsValidClient(i)) count++;
	}

	return count;
}

stock char GetCvarString(ConVar cvar)
{
	char string[255];
	cvar.GetString(string, sizeof(string));
	return string;
}

stock int Client_GetMoney(int client)
{
	return GetEntProp(client, Prop_Send, "m_iAccount");
}

stock void Client_SetMoney(int client, int value)
{
	SetEntProp(client, Prop_Send, "m_iAccount", value);
}

/**
 * Returns the round count.
 *
 * @return Round count.
 */
stock int GetRoundCount()
{
	return GameRules_GetProp("m_totalRoundsPlayed");
}

/**
 * Warmup check
 *
 * @return True if warmup, false otherwise
 */
stock bool IsWarmup()
{
	return view_as<bool>(GameRules_GetProp("m_bWarmupPeriod"));
}

/**
 * Freezetime check
 *
 * @return True if freezetime, false otherwise
 */
stock bool IsFreezeTime()
{
	return view_as<bool>(GameRules_GetProp("m_bFreezePeriod"));
}

/**
 * Bomb planted check
 *
 * @return True if planted, false otherwise
 */
stock bool IsBombPlanted()
{
	return view_as<bool>(GameRules_GetProp("m_bBombPlanted"));
}

/**
 * Bomb dropped check
 *
 * @return True if dropped, false otherwise
 */
stock bool IsBombDropped()
{
	return view_as<bool>(GameRules_GetProp("m_bBombDropped"));
}

/**
 * Returns a random player
 *
 * @return Random valid client index.
 */
stock int GetRandomPlayer()
{
	int[] jatekosok = new int[MaxClients+1];
	int jatekosszam;
	int randomjatekos;

	for(int i = 1; i <= MaxClients; i++)
	{
	    if(!IsValidClient(i)) continue;

	    jatekosok[jatekosszam++] = i;
	}

	randomjatekos = jatekosok[GetRandomInt(0, jatekosszam-1)];
	return randomjatekos;
}

stock bool TRDontHitSelf(int entity, int mask, any data) {
    if (entity == data)
        return false;
    return true;
}

stock Handle FindNamedCookie(const char[] cookieName) {
	Handle cookie = FindClientCookie(cookieName);
	if (cookie == null) {
		cookie = RegClientCookie(cookieName, "", CookieAccess_Protected);
	}
	return cookie;
}

stock void SetCookieStringByName(int client, const char[] cookieName, const char[] value) {
	Handle cookie = FindNamedCookie(cookieName);
	SetClientCookie(client, cookie, value);
	delete cookie;
}

stock void GetCookieStringByName(int client, const char[] cookieName, char[] buffer, int length) {
	Handle cookie = FindNamedCookie(cookieName);
	GetClientCookie(client, cookie, buffer, length);
	delete cookie;
}

stock void SetCookieIntByName(int client, const char[] cookieName, int value) {
	char buffer[INTEGER_STRING_LENGTH];
	IntToString(value, buffer, sizeof(buffer));
	SetCookieStringByName(client, cookieName, buffer);
}

stock int GetCookieIntByName(int client, const char[] cookieName) {
	char buffer[INTEGER_STRING_LENGTH];
	GetCookieStringByName(client, cookieName, buffer, sizeof(buffer));
	return StringToInt(buffer);
}

stock void SetCookieBoolByName(int client, const char[] cookieName, bool value) {
	int convertedInt = value ? 1 : 0;
	SetCookieIntByName(client, cookieName, convertedInt);
}

stock bool GetCookieBoolByName(int client, const char[] cookieName) {
	return GetCookieIntByName(client, cookieName) != 0;
}

stock void SetCookieInt(int client, Handle cookie, int value) {
	char buffer[INTEGER_STRING_LENGTH];
	IntToString(value, buffer, sizeof(buffer));
	SetClientCookie(client, cookie, buffer);
}

stock int GetCookieInt(int client, Handle cookie, int defaultValue = 0) {
	char buffer[INTEGER_STRING_LENGTH];
	GetClientCookie(client, cookie, buffer, sizeof(buffer));
	if (StrEqual(buffer, "")) {
		return defaultValue;
	}

	return StringToInt(buffer);
}

stock void SetCookieBool(int client, Handle cookie, bool value) {
	int convertedInt = value ? 1 : 0;
	SetCookieInt(client, cookie, convertedInt);
}

stock bool GetCookieBool(int client, Handle cookie, bool defaultValue = false) {
	return GetCookieInt(client, cookie, defaultValue) != 0;
}

stock char IntVersionToString(int version)
{
	char verStr[16];
	Format(verStr, sizeof(verStr), "%i", version);
	ReplaceString(verStr, sizeof(verStr), "0", ".");
	return verStr;
}

stock bool DrawLaser(char[] name, float startX, float startY, float startZ, float endX, float endY, float endZ, char[] color)
{
    int index = CreateEntityByName("env_beam");

    if (index != -1)
    {
        float start[3];
        start[0] = startX;
        start[1] = startY;
        start[2] = startZ;

        float end[3];
        end[0] = endX;
        end[1] = endY;
        end[2] = endZ;

        if (!IsModelPrecached("materials/sprites/laserbeam.vmt")) PrecacheModel("materials/sprites/laserbeam.vmt");

        DispatchKeyValue(index, "targetname", name);
        DispatchKeyValue(index, "spawnflags", "1");
        DispatchKeyValue(index, "texture", "sprites/laserbeam.spr");
        DispatchKeyValue(index, "life", "0");
        DispatchKeyValue(index, "BoltWidth", "2.5");
        DispatchKeyValue(index, "rendercolor", color);
        DispatchKeyValue(index, "renderamt", "255");
        SetEntityModel(index, "sprites/laserbeam.vmt");
        TeleportEntity(index, start, NULL_VECTOR, NULL_VECTOR);
        DispatchSpawn(index);
        SetEntPropVector(index, Prop_Data, "m_vecEndPos", end);
        AcceptEntityInput(index, "TurnOn");

        return true;
    }

    return false;
}

stock char GetRandomChar(const char[] pool)
{
	char sPool[255];
	Format(sPool, sizeof(sPool), pool);
	return sPool[GetRandomInt(0, strlen(pool)-1)];
}

/**
 * Knife check
 *
 * @param knifename classname
 * @return True if knife, false otherwise
 */
stock bool IsThisKnife(char[] knifename)
{
	if(IsStrContains(knifename, "knife") || 
    	IsStrContains(knifename, "bayonet"))
    	return true;

	return false;
}

stock bool IsSMG(char[] weapon)
{
	return (IsStrContains(weapon, "mp9") ||
			IsStrContains(weapon, "p90") ||
			IsStrContains(weapon, "mac10") ||
			IsStrContains(weapon, "ump45") ||
			IsStrContains(weapon, "mp7") ||
			IsStrContains(weapon, "mp5sd") ||
			IsStrContains(weapon, "bizon"))
}

stock bool IsHeavy(char[] weapon)
{
	return (IsStrContains(weapon, "nova") ||
			IsStrContains(weapon, "xm1014") ||
			IsStrContains(weapon, "mag7") ||
			IsStrContains(weapon, "negev") ||
			IsStrContains(weapon, "sawedoff"))
}

stock bool IsPistol(char[] weapon)
{
	return (IsStrContains(weapon, "deagle") ||
			IsStrContains(weapon, "usp_silencer") ||
			IsStrContains(weapon, "hkp2000") ||
			IsStrContains(weapon, "glock") ||
			IsStrContains(weapon, "elite") ||
			IsStrContains(weapon, "p250") ||
			IsStrContains(weapon, "cz75a") ||
			IsStrContains(weapon, "fiveseven") ||
			IsStrContains(weapon, "tec9") ||
			IsStrContains(weapon, "revolver"))
}

stock bool IsRifle(char[] weapon)
{
	return (IsStrContains(weapon, "awp") ||
			IsStrContains(weapon, "ak47") ||
			IsStrContains(weapon, "m4a1") ||
			IsStrContains(weapon, "famas") ||
			IsStrContains(weapon, "galilar") ||
			IsStrContains(weapon, "ssg08") ||
			IsStrContains(weapon, "sg556") ||
			IsStrContains(weapon, "aug") ||
			IsStrContains(weapon, "scar20") ||
			IsStrContains(weapon, "g3sg1"))
}

stock char GetWeaponNameFromClass(char[] class)
{
	char name[32];
	if(StrEqual(class, "weapon_awp")) Format(name, sizeof(name), "AWP");
	else if(StrEqual(class, "weapon_ak47")) Format(name, sizeof(name), "AK-47");
	else if(StrEqual(class, "weapon_m4a1")) Format(name, sizeof(name), "M4A4");
	else if(StrEqual(class, "weapon_m4a1_silencer")) Format(name, sizeof(name), "M4A1-S");
	else if(StrEqual(class, "weapon_deagle")) Format(name, sizeof(name), "Desert Eagle");
	else if(StrEqual(class, "weapon_usp_silencer")) Format(name, sizeof(name), "USP-S");
	else if(StrEqual(class, "weapon_hkp2000")) Format(name, sizeof(name), "HKP-2000");
	else if(StrEqual(class, "weapon_glock")) Format(name, sizeof(name), "Glock-18");
	else if(StrEqual(class, "weapon_elite")) Format(name, sizeof(name), "Dual Berettas");
	else if(StrEqual(class, "weapon_p250")) Format(name, sizeof(name), "P250");
	else if(StrEqual(class, "weapon_cz75a")) Format(name, sizeof(name), "CZ75-Auto");
	else if(StrEqual(class, "weapon_fiveseven")) Format(name, sizeof(name), "Five-SeveN");
	else if(StrEqual(class, "weapon_tec9")) Format(name, sizeof(name), "Tec-9");
	else if(StrEqual(class, "weapon_revolver")) Format(name, sizeof(name), "R8 Revolver");
	else if(StrEqual(class, "weapon_nova")) Format(name, sizeof(name), "Nova");
	else if(StrEqual(class, "weapon_xm1014")) Format(name, sizeof(name), "XM1014");
	else if(StrEqual(class, "weapon_mag7")) Format(name, sizeof(name), "MAG-7");
	else if(StrEqual(class, "weapon_sawedoff")) Format(name, sizeof(name), "Sawed-Off");
	else if(StrEqual(class, "weapon_m249")) Format(name, sizeof(name), "M429");
	else if(StrEqual(class, "weapon_negev")) Format(name, sizeof(name), "Negev");
	else if(StrEqual(class, "weapon_mp9")) Format(name, sizeof(name), "MP9");
	else if(StrEqual(class, "weapon_mac10")) Format(name, sizeof(name), "MAC-10");
	else if(StrEqual(class, "weapon_mp7")) Format(name, sizeof(name), "MP7");
	else if(StrEqual(class, "weapon_ump45")) Format(name, sizeof(name), "UMP-45");
	else if(StrEqual(class, "weapon_p90")) Format(name, sizeof(name), "P90");
	else if(StrEqual(class, "weapon_bizon")) Format(name, sizeof(name), "PP-Bizon");
	else if(StrEqual(class, "weapon_famas")) Format(name, sizeof(name), "FAMAS");
	else if(StrEqual(class, "weapon_galilar")) Format(name, sizeof(name), "Galil AR");
	else if(StrEqual(class, "weapon_ssg08")) Format(name, sizeof(name), "SSG 08");
	else if(StrEqual(class, "weapon_sg556")) Format(name, sizeof(name), "SG 553");
	else if(StrEqual(class, "weapon_aug")) Format(name, sizeof(name), "AUG");
	else if(StrEqual(class, "weapon_scar20")) Format(name, sizeof(name), "SCAR-20");
	else if(StrEqual(class, "weapon_g3sg1")) Format(name, sizeof(name), "G3SG1");
	else if(StrEqual(class, "weapon_knife_karambit")) Format(name, sizeof(name), "★ Karambit");
	else if(StrEqual(class, "weapon_knife_m9_bayonet")) Format(name, sizeof(name), "★ M9 Bayonet");
	else if(StrEqual(class, "weapon_bayonet")) Format(name, sizeof(name), "★ Bayonet");
	else if(StrEqual(class, "weapon_knife_survival_bowie")) Format(name, sizeof(name), "★ Bowie Knife");
	else if(StrEqual(class, "weapon_knife_butterfly")) Format(name, sizeof(name), "★ Butterfly Knife");
	else if(StrEqual(class, "weapon_knife_flip")) Format(name, sizeof(name), "★ Flip Knife");
	else if(StrEqual(class, "weapon_knife_push")) Format(name, sizeof(name), "★ Shadow Daggers");
	else if(StrEqual(class, "weapon_knife_tactical")) Format(name, sizeof(name), "★ Huntsman Knife");
	else if(StrEqual(class, "weapon_knife_falchion")) Format(name, sizeof(name), "★ Falchion Knife");
	else if(StrEqual(class, "weapon_knife_gut")) Format(name, sizeof(name), "★ Gut Knife");
	else if(StrEqual(class, "weapon_knife_ursus")) Format(name, sizeof(name), "★ Ursus Knife");
	else if(StrEqual(class, "weapon_knife_gypsy_jackknife")) Format(name, sizeof(name), "★ Navaja Knife");
	else if(StrEqual(class, "weapon_knife_stiletto")) Format(name, sizeof(name), "★ Stiletto Knife");
	else if(StrEqual(class, "weapon_knife_widowmaker")) Format(name, sizeof(name), "★ Talon Knife");
	else if(StrEqual(class, "weapon_mp5sd")) Format(name, sizeof(name), "MP5-SD");
	else if(StrEqual(class, "weapon_knife_css")) Format(name, sizeof(name), "★ Classic Knife");
	else if(StrEqual(class, "weapon_knife_cord")) Format(name, sizeof(name), "★ Paracord Knife");
	else if(StrEqual(class, "weapon_knife_canis")) Format(name, sizeof(name), "★ Survival Knife");
	else if(StrEqual(class, "weapon_knife_outdoor")) Format(name, sizeof(name), "★ Nomad Knife");
	else if(StrEqual(class, "weapon_knife_skeleton")) Format(name, sizeof(name), "★ Skeleton Knife");
	else Format(name, sizeof(name), "Unknown");
	return name;
}

stock char GetServerAdress()
{
	char address[35];
	char port[5];
	FindConVar("hostport").GetString(port, sizeof(port));

	char pieces[4];
	int longip = GetConVarInt(FindConVar("hostip"));
	
	pieces[0] = (longip >> 24) & 0x000000FF;
	pieces[1] = (longip >> 16) & 0x000000FF;
	pieces[2] = (longip >> 8) & 0x000000FF;
	pieces[3] = longip & 0x000000FF;

	Format(address, sizeof(address), "%d.%d.%d.%d:%s", pieces[0], pieces[1], pieces[2], pieces[3], port);
	return address;
}

stock void RemoveWeaponPrefix(const char[] source, char[] output, int size)
{
	strcopy(output, size, source[7]);
}

stock char GetCurrentGameMode()
{
	char mapname[64];
	GetCurrentMap(mapname, sizeof(mapname));
	return GetGameMode(mapname);
}

stock char GetGameMode(char[] map)
{
	char gamemode[32];
	if(IsStrContains(map, "mg_")) Format(gamemode, sizeof(gamemode), "MG");
	else if(IsStrContains(map, "bb_")) Format(gamemode, sizeof(gamemode), "BB");
	else if(IsStrContains(map, "awp_")) Format(gamemode, sizeof(gamemode), "AWP");
	else if(IsStrContains(map, "de_")) Format(gamemode, sizeof(gamemode), "PUB");
	else if(IsStrContains(map, "deathrun_")) Format(gamemode, sizeof(gamemode), "DR");
	else if(IsStrContains(map, "ox_")) Format(gamemode, sizeof(gamemode), "OX");
	else if(IsStrContains(map, "boss_")) Format(gamemode, sizeof(gamemode), "BOSS");
	else if(IsStrContains(map, "zm_")) Format(gamemode, sizeof(gamemode), "ZM");
	else if(IsStrContains(map, "ze_")) Format(gamemode, sizeof(gamemode), "ZE");
	else if(IsStrContains(map, "ttt_")) Format(gamemode, sizeof(gamemode), "TTT");
	else if(IsStrContains(map, "35hp_")) Format(gamemode, sizeof(gamemode), "35HP");
	else if(IsStrContains(map, "surf_")) Format(gamemode, sizeof(gamemode), "SURF");
	else if(IsStrContains(map, "jb_")) Format(gamemode, sizeof(gamemode), "JB");
	else Format(gamemode, sizeof(gamemode), "UNKNOWN");
	return gamemode;
}

stock int GetKnifeDefIndex(char[] classname)
{
	if(StrEqual(classname, "weapon_knife_cord")) return 49;
	else if(StrEqual(classname, "weapon_knife_canis")) return 50;
	else if(StrEqual(classname, "weapon_knife_outdoor")) return 51;
	else if(StrEqual(classname, "weapon_knife_skeleton")) return 52;
	else if(StrEqual(classname, "weapon_knife_css")) return 48;
	else if(StrEqual(classname, "weapon_knife_ursus")) return 43;
	else if(StrEqual(classname, "weapon_knife_gypsy_jackknife")) return 44;
	else if(StrEqual(classname, "weapon_knife_stiletto")) return 45;
	else if(StrEqual(classname, "weapon_knife_widowmaker")) return 46;
	else if(StrEqual(classname, "weapon_knife_karambit")) return 33;
	else if(StrEqual(classname, "weapon_knife_m9_bayonet")) return 34;
	else if(StrEqual(classname, "weapon_bayonet")) return 35;
	else if(StrEqual(classname, "weapon_knife_survival_bowie")) return 36;
	else if(StrEqual(classname, "weapon_knife_butterfly")) return 37;
	else if(StrEqual(classname, "weapon_knife_flip")) return 38;
	else if(StrEqual(classname, "weapon_knife_push")) return 39;
	else if(StrEqual(classname, "weapon_knife_tactical")) return 40;
	else if(StrEqual(classname, "weapon_knife_falchion")) return 41;
	else if(StrEqual(classname, "weapon_knife_gut")) return 42;
	else return -1;
}

/**
 * Replaces the color codes in a string.
 *
 * @param szoveg	string
 * @return string with replaced colorcodes.
 */
stock char ReplaceColors(char[] szoveg, bool clear = false)
{
	char cSzoveg[255];
	Format(cSzoveg, sizeof(cSzoveg), szoveg);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{red}", clear?"":"\x07", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{lightred}", clear?"":"\x0F", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{darkred}", clear?"":"\x02", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{bluegrey}", clear?"":"\x0A", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{blue}", clear?"":"\x0B", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{darkblue}", clear?"":"\x0C", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{purple}", clear?"":"\x03", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{orchid}", clear?"":"\x0E", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{yellow}", clear?"":"\x09", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{gold}", clear?"":"\x10", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{lightgreen}", clear?"":"\x05", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{green}", clear?"":"\x04", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{lime}", clear?"":"\x06", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{grey}", clear?"":"\x08", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{grey2}", clear?"": "\x0D", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{teamcolor}", clear?"":"\x03", false);
	ReplaceString(cSzoveg, sizeof(cSzoveg), "{default}", clear?"":"\x01", false);

	return cSzoveg;
}

/**
 * Returns a random client from the specified team.
 *
 * @param team	teamindex.
 * @return Random client index.
 */
stock int GetRandomPlayerFromTeam(int team)
{
	new jatekosok[MaxClients+1];
	int jatekosszam;

	for(int i=1; i <= MaxClients; i++)
	{
	    if(IsValidClient(i) && GetClientTeam(i) == team)
	    {
	        jatekosok[jatekosszam++] = i;
	    }
	}

	return jatekosok[GetRandomInt(0, jatekosszam-1)];
}

/**
 * Returns an alive client from the specified team.
 *
 * @param team	teamindex.
 * @return Alive client index.
 */
stock int GetAliveClientTeam(int team)
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if(IsClientInGame(i) && GetClientTeam(i) == team && IsPlayerAlive(i) && !IsFakeClient(i))
		{
			return i;
		}
	}
	
	return 0;
}

/**
 * Check whether the client is valid or no.
 *
 * @param client	Clientindex.
 * @return True if valid, false otherwise.
 */
stock bool IsValidClient(int client)
{
	if(client <= 0) return false;
	if(client > MaxClients) return false;
	if(!IsClientConnected(client)) return false;
	if(IsFakeClient(client)) return false;
	if(IsClientSourceTV(client)) return false;
	return IsClientInGame(client);
}

stock bool IsModelEquipped(int client, const char[] modellname)
{
	char ModelName[256];
	GetEntityModel(client, ModelName, sizeof(ModelName));
	
	for(int i=strlen(ModelName)-1;i > 0;i--)
	{
		if(ModelName[i] == '/')
		{
			Format(ModelName, sizeof(ModelName), ModelName[i+1]);
			break;
		}
	}
	
	if(StrEqual(ModelName, modellname))
		return true;

	return false;
}

stock bool IsStrContains(char[] String, char[] Text, bool caseSensitive = false)
{
	if((StrContains(String, Text, caseSensitive) != -1))
		return true;

	return false;
}

stock int GetCharCount(char[] s, char[] c)
{
	int count = 0;
	for (int i = 0; i < strlen(s); ++i)
	{
		if(s[i] == c[0]) count++;
	}

	return count;
}

stock char GetEntityModel(int entity, char[] Buffer, int length)
{
	GetEntPropString(entity, Prop_Data, "m_ModelName", Buffer, length);
}

stock int NullMenuHandler(Handle menu, MenuAction action, int param1, int param2) 
{
	if(action == MenuAction_End)
	{
		delete menu;
	}
}

stock int GetPercent(int number, int count) 
{
    return RoundFloat(FloatMul((float(number) / float(count)), 100.0)); 
}

stock void SetPlayerColor(int client, int colors[3])
{
    SetEntityRenderColor(client, colors[0], colors[1], colors[2], 255);
}

stock bool RemoveMapPath(const char[] map, char[] destination, int maxlen)
{
	if (strlen(map) < 1)
	{
		ThrowError("Bad map name: %s", map);
	}
	
	int pos = FindCharInString(map, '/', true);
	if (pos == -1)
	{
		pos = FindCharInString(map, '\\', true);
		if (pos == -1)
		{
			strcopy(destination, maxlen, map);
			return false;
		}
	}

	int len = strlen(map) - 1 - pos;
	
	SubString(map, pos + 1, len, destination, maxlen);
	return true;
}

stock bool SubString(const char[] source, int start, int len, char[] destination, int maxlen)
{
	if (maxlen < 1)
	{
		ThrowError("Destination size must be 1 or greater, but was %d", maxlen);
	}
	
	if (len == 0)
	{
		destination[0] = '\0';
		return true;
	}
	
	if (start < 0)
	{
		start = strlen(source) + start;
		if (start < 0)
			start = 0;
	}
	
	if (len < 0)
	{
		len = strlen(source) + len - start;
		if (len < 0)
			return false;
	}
	
	int realLength = len + 1 < maxlen ? len + 1 : maxlen;
	
	strcopy(destination, realLength, source[start]);
	return true;
}

stock bool IsLookingAtWall(int client, float distance=40.0) {

	float posEye[3], posEyeAngles[3];
	bool isClientLookingAtWall = false;

	GetClientEyePosition(client,	posEye);
	GetClientEyeAngles(client,		posEyeAngles);

	posEyeAngles[0] = 0.0;

	Handle trace = TR_TraceRayFilterEx(posEye, posEyeAngles, CONTENTS_SOLID, RayType_Infinite, LookingWall_TraceEntityFilter);

	if (TR_DidHit(trace)) {

		if (TR_GetEntityIndex(trace) > 0) {
			CloseHandle(trace);
			return false;
		}

		float posEnd[3];

		TR_GetEndPosition(posEnd, trace);

		if (GetVectorDistance(posEye, posEnd, true) <= (distance * distance)) {
			isClientLookingAtWall = true;
		}
	}

	CloseHandle(trace);

	return isClientLookingAtWall;
}

public bool LookingWall_TraceEntityFilter(int entity, int contentsMask)
{
	return entity == 0;
}

stock void FakePrecacheSound(const char[] szPath)
{
	AddToStringTable(FindStringTable("soundprecache"), szPath);
}

stock bool SafeRemoveWeapon(int client, int weapon, int slot)
{
    if (HasEntProp(weapon, Prop_Send, "m_iItemDefinitionIndex"))
    {
        int iDefIndex = GetEntProp(weapon, Prop_Send, "m_iItemDefinitionIndex");
       
        if (iDefIndex < 0 || iDefIndex > 700)
        {
            return false;
        }
    }
   
    if (HasEntProp(weapon, Prop_Send, "m_bInitialized"))
    {
        if (GetEntProp(weapon, Prop_Send, "m_bInitialized") == 0)
        {
            return false;
        }
    }
   
    if (HasEntProp(weapon, Prop_Send, "m_bStartedArming"))
    {
        if (GetEntSendPropOffs(weapon, "m_bStartedArming") > -1)
        {
            return false;
        }
    }
   
    if (GetPlayerWeaponSlot(client, slot) != weapon)
    {
        return false;
    }
   
    if (!RemovePlayerItem(client, weapon))
    {
        return false;
    }
   
    int iWorldModel = GetEntPropEnt(weapon, Prop_Send, "m_hWeaponWorldModel");
   
    if (IsValidEdict(iWorldModel) && IsValidEntity(iWorldModel))
    {
        if (!AcceptEntityInput(iWorldModel, "Kill"))
        {
            return false;
        }
    }
   
    if (weapon == GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon"))
    {
        SetEntPropEnt(client, Prop_Send, "m_hActiveWeapon", -1);
    }
   
    AcceptEntityInput(weapon, "Kill");
   
    return true;
}

/**
 * Copies a 1 dimensional static array.
 *
 * @param array			Static Array to copy from.
 * @param newArray		New Array to copy to.
 * @param size			Size of the array (or number of cells to copy)
 * @noreturn
 */
stock void Array_Copy(const any[] array, any[] newArray, int size)
{
	for (int i=0; i < size; i++) {
		newArray[i] = array[i];
	}
}

/**
 * Forces the client to look at the target.
 *
 * @param client	Client index.
 * @param target	Target index.
 * @noreturn
 */
stock void LookAtTarget(int client, int target)
{
    float angles[3];
	float clientEyes[3];
	float targetEyes[3];
	float resultant[3];

	GetClientEyePosition(client, clientEyes);
	GetClientEyePosition(target, targetEyes);
	MakeVectorFromPoints(targetEyes, clientEyes, resultant);
	GetVectorAngles(resultant, angles);
	if(angles[0] >= 270){
		angles[0] -= 270;
		angles[0] = (90-angles[0]);
	} else {
		if(angles[0] <= 90){
			angles[0] *= -1;
		}
	}

	angles[1] -= 180;
	TeleportEntity(client, NULL_VECTOR, angles, NULL_VECTOR);
}

/**
 * Returns the itemdefinitionindex.
 *
 * @param classname			Weapon classname.
 * @return itemdefinitionindex
 */
stock int GetWeaponDefIndexFromClass(char[] classname)
{
	if(StrEqual(classname, "weapon_awp")) return 9;
	else if(StrEqual(classname, "weapon_ak47")) return 7;
	else if(StrEqual(classname, "weapon_m4a1")) return 16;
	else if(StrEqual(classname, "weapon_m4a1_silencer")) return 60;
	else if(StrEqual(classname, "weapon_deagle")) return 1;
	else if(StrEqual(classname, "weapon_usp_silencer")) return 61;
	else if(StrEqual(classname, "weapon_hkp2000")) return 32;
	else if(StrEqual(classname, "weapon_glock")) return 4;
	else if(StrEqual(classname, "weapon_elite")) return 2;
	else if(StrEqual(classname, "weapon_p250")) return 36;
	else if(StrEqual(classname, "weapon_cz75a")) return 63;
	else if(StrEqual(classname, "weapon_fiveseven")) return 3;
	else if(StrEqual(classname, "weapon_tec9")) return 30;
	else if(StrEqual(classname, "weapon_revolver")) return 64;
	else if(StrEqual(classname, "weapon_nova")) return 35;
	else if(StrEqual(classname, "weapon_xm1014")) return 25;
	else if(StrEqual(classname, "weapon_mag7")) return 27;
	else if(StrEqual(classname, "weapon_sawedoff")) return 29;
	else if(StrEqual(classname, "weapon_m249")) return 14;
	else if(StrEqual(classname, "weapon_negev")) return 28;
	else if(StrEqual(classname, "weapon_mp9")) return 34;
	else if(StrEqual(classname, "weapon_mac10")) return 17;
	else if(StrEqual(classname, "weapon_mp7")) return 33;
	else if(StrEqual(classname, "weapon_ump45")) return 24;
	else if(StrEqual(classname, "weapon_p90")) return 19;
	else if(StrEqual(classname, "weapon_bizon")) return 26;
	else if(StrEqual(classname, "weapon_famas")) return 10;
	else if(StrEqual(classname, "weapon_galilar")) return 13;
	else if(StrEqual(classname, "weapon_ssg08")) return 40;
	else if(StrEqual(classname, "weapon_aug")) return 8;
	else if(StrEqual(classname, "weapon_sg556")) return 39;
	else if(StrEqual(classname, "weapon_scar20")) return 38;
	else if(StrEqual(classname, "weapon_g3sg1")) return 11;
	else if(StrEqual(classname, "weapon_healthshot")) return 57;
	else return -1;
}

methodmap Jatekos
{
	public Jatekos(int jatekos) {
		return view_as<Jatekos>(jatekos);
	}

	property int index {
		public get() {
			return view_as<int>(this);
		}
	}

	property int UserId {
		public get() {
			return GetClientUserId(this.index);
		}
	}

	property int Serial {
		public get() {
			return GetClientSerial(this.index);
		}
	}

	property bool IsValid {
		public get() {
			return IsValidClient(this.index);
		}
	}

	property bool IsConnected {
		public get() {
			return IsClientConnected(this.index);
		}
	}

	property bool InGame {
		public get() {
			return IsClientInGame(this.index);
		}
	}

	property bool IsTimingOut {
		public get() {
			return IsClientTimingOut(this.index);
		}
	}

	property bool InKickQueue {
		public get() {
			return IsClientInKickQueue(this.index);
		}
	}

	property bool IsAuthorized {
		public get() {
			return IsClientAuthorized(this.index);
		}
	}

	property bool IsFake {
		public get() {
			return IsFakeClient(this.index);
		}
	}

	property bool IsSourceTV {
		public get() {
			return IsClientSourceTV(this.index);
		}
	}

	property bool IsReplay {
		public get() {
			return IsClientReplay(this.index);
		}
	}

	property bool IsObserver {
		public get() {
			return IsClientObserver(this.index);
		}
	}

	property bool IsAlive {
		public get() {
			return IsPlayerAlive(this.index);
		}
	}

	property int FlagBits {
		public get() {
			return GetUserFlagBits(this.index);
		}
	}

	property int Team {
		public get() {
			return GetClientTeam(this.index);
		}
		public set(int team) {
			ChangeClientTeam(this.index, team);
		}
	}

	property int Health {
		public get() {
			return GetClientHealth(this.index);
		}
		public set(int hp) {
			SetEntityHealth(this.index, hp);
		}
	}

	property int Deaths {
		public get() {
			return GetClientDeaths(this.index);
		}
	}

	property int Frags {
		public get() {
			return GetClientFrags(this.index);
		}
	}

	property int Armor {
		public get() {
			return GetClientArmor(this.index);
		}

		public set(int armor) {
			SetEntProp(this.index, Prop_Send, "m_ArmorValue", armor);
		}
	}

	property int Helmet {
		public get() {
			return GetEntProp(this.index, Prop_Send, "m_bHasHelmet");
		}

		public set(int helmet) {
			SetEntProp(this.index, Prop_Send, "m_bHasHelmet", helmet);
		}
	}

	property int DataRate {
		public get() {
			return GetClientDataRate(this.index);
		}
	}

	property bool IsBot {
		public get() {
			return IsFakeClient(this.index);
		}
	}

	property float ConnectionTime {
		public get() {
			return GetClientTime(this.index);
		}
	}

	property float MovementValue {
		public get() {
			return GetEntPropFloat(this.index, Prop_Data, "m_flLaggedMovementValue"); 
		}

		public set(float value) {
			SetEntPropFloat(this.index, Prop_Data, "m_flLaggedMovementValue", value);
		}
	}

	property int Money {
		public get() {
			return GetEntProp(this.index, Prop_Send, "m_iAccount");
		}

		public set(int value) {
			SetEntProp(this.index, Prop_Send, "m_iAccount", value);
		}
	}
	
	public bool GetName(char[] name, int maxlen) {
		return GetClientName(this.index, name, maxlen);
	}

	public bool GetIP(char[] ip, int maxlen, bool remport=true) {
		return GetClientIP(this.index, ip, maxlen, remport);
	}

	public bool GetAuthId(AuthIdType authType, char[] auth, int maxlen, bool validate=true) {
		return GetClientAuthId(this.index, authType, auth, maxlen, validate);
	}

	public char GetClanTag(char[] buffer, int maxlength)
	{
		return CS_GetClientClanTag(this.index, buffer, maxlength);
	}

	public int GrabSteamAccountID(bool validate=true) {
		return GetSteamAccountID(this.index, validate);
	}

	public bool GetInfo(const char[] key, char[] value, int maxlen) {
		return GetClientInfo(this.index, key, value, maxlen);
	}

	public void SetFlagBits(int flags) {
	   SetUserFlagBits(this.index, flags);
	}

	public bool CanTarget(int target) {
		return CanUserTarget(this.index, target);
	}

	public bool AdminCacheCheck() {
		return RunAdminCacheChecks(this.index);
	}

	public void PostAdminCheck() {
		NotifyPostAdminCheck(this.index);
	}

	public void GetModel(char[] model, int maxlen) {
		GetClientModel(this.index, model, maxlen);
	}

	public void GetWeapon(char[] weapon, int maxlen) {
		GetClientWeapon(this.index, weapon, maxlen);
	}

	public void GetMaxs(float vec[3]) {
		GetClientMaxs(this.index, vec);
	}

	public void GetMins(float vec[3]) {
		GetClientMins(this.index, vec);
	}

	public void GetAbsAngles(float ang[3]) {
	   GetClientAbsAngles(this.index, ang);
	}

	public void SetClanTag(char[] tag) {
		CS_SetClientClanTag(this.index, tag);
	}

	public void GetAbsOrigin(float vec[3]) {
		GetClientAbsOrigin(this.index, vec);
	}

	public void GetEyeAngle(float eang[3]) {
		GetClientEyeAngles(this.index, eang);
	}

	public void GetEyePosition(float pos[3]) {
		GetClientEyePosition(this.index, pos);
	}

	public float GetLatency(NetFlow flow) {
		return GetClientLatency(this.index, flow);
	}

	public float GetAvgLatency(NetFlow flow) {
		return GetClientAvgLatency(this.index, flow);
	}

	public float GetAvgLoss(NetFlow flow) {
		return GetClientAvgLoss(this.index, flow);
	}

	public float GetAvgChoke(NetFlow flow) {
		return GetClientAvgChoke(this.index, flow);
	}

	public float GetAvgData(NetFlow flow) {
		return GetClientAvgData(this.index, flow);
	}

	public float GetAvgPackets(NetFlow flow) {
		return GetClientAvgPackets(this.index, flow);
	}

	public void ChangeTeam(int team) {
		ChangeClientTeam(this.index, team);
	}

	public void SetModel(char[] model) {
		SetEntityModel(this.index, model);
	}

	public void Kill() {
		ForcePlayerSuicide(this.index);
	}

	public void Respawn() {
		CS_RespawnPlayer(this.index);
	}

	public void SetName(char[] newname) {
		SetClientName(this.index, newname);
	}

	public void Boost(float amount) {
		float direction[3], vel[3];
		GetEntPropVector(this.index, Prop_Data, "m_vecVelocity", vel); 
		NormalizeVector(vel, direction);
		ScaleVector(direction, amount); 
		AddVectors(vel, direction, vel); 
		TeleportEntity(this.index, NULL_VECTOR, NULL_VECTOR, vel); 
	}

	public void SetColor(int colors[3]) {
		SetEntityRenderColor(this.index, colors[0], colors[1], colors[2], 255);
	}

	public void Teleport(const float origin[3], const float angles[3], const float velocity[3])
	{
		TeleportEntity(this.index, origin, angles, velocity);
	}

	public void SetMoveType(MoveType type)
	{
		SetEntityMoveType(this.index, type);
	}

	public void Kick(const char[] format="", any ...) {
		if (format[0] == '\0')
			KickClient(this.index, "No reason given");
		else {
			char message[256];
			VFormat(message, sizeof(message), format, 3);
			KickClient(this.index, message);
		}
	}
}

methodmap Weapon < KeyValues 
{
	public void GetVector(float[3] vec)
	{
		this.GetVector(NULL_STRING, vec);
	}
	
	public void GetClassname(char[] buffer, int maxlength)
	{
		this.GetSectionName(buffer, maxlength);
	}
	
	public bool Spawn()
	{
		char classname[32];
		float vec[3];
		int entity;
		
		this.GetVector(vec);
		this.GetClassname(classname, sizeof(classname));
		
		entity = CreateEntityByName(classname);
		if (entity != -1)
		{
			TeleportEntity(entity, vec, NULL_VECTOR, NULL_VECTOR);
			SetEntProp(entity, Prop_Send, "m_iItemDefinitionIndex", GetWeaponDefIndexFromClass(classname));
			DispatchSpawn(entity);
			return true;
		}
		else
		{
			return false;
		}
	}
	
	public Weapon(const char[] classname, const float vec[3])
	{
		KeyValues kv = new KeyValues(classname);
		kv.SetVector(NULL_STRING, vec); 
		return view_as<Weapon>(kv);
	}
	
	public static void SpawnWeapons(ArrayList array)
	{
		Weapon weapon;
		for (int i = 0; i < array.Length; i++)
		{
			weapon = array.Get(i);
			weapon.Spawn();
		}
	}
	
	public static Weapon EntityToWeapon(int entity)
	{
		if (!IsValidEntity(entity))
		{
			return null;
		}
		else
		{
			char classname[64];
			float vec[3];

			GetEntityClassname(entity, classname, sizeof(classname));
			GetEntPropVector(entity, Prop_Send, "m_vecOrigin", vec);
			
			return (new Weapon(classname, vec));
		}
	}
}