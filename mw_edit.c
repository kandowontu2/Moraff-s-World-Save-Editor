/*
 * Moraff's World Save Editor - Win32 GUI
 * Compile: gcc -O2 -o mw_edit.exe mw_edit.c -lgdi32 -lcomdlg32 -mwindows
 */
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAVE_SIZE 2344
#define APP_TITLE L"Moraff's World Save Editor"

/* ---- Field types ---- */
typedef enum { T_STR, T_U8, T_S8, T_U16, T_U32, T_F32, T_F64, T_SELECT } FType;

typedef struct {
    const char *name;
    int offset;
    FType type;
    const char **opts;
    int nopts;
} Field;

static const char *race_opts[] = {"Human","Elf","Dwarf","Hobbit","Gnome","Ogre","Sprite","Imp"};
static const char *sex_opts[]  = {"Male","Female"};
static const char *cls_opts[]  = {"Fighter","Worshiper","Monk","Wizard","Priest","Sage","Mage"};

/* ---- Tab definitions ---- */
/* Each tab has groups, each group has fields */
typedef struct { const char *name; Field *fields; int nfields; } GrpDef;
typedef struct { const char *name; GrpDef *groups; int ngroups; } TabDef;

/* Tab 0: Character & Stats */
static Field f_identity[] = {
    {"Name",          0x000, T_STR,    NULL, 0},
    {"Race",          0x028, T_SELECT, race_opts, 8},
    {"Sex",           0x029, T_SELECT, sex_opts, 2},
    {"Class",         0x02A, T_SELECT, cls_opts, 7},
    {"Height",        0x03D, T_U8,    NULL, 0},
    {"Weight",        0x03F, T_U8,    NULL, 0},
};
static Field f_level[] = {
    {"Level",         0x7A8, T_U16, NULL, 0},
    {"HP Current",    0x031, T_U16, NULL, 0},
    {"HP Max",        0x033, T_U16, NULL, 0},
    {"SP Current",    0x035, T_F32, NULL, 0},
    {"SP Maximum",    0x039, T_F32, NULL, 0},
    {"Age",           0x7D6, T_U32, NULL, 0},
};
static Field f_attrs[] = {
    {"Strength",      0x812, T_U16, NULL, 0},
    {"Intelligence",  0x814, T_U16, NULL, 0},
    {"Wisdom",        0x816, T_U16, NULL, 0},
    {"Constitution",  0x818, T_U16, NULL, 0},
    {"Agility",       0x81A, T_U16, NULL, 0},
    {"Luck",          0x81C, T_U16, NULL, 0},
};
static Field f_pos[] = {
    {"Floor Depth",   0x7B0, T_U16, NULL, 0},
    {"X Position",    0x7AC, T_U16, NULL, 0},
    {"Y Position",    0x7AE, T_U16, NULL, 0},
    {"Raise Contract",0x806, T_U16, NULL, 0},
    {"Diseased Turns",0x7CA, T_U16, NULL, 0},
    {"Poisoned Turns",0x7CC, T_U16, NULL, 0},
};
static GrpDef g_charstats[] = {
    {"Character",  f_identity, 6},
    {"Level & HP", f_level, 6},
    {"Attributes", f_attrs, 6},
    {"Position",   f_pos, 6},
};

/* Tab 1: Inventory */
static Field f_money[] = {
    {"Jewels Pocket",  0x454, T_U32, NULL, 0},
    {"Jewels Bank",    0x458, T_U32, NULL, 0},
    {"Copper Stones",  0x45C, T_U32, NULL, 0},
    {"Silver Stones",  0x460, T_U32, NULL, 0},
    {"Ivory Stones",   0x464, T_U32, NULL, 0},
    {"Gold Stones",    0x468, T_U32, NULL, 0},
    {"Platinum Stones",0x46C, T_U32, NULL, 0},
    {"Jewel Stones",   0x470, T_U32, NULL, 0},
};
static Field f_items[] = {
    {"Holy H Grenade", 0x7C8, T_U8, NULL, 0},
    {"Stone of Telep", 0x810, T_U8, NULL, 0},
    {"Stone of See",   0x7C9, T_U8, NULL, 0},
    {"Floor Slosher",  0x7FE, T_U8, NULL, 0},
    {"Potion of Heal", 0x80E, T_U8, NULL, 0},
};
static Field f_pills[] = {
    {"Green Pill",     0x15E, T_S8, NULL, 0},
    {"Orange Pill",    0x15D, T_S8, NULL, 0},
    {"Blue Pill",      0x15F, T_S8, NULL, 0},
    {"Red Pill",       0x160, T_S8, NULL, 0},
    {"White Pill",     0x161, T_S8, NULL, 0},
    {"Yellow Pill",    0x162, T_S8, NULL, 0},
};
static Field f_gear[] = {
    {"Ring of Regen",  0x7C6, T_U8, NULL, 0},
    {"Ring of Prot+",  0x7D1, T_U8, NULL, 0},
    {"AntiMagic Ring", 0x7D2, T_U8, NULL, 0},
    {"Body Armor Lv",  0x7D0, T_U8, NULL, 0},
    {"Gauntlet",       0x846, T_S8, NULL, 0},
};
static GrpDef g_inventory[] = {
    {"Money",     f_money, 8},
    {"Items",     f_items, 5},
    {"Pills",     f_pills, 6},
    {"Worn Gear", f_gear, 5},
};

/* Tab 2: Spell Effects */
static Field f_effects[] = {
    {"Weapon Plus Turns",  0x7CE, T_U8,  NULL, 0},
    {"Armor Plus Turns",   0x7CF, T_U8,  NULL, 0},
    {"Feather Turns",      0x7D3, T_U8,  NULL, 0},
    {"Fast Move Turns",    0x7D4, T_U8,  NULL, 0},
    {"Invisible Turns",    0x7D5, T_U8,  NULL, 0},
    {"Str Bonus Turns",    0x7DA, T_U8,  NULL, 0},
    {"Agi Bonus Turns",    0x7DB, T_U8,  NULL, 0},
    {"Super Str Turns",    0x7DC, T_U8,  NULL, 0},
    {"Super Agi Turns",    0x7DD, T_U8,  NULL, 0},
    {"Battle Str Turns",   0x7DE, T_U16, NULL, 0},
    {"Battle Speed Turns", 0x7E0, T_U16, NULL, 0},
    {"Slow Monster Turns", 0x7E2, T_U16, NULL, 0},
    {"Power Weapon Level", 0x7E4, T_U8,  NULL, 0},
    {"Power Wpn Turns",    0x7E5, T_U16, NULL, 0},
    {"Protect Level",      0x7E7, T_U8,  NULL, 0},
    {"Protect Turns",      0x7E8, T_U16, NULL, 0},
    {"Resist Poison Turns",0x7EA, T_U16, NULL, 0},
    {"Resist Disease Trns",0x7EC, T_U16, NULL, 0},
    {"Anti Cold Turns",    0x7EE, T_U16, NULL, 0},
    {"Anti Fire Turns",    0x7F0, T_U16, NULL, 0},
    {"Resist Drain Turns", 0x7F2, T_U16, NULL, 0},
    {"Stop Monster Turns", 0x7F4, T_U16, NULL, 0},
    {"Hold Monster Turns", 0x7F6, T_U16, NULL, 0},
};
static GrpDef g_effects[] = {
    {"Active Spell Effects", f_effects, 23},
};

static TabDef tabs[] = {
    {"Character && Stats", g_charstats, 4},
    {"Inventory",          g_inventory, 4},
    {"Spell Effects",      g_effects, 1},
    {"Equipment",          NULL, 0},   /* custom draw */
    {"Spell Grids",        NULL, 0},   /* custom draw */
};
#define NTABS 5
#define TAB_EQUIP 3
#define TAB_GRIDS 4

/* Equipment */
static const char *wep_names[] = {"Fist","Stick","Club","Mace","Knife","Shortsword","Long Sword","Great Sword"};
static const char *arm_names[] = {"Skin","Leather","Chain","Scale","Plate","Field Plate","Titanium","Ogre"};
#define EQ_WEP_OWN  0x081
#define EQ_WEP_ENCH 0x08E
#define EQ_ARM_OWN  0x0B0
#define EQ_ARM_ENCH 0x0B8

/* Spell grids */
static int spell_bases[]  = {0x177, 0x1A4, 0x1D1, 0x1FE};
static int scroll_bases[] = {0x22B, 0x258, 0x285, 0x2B2};
static int wand_bases[]   = {0x2DF, 0x30C, 0x339, 0x366};
static int paper_bases[]  = {0x393, 0x3C0, 0x3ED, 0x41A};
static const char *grid_names[] = {"Spells","Scrolls","Wands","Papers"};
static int *grid_bases[] = {spell_bases, scroll_bases, wand_bases, paper_bases};
static int grid_is_wand[] = {0, 0, 1, 0};
static const char *type_names[] = {"E (Permanent)","H (Preparation)","A (Wizard Battle)","P (Priest Battle)"};

/* ---- App state ---- */
static BYTE saveData[SAVE_SIZE];
static BYTE origData[SAVE_SIZE];
static BOOL fileLoaded = FALSE;
static char filePath[MAX_PATH] = {0};
static HFONT hFont, hFontBold, hFontMono, hFontGrp;

/* Layout constants */
#define LM        16
#define COL_W     340
#define COL_GAP   30
#define ROW_H     24
#define LABEL_W   160
#define EDIT_W    140
#define TOP_Y     52
#define GRP_PAD   10
#define GRP_HDR   28

/* Control IDs */
#define ID_OPEN     1001
#define ID_SAVE     1002
#define ID_SAVEAS   1003
#define ID_TAB_BASE 1010
#define ID_FIELD_BASE 2000
#define ID_EQ_BASE    3000
#define ID_GRID_TAB   4000
#define ID_GRID_ALLON 4100
#define ID_GRID_ALLOFF 4200
#define ID_GRID_CELL  5000

/* Max controls */
#define MAX_FIELDS 128
static HWND hFieldLabels[MAX_FIELDS], hFieldEdits[MAX_FIELDS];
static HWND hGrpBoxes[16];
static int nFieldControls = 0, nGrpBoxes = 0;

static HWND hEqLabels[16], hEqOwn[16], hEqEnch[16];
static HWND hEqGrpBoxes[2];
static HWND hGridTabBtns[4], hGridTypeLabels[4];
static HWND hGridCells[4][30], hGridAllOn[4], hGridAllOff[4];
static HWND hTabBtns[NTABS], hStatusBar, hMainWnd;
static int currentTab = 0, currentGridSet = 0;
static int scrollY = 0, contentH = 900;

/* ---- Forward declarations ---- */
static void refreshGridUI(void);
static void showTab(int tab);

/* ---- Read/write ---- */
static DWORD readField(Field *f) {
    int o = f->offset;
    switch (f->type) {
        case T_U8: case T_S8: case T_SELECT: return saveData[o];
        case T_U16: return saveData[o] | (saveData[o+1]<<8);
        case T_U32: return saveData[o]|(saveData[o+1]<<8)|(saveData[o+2]<<16)|(saveData[o+3]<<24);
        case T_F32: { float v; memcpy(&v, &saveData[o], 4); return (DWORD)(int)v; }
        case T_F64: { double v; memcpy(&v, &saveData[o], 8); return (DWORD)(int)v; }
        default: return 0;
    }
}

static void writeField(Field *f, DWORD val) {
    int o = f->offset;
    switch (f->type) {
        case T_U8: case T_SELECT: saveData[o] = (BYTE)val; break;
        case T_S8: saveData[o] = (BYTE)(val > 127 ? 127 : val); break;
        case T_U16: if(val>32767)val=32767; saveData[o]=val&0xFF; saveData[o+1]=(val>>8)&0xFF; break;
        case T_U32: saveData[o]=val&0xFF; saveData[o+1]=(val>>8)&0xFF; saveData[o+2]=(val>>16)&0xFF; saveData[o+3]=(val>>24)&0xFF; break;
        case T_F32: { float v=(float)(int)val; memcpy(&saveData[o],&v,4); break; }
        case T_F64: { double v=(double)(int)val; memcpy(&saveData[o],&v,8); break; }
        default: break;
    }
}

/* ---- Scroll ---- */
static void updateScroll(HWND hwnd) {
    RECT rc; GetClientRect(hwnd, &rc);
    SCROLLINFO si = {sizeof(si), SIF_RANGE|SIF_PAGE|SIF_POS, 0, contentH, (UINT)rc.bottom, scrollY, 0};
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

static void doScroll(HWND hwnd, int newY) {
    RECT rc; GetClientRect(hwnd, &rc);
    int maxY = contentH - rc.bottom;
    if (maxY < 0) maxY = 0;
    if (newY < 0) newY = 0;
    if (newY > maxY) newY = maxY;
    if (newY != scrollY) {
        int dy = scrollY - newY;
        scrollY = newY;
        ScrollWindow(hwnd, 0, dy, NULL, NULL);
        updateScroll(hwnd);
        UpdateWindow(hwnd);
    }
}

/* ---- Status ---- */
static void setStatus(const wchar_t *msg) { SetWindowTextW(hStatusBar, msg); }

/* ---- Populate/Collect ---- */
static void populateFields(void) {
    if (!fileLoaded) return;
    int ci = 0;
    for (int t = 0; t < NTABS; t++) {
        TabDef *td = &tabs[t];
        if (!td->groups) continue;
        for (int g = 0; g < td->ngroups; g++) {
            GrpDef *gd = &td->groups[g];
            for (int i = 0; i < gd->nfields; i++, ci++) {
                Field *f = &gd->fields[i];
                if (f->type == T_STR) {
                    char buf[64]={0}; memcpy(buf,&saveData[f->offset],40); buf[40]=0;
                    SetWindowTextA(hFieldEdits[ci], buf);
                } else if (f->type == T_SELECT) {
                    SendMessageA(hFieldEdits[ci], CB_SETCURSEL, readField(f), 0);
                } else {
                    char buf[32]; sprintf(buf, "%u", readField(f));
                    SetWindowTextA(hFieldEdits[ci], buf);
                }
            }
        }
    }
    for (int i = 0; i < 16; i++) {
        int ownOff = i<8 ? EQ_WEP_OWN+i : EQ_ARM_OWN+(i-8);
        int enchOff = i<8 ? EQ_WEP_ENCH+i : EQ_ARM_ENCH+(i-8);
        SendMessage(hEqOwn[i], BM_SETCHECK, saveData[ownOff]?BST_CHECKED:BST_UNCHECKED, 0);
        char buf[16]; sprintf(buf,"%u",saveData[enchOff]);
        SetWindowTextA(hEqEnch[i], buf);
    }
    refreshGridUI();
}

static void collectFields(void) {
    if (!fileLoaded) return;
    int ci = 0;
    for (int t = 0; t < NTABS; t++) {
        TabDef *td = &tabs[t];
        if (!td->groups) continue;
        for (int g = 0; g < td->ngroups; g++) {
            GrpDef *gd = &td->groups[g];
            for (int i = 0; i < gd->nfields; i++, ci++) {
                Field *f = &gd->fields[i];
                if (f->type == T_STR) {
                    char buf[64]={0}; GetWindowTextA(hFieldEdits[ci],buf,41);
                    memset(&saveData[f->offset],0,40);
                    memcpy(&saveData[f->offset],buf,strlen(buf));
                } else if (f->type == T_SELECT) {
                    int sel=(int)SendMessageA(hFieldEdits[ci],CB_GETCURSEL,0,0);
                    if (sel>=0) writeField(f,sel);
                } else {
                    char buf[32]; GetWindowTextA(hFieldEdits[ci],buf,sizeof(buf));
                    writeField(f, strtoul(buf,NULL,10));
                }
            }
        }
    }
    for (int i = 0; i < 16; i++) {
        int ownOff = i<8 ? EQ_WEP_OWN+i : EQ_ARM_OWN+(i-8);
        int enchOff = i<8 ? EQ_WEP_ENCH+i : EQ_ARM_ENCH+(i-8);
        saveData[ownOff] = (SendMessage(hEqOwn[i],BM_GETCHECK,0,0)==BST_CHECKED)?1:0;
        char buf[16]; GetWindowTextA(hEqEnch[i],buf,sizeof(buf));
        int v=atoi(buf); if(v>127)v=127; if(v<0)v=0;
        saveData[enchOff]=(BYTE)v;
    }
}

/* ---- File I/O ---- */
static BOOL doOpen(HWND hwnd) {
    OPENFILENAMEA ofn={0}; char path[MAX_PATH]={0};
    ofn.lStructSize=sizeof(ofn); ofn.hwndOwner=hwnd;
    ofn.lpstrFilter="Save Files (*.*)\0*.*\0"; ofn.lpstrFile=path;
    ofn.nMaxFile=MAX_PATH; ofn.Flags=OFN_FILEMUSTEXIST;
    if (!GetOpenFileNameA(&ofn)) return FALSE;
    FILE *fp=fopen(path,"rb"); if(!fp){MessageBoxA(hwnd,"Cannot open","Error",MB_OK);return FALSE;}
    fseek(fp,0,SEEK_END); long sz=ftell(fp); fseek(fp,0,SEEK_SET);
    if (sz!=SAVE_SIZE) {
        char msg[128]; sprintf(msg,"Expected %d bytes, got %ld.",SAVE_SIZE,sz);
        MessageBoxA(hwnd,msg,"Warning",MB_OK|MB_ICONWARNING);
        if(sz<SAVE_SIZE){fclose(fp);return FALSE;}
    }
    fread(saveData,1,SAVE_SIZE,fp); fclose(fp);
    memcpy(origData,saveData,SAVE_SIZE); strcpy(filePath,path);
    fileLoaded=TRUE; populateFields();
    wchar_t status[256], wpath[MAX_PATH];
    MultiByteToWideChar(CP_ACP,0,path,-1,wpath,MAX_PATH);
    swprintf(status,256,L"Loaded: %s",wpath); setStatus(status);
    return TRUE;
}

static void doSave(HWND hwnd, BOOL saveAs) {
    if(!fileLoaded) return; collectFields();
    char path[MAX_PATH]; strcpy(path,filePath);
    if (saveAs||path[0]==0) {
        OPENFILENAMEA ofn={0}; ofn.lStructSize=sizeof(ofn); ofn.hwndOwner=hwnd;
        ofn.lpstrFilter="All Files (*.*)\0*.*\0"; ofn.lpstrFile=path;
        ofn.nMaxFile=MAX_PATH; ofn.Flags=OFN_OVERWRITEPROMPT;
        if(!GetSaveFileNameA(&ofn)) return;
    }
    FILE *fp=fopen(path,"wb"); if(!fp){MessageBoxA(hwnd,"Cannot write","Error",MB_OK);return;}
    fwrite(saveData,1,SAVE_SIZE,fp); fclose(fp);
    memcpy(origData,saveData,SAVE_SIZE); strcpy(filePath,path);
    setStatus(L"Saved!");
}

/* ---- Grid helpers ---- */
static void refreshGridUI(void) {
    if (!fileLoaded) return;
    int *bases = grid_bases[currentGridSet];
    int isWand = grid_is_wand[currentGridSet];
    for (int t = 0; t < 4; t++) {
        for (int s = 0; s < 30; s++) {
            BYTE v = saveData[bases[t]+s];
            char buf[8];
            if (isWand) { if(v==0) strcpy(buf,"-"); else if(v<=9){buf[0]='0'+v;buf[1]=0;} else strcpy(buf,"+"); }
            else strcpy(buf, v ? "\xFE" : "-");
            SetWindowTextA(hGridCells[t][s], buf);
        }
    }
}

static void onGridCellClick(int t, int s) {
    if (!fileLoaded) return;
    int *bases=grid_bases[currentGridSet]; int off=bases[t]+s;
    if (grid_is_wand[currentGridSet]) { BYTE v=saveData[off]+1; if(v>=10)v=0; saveData[off]=v; }
    else saveData[off]=saveData[off]?0:1;
    refreshGridUI();
}

static void onGridAllOn(int t) {
    if(!fileLoaded) return; int *bases=grid_bases[currentGridSet];
    for(int s=0;s<30;s++) saveData[bases[t]+s]=grid_is_wand[currentGridSet]?9:1;
    refreshGridUI();
}

static void onGridAllOff(int t) {
    if(!fileLoaded) return; int *bases=grid_bases[currentGridSet];
    for(int s=0;s<30;s++) saveData[bases[t]+s]=0;
    refreshGridUI();
}

/* ---- Create controls ---- */
static void createControls(HWND hwnd) {
    HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    hFont = CreateFontA(15,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Segoe UI");
    hFontBold = CreateFontA(15,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Segoe UI");
    hFontMono = CreateFontA(14,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Consolas");
    hFontGrp = CreateFontA(16,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Segoe UI");

    /* Toolbar */
    struct { const char *text; int id; int x; } btns[] = {
        {"Open",ID_OPEN,LM}, {"Save",ID_SAVE,LM+85}, {"Save As",ID_SAVEAS,LM+170}
    };
    for (int i=0;i<3;i++) {
        HWND h=CreateWindowA("BUTTON",btns[i].text,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
            btns[i].x,8,75,28,hwnd,(HMENU)(LONG_PTR)btns[i].id,hInst,NULL);
        SendMessage(h,WM_SETFONT,(WPARAM)hFont,0);
    }

    /* Tab buttons */
    for (int i=0;i<NTABS;i++) {
        char lbl[32]; strcpy(lbl, tabs[i].name);
        /* Fix && display */
        hTabBtns[i] = CreateWindowA("BUTTON",lbl,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
            LM+280+i*120,8,110,28,hwnd,(HMENU)(LONG_PTR)(ID_TAB_BASE+i),hInst,NULL);
        SendMessage(hTabBtns[i],WM_SETFONT,(WPARAM)hFont,0);
    }

    hStatusBar = CreateWindowA("STATIC","No file loaded",WS_CHILD|WS_VISIBLE|SS_RIGHT,
        LM+920,14,300,20,hwnd,NULL,hInst,NULL);
    SendMessage(hStatusBar,WM_SETFONT,(WPARAM)hFont,0);

    /* ---- Field-based tabs (0,1,2) ---- */
    int ci = 0; /* control index */
    nGrpBoxes = 0;
    for (int t = 0; t < NTABS; t++) {
        TabDef *td = &tabs[t];
        if (!td->groups) continue;

        /* Layout: 2 columns, groups fill left then right */
        int col = 0, y = TOP_Y;
        int colX[2] = {LM, LM + COL_W + COL_GAP};
        int colY[2] = {TOP_Y, TOP_Y};

        for (int g = 0; g < td->ngroups; g++) {
            GrpDef *gd = &td->groups[g];
            int grpH = GRP_HDR + gd->nfields * (ROW_H + 4) + GRP_PAD;

            /* Pick column with less content */
            col = (colY[0] <= colY[1]) ? 0 : 1;
            int gx = colX[col];
            int gy = colY[col];

            /* Group box */
            hGrpBoxes[nGrpBoxes] = CreateWindowA("BUTTON", gd->name,
                WS_CHILD|BS_GROUPBOX,
                gx, gy, COL_W, grpH, hwnd, NULL, hInst, NULL);
            SendMessage(hGrpBoxes[nGrpBoxes], WM_SETFONT, (WPARAM)hFontGrp, 0);
            nGrpBoxes++;

            int fy = gy + GRP_HDR;
            for (int i = 0; i < gd->nfields; i++, ci++) {
                Field *f = &gd->fields[i];
                char lbl[64]; sprintf(lbl, "%s", f->name);

                hFieldLabels[ci] = CreateWindowA("STATIC", lbl, WS_CHILD|SS_RIGHT,
                    gx+8, fy+3, LABEL_W-10, ROW_H, hwnd, NULL, hInst, NULL);
                SendMessage(hFieldLabels[ci], WM_SETFONT, (WPARAM)hFont, 0);

                if (f->type == T_SELECT) {
                    hFieldEdits[ci] = CreateWindowA("COMBOBOX","",
                        WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL,
                        gx+LABEL_W+4,fy,EDIT_W,200,hwnd,
                        (HMENU)(LONG_PTR)(ID_FIELD_BASE+ci),hInst,NULL);
                    SendMessage(hFieldEdits[ci],WM_SETFONT,(WPARAM)hFont,0);
                    for(int j=0;j<f->nopts;j++)
                        SendMessageA(hFieldEdits[ci],CB_ADDSTRING,0,(LPARAM)f->opts[j]);
                } else if (f->type == T_STR) {
                    hFieldEdits[ci] = CreateWindowA("EDIT","",
                        WS_CHILD|WS_BORDER|ES_AUTOHSCROLL,
                        gx+LABEL_W+4,fy,EDIT_W,ROW_H,hwnd,
                        (HMENU)(LONG_PTR)(ID_FIELD_BASE+ci),hInst,NULL);
                    SendMessage(hFieldEdits[ci],WM_SETFONT,(WPARAM)hFont,0);
                    SendMessage(hFieldEdits[ci],EM_SETLIMITTEXT,39,0);
                } else {
                    hFieldEdits[ci] = CreateWindowA("EDIT","0",
                        WS_CHILD|WS_BORDER|ES_NUMBER|ES_RIGHT,
                        gx+LABEL_W+4,fy,EDIT_W,ROW_H,hwnd,
                        (HMENU)(LONG_PTR)(ID_FIELD_BASE+ci),hInst,NULL);
                    SendMessage(hFieldEdits[ci],WM_SETFONT,(WPARAM)hFontMono,0);
                }
                fy += ROW_H + 4;
            }
            colY[col] = gy + grpH + 8;
        }
    }
    nFieldControls = ci;

    /* ---- Equipment tab ---- */
    int eqY = TOP_Y;
    const char *eqSections[] = {"Weapons", "Armor"};
    const char **eqNames[] = {wep_names, arm_names};
    for (int sec = 0; sec < 2; sec++) {
        int baseIdx = sec * 8;
        int gx = LM + sec * (COL_W + COL_GAP);
        int grpH = GRP_HDR + 8*(ROW_H+4) + GRP_PAD + 20;
        hEqGrpBoxes[sec] = CreateWindowA("BUTTON", eqSections[sec],
            WS_CHILD|BS_GROUPBOX, gx, eqY, COL_W, grpH, hwnd, NULL, hInst, NULL);
        SendMessage(hEqGrpBoxes[sec],WM_SETFONT,(WPARAM)hFontGrp,0);

        /* Column headers */
        int fy = eqY + GRP_HDR;
        CreateWindowA("STATIC","Name",WS_CHILD|SS_LEFT,gx+12,fy,100,18,hwnd,NULL,hInst,NULL);
        HWND hh1=CreateWindowA("STATIC","Own",WS_CHILD|SS_CENTER,gx+190,fy,40,18,hwnd,NULL,hInst,NULL);
        HWND hh2=CreateWindowA("STATIC","Ench",WS_CHILD|SS_RIGHT,gx+240,fy,60,18,hwnd,NULL,hInst,NULL);
        SendMessage(hh1,WM_SETFONT,(WPARAM)hFontBold,0);
        SendMessage(hh2,WM_SETFONT,(WPARAM)hFontBold,0);
        fy += 22;

        for (int i = 0; i < 8; i++) {
            int idx = baseIdx + i;
            char lbl[32]; sprintf(lbl, "%d. %s", i+1, eqNames[sec][i]);
            hEqLabels[idx] = CreateWindowA("STATIC",lbl,WS_CHILD|SS_LEFT,
                gx+12,fy+3,170,ROW_H,hwnd,NULL,hInst,NULL);
            SendMessage(hEqLabels[idx],WM_SETFONT,(WPARAM)hFont,0);
            hEqOwn[idx] = CreateWindowA("BUTTON","",WS_CHILD|BS_AUTOCHECKBOX,
                gx+200,fy+3,20,20,hwnd,(HMENU)(LONG_PTR)(ID_EQ_BASE+idx),hInst,NULL);
            hEqEnch[idx] = CreateWindowA("EDIT","0",WS_CHILD|WS_BORDER|ES_NUMBER|ES_RIGHT,
                gx+240,fy,60,ROW_H,hwnd,(HMENU)(LONG_PTR)(ID_EQ_BASE+100+idx),hInst,NULL);
            SendMessage(hEqEnch[idx],WM_SETFONT,(WPARAM)hFontMono,0);
            fy += ROW_H + 4;
        }
    }

    /* ---- Grid tab ---- */
    int gy = TOP_Y;
    for (int i=0;i<4;i++) {
        hGridTabBtns[i] = CreateWindowA("BUTTON",grid_names[i],WS_CHILD|BS_PUSHBUTTON,
            LM+i*100,gy,90,26,hwnd,(HMENU)(LONG_PTR)(ID_GRID_TAB+i),hInst,NULL);
        SendMessage(hGridTabBtns[i],WM_SETFONT,(WPARAM)hFont,0);
    }
    gy += 36;
    for (int t=0;t<4;t++) {
        hGridTypeLabels[t] = CreateWindowA("STATIC",type_names[t],WS_CHILD|SS_LEFT,
            LM,gy,200,18,hwnd,NULL,hInst,NULL);
        SendMessage(hGridTypeLabels[t],WM_SETFONT,(WPARAM)hFontBold,0);
        gy += 20;
        for (int s=0;s<30;s++) {
            int row=s/15, col2=s%15;
            hGridCells[t][s] = CreateWindowA("BUTTON","-",WS_CHILD|BS_PUSHBUTTON|BS_CENTER,
                LM+col2*44, gy+row*28, 40, 24, hwnd,
                (HMENU)(LONG_PTR)(ID_GRID_CELL+t*30+s),hInst,NULL);
            SendMessage(hGridCells[t][s],WM_SETFONT,(WPARAM)hFontMono,0);
        }
        gy += 60;
        hGridAllOn[t] = CreateWindowA("BUTTON","All On",WS_CHILD|BS_PUSHBUTTON,
            LM,gy,65,22,hwnd,(HMENU)(LONG_PTR)(ID_GRID_ALLON+t),hInst,NULL);
        hGridAllOff[t] = CreateWindowA("BUTTON","All Off",WS_CHILD|BS_PUSHBUTTON,
            LM+75,gy,65,22,hwnd,(HMENU)(LONG_PTR)(ID_GRID_ALLOFF+t),hInst,NULL);
        SendMessage(hGridAllOn[t],WM_SETFONT,(WPARAM)hFont,0);
        SendMessage(hGridAllOff[t],WM_SETFONT,(WPARAM)hFont,0);
        gy += 30;
    }

    showTab(0);
}

/* ---- Tab switching ---- */
static void showTab(int tab) {
    currentTab = tab;
    /* Highlight active tab button */
    for (int i=0;i<NTABS;i++)
        InvalidateRect(hTabBtns[i],NULL,TRUE);

    /* Figure out which field controls belong to which tab */
    int ci = 0;
    for (int t = 0; t < NTABS; t++) {
        TabDef *td = &tabs[t];
        if (!td->groups) continue;
        BOOL vis = (t == tab);
        for (int g = 0; g < td->ngroups; g++) {
            for (int i = 0; i < td->groups[g].nfields; i++, ci++) {
                ShowWindow(hFieldLabels[ci], vis?SW_SHOW:SW_HIDE);
                ShowWindow(hFieldEdits[ci], vis?SW_SHOW:SW_HIDE);
            }
        }
    }

    /* Group boxes - figure out which tab each belongs to */
    int gi = 0;
    for (int t = 0; t < NTABS; t++) {
        TabDef *td = &tabs[t];
        if (!td->groups) continue;
        BOOL vis = (t == tab);
        for (int g = 0; g < td->ngroups; g++, gi++)
            ShowWindow(hGrpBoxes[gi], vis?SW_SHOW:SW_HIDE);
    }

    /* Equipment */
    BOOL showEq = (tab == TAB_EQUIP);
    for (int i=0;i<16;i++) {
        ShowWindow(hEqLabels[i], showEq?SW_SHOW:SW_HIDE);
        ShowWindow(hEqOwn[i], showEq?SW_SHOW:SW_HIDE);
        ShowWindow(hEqEnch[i], showEq?SW_SHOW:SW_HIDE);
    }
    for (int i=0;i<2;i++) ShowWindow(hEqGrpBoxes[i], showEq?SW_SHOW:SW_HIDE);

    /* Grids */
    BOOL showGr = (tab == TAB_GRIDS);
    for (int i=0;i<4;i++) {
        ShowWindow(hGridTabBtns[i], showGr?SW_SHOW:SW_HIDE);
        ShowWindow(hGridTypeLabels[i], showGr?SW_SHOW:SW_HIDE);
        ShowWindow(hGridAllOn[i], showGr?SW_SHOW:SW_HIDE);
        ShowWindow(hGridAllOff[i], showGr?SW_SHOW:SW_HIDE);
        for (int s=0;s<30;s++) ShowWindow(hGridCells[i][s], showGr?SW_SHOW:SW_HIDE);
    }

    /* Reset scroll */
    scrollY = 0;
    updateScroll(hMainWnd);
    InvalidateRect(hMainWnd, NULL, TRUE);
}

/* ---- WndProc ---- */
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        createControls(hwnd);
        return 0;
    case WM_SIZE:
        updateScroll(hwnd);
        return 0;
    case WM_VSCROLL: {
        RECT rc; GetClientRect(hwnd,&rc);
        int pos=scrollY;
        switch(LOWORD(wParam)){
            case SB_LINEUP:pos-=30;break; case SB_LINEDOWN:pos+=30;break;
            case SB_PAGEUP:pos-=rc.bottom;break; case SB_PAGEDOWN:pos+=rc.bottom;break;
            case SB_THUMBTRACK:pos=HIWORD(wParam);break;
            case SB_THUMBPOSITION:pos=HIWORD(wParam);break;
        }
        doScroll(hwnd,pos); return 0;
    }
    case WM_MOUSEWHEEL: {
        int delta=GET_WHEEL_DELTA_WPARAM(wParam);
        doScroll(hwnd, scrollY-delta/2); return 0;
    }
    case WM_COMMAND: {
        int id=LOWORD(wParam);
        if(id==ID_OPEN){doOpen(hwnd);return 0;}
        if(id==ID_SAVE){doSave(hwnd,FALSE);return 0;}
        if(id==ID_SAVEAS){doSave(hwnd,TRUE);return 0;}
        if(id>=ID_TAB_BASE&&id<ID_TAB_BASE+NTABS){collectFields();showTab(id-ID_TAB_BASE);return 0;}
        if(id>=ID_GRID_TAB&&id<ID_GRID_TAB+4){collectFields();currentGridSet=id-ID_GRID_TAB;refreshGridUI();return 0;}
        if(id>=ID_GRID_ALLON&&id<ID_GRID_ALLON+4){onGridAllOn(id-ID_GRID_ALLON);return 0;}
        if(id>=ID_GRID_ALLOFF&&id<ID_GRID_ALLOFF+4){onGridAllOff(id-ID_GRID_ALLOFF);return 0;}
        if(id>=ID_GRID_CELL&&id<ID_GRID_CELL+120){int idx=id-ID_GRID_CELL;onGridCellClick(idx/30,idx%30);return 0;}
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdc=(HDC)wParam;
        SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        return (LRESULT)GetSysColorBrush(COLOR_3DFACE);
    }
    case WM_DESTROY:
        DeleteObject(hFont); DeleteObject(hFontBold);
        DeleteObject(hFontMono); DeleteObject(hFontGrp);
        PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmdLine, int showCmd) {
    (void)hPrev; (void)cmdLine;
    WNDCLASSW wc={0};
    wc.lpfnWndProc=WndProc; wc.hInstance=hInst;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground=(HBRUSH)(COLOR_3DFACE+1);
    wc.lpszClassName=L"MWEditor";
    wc.hIcon=LoadIcon(NULL,IDI_APPLICATION);
    RegisterClassW(&wc);

    hMainWnd = CreateWindowW(L"MWEditor",APP_TITLE,
        WS_OVERLAPPEDWINDOW|WS_VSCROLL,
        CW_USEDEFAULT,CW_USEDEFAULT, 950, 760,
        NULL,NULL,hInst,NULL);
    ShowWindow(hMainWnd,showCmd);
    UpdateWindow(hMainWnd);

    MSG m;
    while(GetMessage(&m,NULL,0,0)){TranslateMessage(&m);DispatchMessage(&m);}
    return (int)m.wParam;
}
