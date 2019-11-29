#include <sourcemod>
#include <clientprefs>
#include <multicolors>
#include <nexd>
#include <sdkhooks>

#define PLUGIN_NEV	"Scoreboard Custom Levels"
#define PLUGIN_LERIAS	"(9_9)"
#define PLUGIN_AUTHOR	"Nexd"
#define PLUGIN_VERSION	"1.1.1"
#define PLUGIN_URL	"https://github.com/KillStr3aK"
#define MAX_ICONS 128
#pragma tabsize 0

enum LevelIcon
{
	String:MenuNev[32],
	Flag,
	IconIndex
}

enum {
	SaveClients,
	ChatPrefix,
	Count
}

int g_eLevelIcons[MAX_ICONS][LevelIcon];
int g_iLevelIcons = 0;

int m_iOffset = -1;
int m_iLevel[MAXPLAYERS+1];

char m_cFilePath[PLATFORM_MAX_PATH];
char m_cPrefix[128];

Handle m_hIndexCookie = INVALID_HANDLE;
ConVar g_cR[Count];

public Plugin myinfo = 
{
	name = PLUGIN_NEV,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_LERIAS,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
};

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{
	RegPluginLibrary("ScoreboardCustomLevels");
	CreateNative("SCL_GetLevel", Native_GetLevel);
	
	return APLRes_Success;
}

public Native_GetLevel(Handle:plugin, params)
{
	int client = GetNativeCell(1);
	
	return m_iLevel[client];
}

public void OnPluginStart()
{
	m_iOffset = FindSendPropInfo("CCSPlayerResource", "m_nPersonaDataPublicLevel");
	BuildPath(Path_SM, m_cFilePath, sizeof(m_cFilePath), "configs/level_icons.cfg");

	RegConsoleCmd("sm_icons", Command_LevelIcons);

	m_hIndexCookie = RegClientCookie("levelicon_index", "Image index for the level", CookieAccess_Private);
	g_cR[SaveClients] = CreateConVar("level_icons_save", "1", "Save player preferences?");
	g_cR[ChatPrefix] = CreateConVar("level_icons_chat_prefix", "{default}[{red}Level-Icons{default}]", "Chat prefix in messages");

	for (int i = MaxClients; i > 0; --i)
    {
        if (!AreClientCookiesCached(i))
        {
            continue;
        }
        
        OnClientCookiesCached(i);
    }
}

public void OnConfigsExecuted()
{
	g_cR[ChatPrefix].GetString(m_cPrefix, sizeof(m_cPrefix));
}

public void OnClientCookiesCached(int client) 
{
	char Index[8];
	GetClientCookie(client, m_hIndexCookie, Index, sizeof(Index));
	m_iLevel[client] = StringToInt(Index);
}

public Action Command_LevelIcons(int client, int args)
{
	if(!IsValidClient(client)) return Plugin_Handled;
	IconMenu(view_as<Jatekos>(client));
    return Plugin_Handled; 
}

public void IconMenu(Jatekos jatekos)
{
	char IndexString[10];
	char m_cMenuLine[128];

	Menu menu = CreateMenu(IconList);
	menu.SetTitle("Level Icons");
	if(m_iLevel[jatekos.index] != -1) menu.AddItem("clear", "Clear");
	else menu.AddItem("", "Clear", ITEMDRAW_DISABLED);
	menu.AddItem("", "", ITEMDRAW_SPACER);
	for(int i = 0; i < g_iLevelIcons; ++i)
	{
		IntToString(g_eLevelIcons[i][IconIndex], IndexString, sizeof(IndexString));
		Format(m_cMenuLine, sizeof(m_cMenuLine), "%s [ EQUIPPED ]", g_eLevelIcons[i][MenuNev]);
		if(m_iLevel[jatekos.index] == g_eLevelIcons[i][IconIndex]) menu.AddItem(IndexString, m_cMenuLine, ITEMDRAW_DISABLED);
		else {
			if(g_eLevelIcons[i][Flag] != -1)
			{
				if(CheckCommandAccess(jatekos.index, "", g_eLevelIcons[i][Flag])) menu.AddItem(IndexString, g_eLevelIcons[i][MenuNev]);
				else menu.AddItem(IndexString, g_eLevelIcons[i][MenuNev], ITEMDRAW_DISABLED);
			} else {
				menu.AddItem(IndexString, g_eLevelIcons[i][MenuNev]);
			}
		}
	}

	menu.Display(jatekos.index, MENU_TIME_FOREVER);
}

public int IconList(Menu menu, MenuAction mAction, int client, int item)
{
	if(mAction == MenuAction_Select)
	{
		char info[10];
		menu.GetItem(item, info, sizeof(info));
		if(StrEqual(info, "clear")) {
			LevelIconUnEquip(view_as<Jatekos>(client));
		} else {
			LevelIconEquip(view_as<Jatekos>(client), StringToInt(info));
		}

		IconMenu(Jatekos(client));
	}
}

public void LevelIconReset() 
{ 
	g_iLevelIcons = 0;
}

public void LevelIconEquip(Jatekos jatekos, int iconindex)
{
	CPrintToChat(jatekos.index, "%s You've equipped the {green}%s {default}icon!", m_cPrefix, g_eLevelIcons[GetIconFromIconIndex(iconindex)==-1?0:GetIconFromIconIndex(iconindex)][MenuNev]);
	m_iLevel[jatekos.index] = iconindex;
	if(g_cR[SaveClients].BoolValue) SetCookieInt(jatekos.index, m_hIndexCookie, iconindex);
}

public void LevelIconUnEquip(Jatekos jatekos)
{
	CPrintToChat(jatekos.index, "%s You've {green}cleared {default}your icon!", m_cPrefix);
	m_iLevel[jatekos.index] = -1;
	if(g_cR[SaveClients].BoolValue) SetClientCookie(jatekos.index, m_hIndexCookie, "-1");
}

public void OnMapStart()
{
	LevelIconReset();
	char sBuffer[PLATFORM_MAX_PATH];

	SDKHook(GetPlayerResourceEntity(), SDKHook_ThinkPost, OnThinkPost);

	KeyValues kv = CreateKeyValues("LevelIcons");
    FileToKeyValues(kv, m_cFilePath);
    
    if (!KvGotoFirstSubKey(kv)) return;

    do
	{
        KvGetString(kv, "name", g_eLevelIcons[g_iLevelIcons][MenuNev], 32);

		g_eLevelIcons[g_iLevelIcons][Flag] = KvGetNum(kv, "flag");
        g_eLevelIcons[g_iLevelIcons][IconIndex] = KvGetNum(kv, "index");
        g_iLevelIcons++;
    } while (KvGotoNextKey(kv));
    kv.Close();

    for(int i = 0; i < g_iLevelIcons; ++i)
	{
		FormatEx(sBuffer, sizeof(sBuffer), "materials/panorama/images/icons/xp/level%i.png", g_eLevelIcons[i][IconIndex]);
    	AddFileToDownloadsTable(sBuffer);
	}
}

public void OnThinkPost(int m_iEntity)
{
	int m_iLevelTemp[MAXPLAYERS+1] = 0;
	GetEntDataArray(m_iEntity, m_iOffset, m_iLevelTemp, MAXPLAYERS+1);

	for(int i = 1; i <= MaxClients; i++)
	{
		if(m_iLevel[i] > 0)
		{
			if(m_iLevel[i] != m_iLevelTemp[i]) SetEntData(m_iEntity, m_iOffset + (i * 4), m_iLevel[i]);
		}
	}
}

stock int GetIconFromIconIndex(int iconindex)
{
	for (int i = 0; i < g_iLevelIcons; ++i)
	{
		if(g_eLevelIcons[i][IconIndex] == iconindex) return i;
	}

	return -1;
}