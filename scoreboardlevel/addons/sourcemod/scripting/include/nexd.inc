#if defined _nexd_included_ 
  #endinput 
#endif 
#define _nexd_included_
//#define PREFIX "\x01[\x0BRendszer\x01]" //Used for my things, change it if you want
#define empty "\0"
#define m_iMaxVelocity 9999999.0
#define MAX_WEAPONS 48
#define INTEGER_STRING_LENGTH 20
#define GAMEUNITS_TO_METERS 0.01905
#define LoopValidClients(%1) for(int %1 = 1; %1 <= MaxClients; %1++) if(IsValidClient(%1))


#include <clients>
#include <sdktools>
#include <cstrike>
#include <clientprefs>

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

stock bool IsInteger(char[] buffer)
{
    for (int i = 0; i < strlen(buffer); i++)
    {
        if ( !IsCharNumeric(buffer[i]) )
            return false;
    }

    return true;    
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

stock Weapon_GetPrimaryAmmoType(int weapon)
{
	return GetEntProp(weapon, Prop_Data, "m_iPrimaryAmmoType");
}

stock Weapon_GetSecondaryAmmoType(int weapon)
{
	return GetEntProp(weapon, Prop_Data, "m_iSecondaryAmmoType");
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
		// Oh My God! They Killed Kenny!!
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

stock Client_GetWeaponsOffset(int client)
{
	static offset = -1;

	if (offset == -1) {
		offset = FindDataMapInfo(client, "m_hMyWeapons");
	}

	return offset;
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

stock int GetAliveClientsCountTeam(int team)
{
	int j = 0;
	for (int i = 1; i <= MaxClients; i++)
	{
		if(IsClientInGame(i) && GetClientTeam(i) == team && IsPlayerAlive(i) && !IsFakeClient(i))
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
		if(IsClientInGame(i) && GetClientTeam(i) == team)
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

stock bool IsWarmup()
{
	return view_as<bool>(GameRules_GetProp("m_bWarmupPeriod"));
}

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

stock bool IsKnife(char[] knifename)
{
	if(StrEqual(knifename, "weapon_knife") || 
    	StrEqual(knifename, "weapon_bayonet") || 
    	StrEqual(knifename, "weapon_knife_flip") || 
    	StrEqual(knifename, "weapon_knife_gut") || 
    	StrEqual(knifename, "weapon_knife_karambit") || 
    	StrEqual(knifename, "weapon_knife_m9_bayonet") || 
    	StrEqual(knifename, "weapon_knife_tactical") || 
    	StrEqual(knifename, "weapon_knife_push") || 
    	StrEqual(knifename, "weapon_knife_butterfly") || 
    	StrEqual(knifename, "weapon_knife_falchion") || 
    	StrEqual(knifename, "weapon_knife_survival_bowie") || 
    	StrEqual(knifename, "weapon_knife_ursus") || 
    	StrEqual(knifename, "weapon_knife_gypsy_jackknife") || 
    	StrEqual(knifename, "weapon_knife_stiletto"))
    	return true;

    return false;
}

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

stock GetEntityModel(int entity, char[] Buffer, int length)
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

stock GetPercent(number, count) 
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
	
	public bool GetName(char[] name, int maxlen) {
		return GetClientName(this.index, name, maxlen);
	}

	public bool GetIP(char[] ip, int maxlen, bool remport=true) {
		return GetClientIP(this.index, ip, maxlen, remport);
	}

	public bool GetAuthId(AuthIdType authType, char[] auth, int maxlen, bool validate=true) {
		return GetClientAuthId(this.index, authType, auth, maxlen, validate);
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