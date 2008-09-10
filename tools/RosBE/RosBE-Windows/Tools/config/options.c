/*
 * PROJECT:     RosBE Options Dialog
 * LICENSE:     GNU General Public License v2. (see LICENSE.txt)
 * FILE:        Tools/config/options.c
 * PURPOSE:     Configuring RosBE
 * COPYRIGHT:   Copyright 2007 Maarten Bosma
 *              Copyright 2007 Pierre Schweitzer
 *
 */

#include "options.h"

typedef struct _OPTIONS_DLG
{
    HWND hwndDlg;
    HWND hwndToolTip;
    HICON hIcon;
    HFONT hFont;
    SETTINGS Settings;
} OPTIONS_DLG, *POPTIONS_DLG;

HINSTANCE hInstance;

BOOL CreateDir(HWND hwnd, WCHAR* dir)
{
    WCHAR msgerror[256];

    if (0 > (LONG)GetFileAttributes(dir))
    {
        if (CreateDirectory(dir, NULL) == 0)
        {
            LoadString(hInstance, MSG_DIREFAILED, msgerror, 256);
            MessageBox(hwnd, msgerror, NULL, MB_ICONERROR);
            return FALSE;
        }
    }
    return TRUE;
}

static
const
WCHAR*
getConfigFile() 
{
    static WCHAR filename[MAX_PATH];
    if (SHGetSpecialFolderPathW(NULL, filename, CSIDL_APPDATA, FALSE))
    {
        if ((wcslen(filename) + wcslen(L"\\RosBE\\rosbe-options.cmd")) < MAX_PATH)
            wcscat(filename, L"\\RosBE\\rosbe-options.cmd");
    }
    else
    {
        wcscpy(filename, L"rosbe-options.cmd");
    }
    return filename;
}


INT
WriteSettings(POPTIONS_DLG infoPtr)
{
    INT foreground, background;
    BOOL showtime, writelog, useccache, strip, nostrip, objstate, outstate,modules;
    WCHAR logdir[MAX_PATH], objdir[MAX_PATH], outdir[MAX_PATH], mingwpath[MAX_PATH], checkmgw[MAX_PATH];
    WCHAR msgerror[256];
    HANDLE hFile;
    FILE *pFile;

    showtime = (SendDlgItemMessage(infoPtr->hwndDlg, ID_SHOWBUILDTIME, BM_GETCHECK, 0, 0) == BST_CHECKED);
    writelog = (SendDlgItemMessage(infoPtr->hwndDlg, ID_SAVELOGS, BM_GETCHECK, 0, 0) == BST_CHECKED);
    useccache = (SendDlgItemMessage(infoPtr->hwndDlg, ID_USECCACHE, BM_GETCHECK, 0, 0) == BST_CHECKED);
    modules = (SendDlgItemMessage(infoPtr->hwndDlg, ID_MODULES, BM_GETCHECK, 0, 0) == BST_CHECKED);
    strip = (SendDlgItemMessageW(infoPtr->hwndDlg, ID_STRIP, BM_GETCHECK, 0, 0) == BST_CHECKED);
    nostrip = (SendDlgItemMessageW(infoPtr->hwndDlg, ID_NOSTRIP, BM_GETCHECK, 0, 0) == BST_CHECKED);
    objstate = (SendDlgItemMessage(infoPtr->hwndDlg, ID_OTHEROBJ, BM_GETCHECK, 0, 0) == BST_CHECKED);
    outstate = (SendDlgItemMessageW(infoPtr->hwndDlg, ID_OTHEROUT, BM_GETCHECK, 0, 0) == BST_CHECKED);
    foreground = (INT) SendDlgItemMessageW(infoPtr->hwndDlg, IDC_FONT, CB_GETCURSEL, 0, 0);
    background = (INT) SendDlgItemMessageW(infoPtr->hwndDlg, IDC_BACK, CB_GETCURSEL, 0, 0);
    GetDlgItemTextW(infoPtr->hwndDlg, ID_LOGDIR, logdir, MAX_PATH);
    GetDlgItemTextW(infoPtr->hwndDlg, ID_MGWDIR, mingwpath, MAX_PATH);
    GetDlgItemTextW(infoPtr->hwndDlg, ID_OBJDIR, objdir, MAX_PATH);
    GetDlgItemTextW(infoPtr->hwndDlg, ID_OUTDIR, outdir, MAX_PATH);

    if (writelog && (logdir[0] != 0))
        if (!CreateDir(infoPtr->hwndDlg, logdir))
            return FALSE;

    if (objstate && (objdir[0] != 0))
        if (!CreateDir(infoPtr->hwndDlg, objdir))
            return FALSE;

    if (outstate && (outdir[0] != 0))
        if (!CreateDir(infoPtr->hwndDlg, outdir))
            return FALSE;

    wcscpy(checkmgw, mingwpath);
    if ((wcslen(checkmgw) + wcslen(L"\\bin\\gcc.exe")) < MAX_PATH)
        wcscat(checkmgw, L"\\bin\\gcc.exe");
    hFile = CreateFile(checkmgw, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        LoadString(hInstance, MSG_NOGCCFOUND, msgerror, 256);
        MessageBox(infoPtr->hwndDlg, msgerror, NULL, MB_ICONERROR);
        return FALSE;
    }
    CloseHandle(hFile);

    pFile = _wfopen(getConfigFile(), L"w");
    if (pFile)
    {
        fwprintf(pFile, L"::\n");
        fwprintf(pFile, L":: This file has been automatically generated by the ReactOS\n");
        fwprintf(pFile, L":: Build Environment options utility.\n");
        fwprintf(pFile, L"::\n\n");
        fwprintf(pFile, L"color %X%X\n", background, foreground);
        fwprintf(pFile, L"set _ROSBE_SHOWTIME=%d\n", showtime);
        fwprintf(pFile, L"set _ROSBE_USECCACHE=%d\n", useccache);
        fwprintf(pFile, L"set _ROSBE_STRIP=%d\n", strip);
        fwprintf(pFile, L"set _ROSBE_NOSTRIP=%d\n", nostrip);
        fwprintf(pFile, L"set _ROSBE_WRITELOG=%d\n", writelog);
        fwprintf(pFile, L"set _ROSBE_MODULES=%d\n", modules);
        if (logdir[0] != 0) fwprintf(pFile, L"set _ROSBE_LOGDIR=%s\n", logdir);
        if (mingwpath[0] != 0) fwprintf(pFile, L"set _ROSBE_MINGWPATH=%s\n", mingwpath);
        if ((objdir[0] != 0) && objstate) fwprintf(pFile, L"set _ROSBE_OBJPATH=%s\n", objdir);
        if ((outdir[0] != 0) && outstate) fwprintf(pFile, L"set _ROSBE_OUTPATH=%s\n", outdir);
        fclose(pFile);
        return TRUE;
    }
    LoadString(hInstance, MSG_FILEFAILED, msgerror, 256);
    MessageBox(infoPtr->hwndDlg, msgerror, NULL, MB_ICONERROR);
    return FALSE;
}

VOID LoadSettings(POPTIONS_DLG infoPtr)
{
    FILE *pFile;
    WCHAR *ptr, *ptr2;
    WCHAR WTempLine[25+MAX_PATH];
    WCHAR SBTitle[256];
    PSETTINGS LoadedSettings = &infoPtr->Settings;

    pFile = _wfopen(getConfigFile(), L"r");
    if (pFile)
    {
        while (fgetws(WTempLine, 24+MAX_PATH, pFile))
        {
            ptr = wcstok(WTempLine, L" ");
            if (wcscmp(ptr, L"color") == 0)
            {
                ptr = wcstok(NULL, L" ");
                ptr[2] = ptr[1];
                ptr[1] = 0;
                LoadedSettings->background = wcstol(&ptr[0], NULL, 16);
                LoadedSettings->foreground = wcstol(&ptr[2], NULL, 16);
            }
            else if (wcscmp(ptr, L"set") == 0)
            {
                ptr = wcstok(NULL, L" ");
                ptr = wcstok(ptr, L"=");
                ptr2 = wcstok(NULL, L"=");
                if (wcscmp(ptr, L"_ROSBE_SHOWTIME") == 0)
                    LoadedSettings->showtime = wcstol(ptr2, NULL, 2);
                else if (wcscmp(ptr, L"_ROSBE_USECCACHE") == 0)
                    LoadedSettings->useccache = wcstol(ptr2, NULL, 2);
                else if (wcscmp(ptr, L"_ROSBE_STRIP") == 0)
                    LoadedSettings->strip = wcstol(ptr2, NULL, 2);
                else if (wcscmp(ptr, L"_ROSBE_NOSTRIP") == 0)
                    LoadedSettings->nostrip = wcstol(ptr2, NULL, 2);
                else if (wcscmp(ptr, L"_ROSBE_MODULES") == 0)
                    LoadedSettings->modules = wcstol(ptr2, NULL, 2);
                else if (wcscmp(ptr, L"_ROSBE_WRITELOG") == 0)
                    LoadedSettings->writelog = wcstol(ptr2, NULL, 2);
                else if (wcscmp(ptr, L"_ROSBE_LOGDIR") == 0)
                    wcsncpy(LoadedSettings->logdir, ptr2, wcslen(ptr2)-1);
                else if (wcscmp(ptr, L"_ROSBE_MINGWPATH") == 0)
                    wcsncpy(LoadedSettings->mingwpath, ptr2, wcslen(ptr2)-1);
                else if (wcscmp(ptr, L"_ROSBE_OBJPATH") == 0)
                    wcsncpy(LoadedSettings->objdir, ptr2, wcslen(ptr2)-1);
                else if (wcscmp(ptr, L"_ROSBE_OUTPATH") == 0)
                    wcsncpy(LoadedSettings->outdir, ptr2, wcslen(ptr2)-1);
            }
        }
        fclose(pFile);
        LoadString(hInstance, MSG_SETLOADSUC, SBTitle, 256);
        SetDlgItemText(infoPtr->hwndDlg, ID_STATUSBAR, SBTitle);
    }
    else
    {
        LoadedSettings->foreground = 0xa;
        LoadedSettings->background = 0;
        GetCurrentDirectory(MAX_PATH, LoadedSettings->mingwpath);
        if ((wcslen(LoadedSettings->mingwpath) + wcslen(MINGWVERSION)) < MAX_PATH)
            wcscat(LoadedSettings->mingwpath, MINGWVERSION);

        LoadString(hInstance, HLP_DEFAULTMSG, SBTitle, 256);
        SetDlgItemText(infoPtr->hwndDlg, ID_STATUSBAR, SBTitle);
    }
    SendDlgItemMessageW(infoPtr->hwndDlg, IDC_FONT, CB_SETCURSEL, LoadedSettings->foreground, 0);
    SendDlgItemMessageW(infoPtr->hwndDlg, IDC_BACK, CB_SETCURSEL, LoadedSettings->background, 0);
    SendDlgItemMessage(infoPtr->hwndDlg, ID_SHOWBUILDTIME, BM_SETCHECK, LoadedSettings->showtime, 0);
    SendDlgItemMessage(infoPtr->hwndDlg, ID_MODULES, BM_SETCHECK, LoadedSettings->modules, 0);
    SendDlgItemMessage(infoPtr->hwndDlg, ID_SAVELOGS, BM_SETCHECK, LoadedSettings->writelog, 0);
    if (LoadedSettings->writelog)
    {
        EnableWindow(GetDlgItem(infoPtr->hwndDlg, ID_BROWSE), TRUE);
        EnableWindow(GetDlgItem(infoPtr->hwndDlg, ID_LOGDIR), TRUE);
    }
    SendDlgItemMessage(infoPtr->hwndDlg, ID_USECCACHE, BM_SETCHECK, LoadedSettings->useccache, 0);
    SendDlgItemMessageW(infoPtr->hwndDlg, ID_STRIP, BM_SETCHECK, LoadedSettings->strip, 0);
    SendDlgItemMessageW(infoPtr->hwndDlg, ID_NOSTRIP, BM_SETCHECK, LoadedSettings->nostrip, 0);
    SetDlgItemText(infoPtr->hwndDlg, ID_MGWDIR, LoadedSettings->mingwpath);
    SetDlgItemText(infoPtr->hwndDlg, ID_LOGDIR, LoadedSettings->logdir);
    if (LoadedSettings->objdir[0] != 0)
    {
        LoadedSettings->objstate = 1;
        SetDlgItemText(infoPtr->hwndDlg, ID_OBJDIR, LoadedSettings->objdir);
        SendDlgItemMessage(infoPtr->hwndDlg, ID_OTHEROBJ, BM_SETCHECK, BST_CHECKED, 0);
        EnableWindow(GetDlgItem(infoPtr->hwndDlg, ID_BROWSEOBJ), TRUE);
        EnableWindow(GetDlgItem(infoPtr->hwndDlg, ID_OBJDIR), TRUE);
    }
    if (LoadedSettings->outdir[0] != 0)
    {
        LoadedSettings->outstate = 1;
        SetDlgItemText(infoPtr->hwndDlg, ID_OUTDIR, LoadedSettings->outdir);
        SendDlgItemMessage(infoPtr->hwndDlg, ID_OTHEROUT, BM_SETCHECK, BST_CHECKED, 0);
        EnableWindow(GetDlgItem(infoPtr->hwndDlg, ID_BROWSEOUT), TRUE);
        EnableWindow(GetDlgItem(infoPtr->hwndDlg, ID_OUTDIR), TRUE);
    }
}

VOID SetSaveState(POPTIONS_DLG infoPtr)
{
    INT foreground, background;
    BOOL showtime, writelog, useccache, strip, nostrip, objstate, outstate,modules;
    WCHAR logdir[MAX_PATH], objdir[MAX_PATH], outdir[MAX_PATH], mingwpath[MAX_PATH];
    BOOL StateObj = TRUE, StateOut = TRUE, StateLog = TRUE, State = TRUE;
    PSETTINGS DefaultSettings = &infoPtr->Settings;

    showtime = (SendDlgItemMessage(infoPtr->hwndDlg, ID_SHOWBUILDTIME, BM_GETCHECK, 0, 0) == BST_CHECKED);
    writelog = (SendDlgItemMessage(infoPtr->hwndDlg, ID_SAVELOGS, BM_GETCHECK, 0, 0) == BST_CHECKED);
    useccache = (SendDlgItemMessage(infoPtr->hwndDlg, ID_USECCACHE, BM_GETCHECK, 0, 0) == BST_CHECKED);
    strip = (SendDlgItemMessageW(infoPtr->hwndDlg, ID_STRIP, BM_GETCHECK, 0, 0) == BST_CHECKED);
    nostrip = (SendDlgItemMessageW(infoPtr->hwndDlg, ID_NOSTRIP, BM_GETCHECK, 0, 0) == BST_CHECKED);
    modules = (SendDlgItemMessageW(infoPtr->hwndDlg, ID_MODULES, BM_GETCHECK, 0, 0) == BST_CHECKED);
    objstate = (SendDlgItemMessageW(infoPtr->hwndDlg, ID_OTHEROBJ, BM_GETCHECK, 0, 0) == BST_CHECKED);
    outstate = (SendDlgItemMessageW(infoPtr->hwndDlg, ID_OTHEROUT, BM_GETCHECK, 0, 0) == BST_CHECKED);
    foreground = (INT) SendDlgItemMessageW(infoPtr->hwndDlg, IDC_FONT, CB_GETCURSEL, 0, 0);
    background = (INT) SendDlgItemMessageW(infoPtr->hwndDlg, IDC_BACK, CB_GETCURSEL, 0, 0);
    GetDlgItemTextW(infoPtr->hwndDlg, ID_LOGDIR, logdir, MAX_PATH);
    GetDlgItemTextW(infoPtr->hwndDlg, ID_MGWDIR, mingwpath, MAX_PATH);
    GetDlgItemTextW(infoPtr->hwndDlg, ID_OBJDIR, objdir, MAX_PATH);
    GetDlgItemTextW(infoPtr->hwndDlg, ID_OUTDIR, outdir, MAX_PATH);

    if (objstate)
        if ((wcscmp(objdir, DefaultSettings->objdir) != 0) && (wcslen(objdir) > 0))
            StateObj = FALSE;
    if (outstate)
        if ((wcscmp(outdir, DefaultSettings->outdir) != 0) && (wcslen(outdir) > 0))
            StateOut = FALSE;
    if (writelog)
        if ((wcscmp(logdir, DefaultSettings->logdir) != 0) && (wcslen(logdir) > 0))
            StateLog = FALSE;

    State ^= ((foreground == DefaultSettings->foreground) && (background == DefaultSettings->background) &&
            (showtime == DefaultSettings->showtime) && (writelog == DefaultSettings->writelog) &&
            (useccache == DefaultSettings->useccache) && (strip == DefaultSettings->strip) && (modules == DefaultSettings->modules) &&
            (objstate == DefaultSettings->objstate) && (outstate == DefaultSettings->outstate) &&
            (StateLog) && (wcscmp(mingwpath, DefaultSettings->mingwpath) == 0) &&
            (StateObj) && (StateOut) && (nostrip == DefaultSettings->nostrip));

    EnableWindow(GetDlgItem(infoPtr->hwndDlg, ID_OK), State);
}

INT CALLBACK
BrowseProc(HWND Dlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndParent;
    hwndParent = GetWindow(Dlg, GW_OWNER);
    switch (Msg)
    {
        case BFFM_INITIALIZED:
        {
            WCHAR ActualPath[MAX_PATH];
            GetDlgItemText(hwndParent, (INT) lParam, ActualPath, MAX_PATH);
            SendMessage(Dlg, BFFM_SETSELECTION, TRUE, (LPARAM)ActualPath);
            break;
        }
        case BFFM_VALIDATEFAILED:
        {
            if (lParam != ID_MGWDIR)
            {
                WCHAR BoxMsg[256], BoxTitle[128];
                size_t PathLen;
                LoadString(hInstance, MSG_WARNINGBOX, BoxTitle, 128);
                LoadString(hInstance, MSG_INVALIDDIR, BoxMsg, 256);
                if (MessageBox(Dlg, BoxMsg, BoxTitle, MB_ICONWARNING | MB_YESNO) == IDYES)
                {
                    PathLen = wcslen((LPWSTR)wParam);
                    if (wcscmp((LPWSTR)wParam+PathLen, L"\\"))
                        wcsset((LPWSTR)wParam+(PathLen-1), '\0');
                    if (CreateDirectory((LPWSTR)wParam, NULL) == 0)
                    {
                        LoadString(hInstance, MSG_DIREFAILED, BoxMsg, 256);
                        MessageBox(Dlg, BoxMsg, NULL, MB_ICONERROR);
                    }
                    else
                    {
                        SetDlgItemText(hwndParent, (INT) lParam, (LPWSTR)wParam);
                    }
                }
            }
            break;
        }
    }
    return FALSE;
}

static VOID
AssociateToolWithControl(POPTIONS_DLG infoPtr, int ControlId, UINT StringId)
{
    HWND hwndControl;
    TOOLINFO ti;

    hwndControl = GetDlgItem(infoPtr->hwndDlg, ControlId);
    if (infoPtr->hwndToolTip && hwndControl)
    {
        ZeroMemory(&ti, sizeof(ti));
        ti.cbSize = sizeof(ti);
        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        ti.hwnd = infoPtr->hwndDlg;
        ti.uId = (UINT_PTR)hwndControl;
        ti.hinst = hInstance;
        ti.lpszText = MAKEINTRESOURCE(StringId);

        SendMessage(infoPtr->hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    }
}

INT_PTR CALLBACK
DlgProc(HWND Dlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    POPTIONS_DLG infoPtr = NULL;

    if (Msg != WM_INITDIALOG)
    {
        infoPtr = (POPTIONS_DLG)GetWindowLongPtr(Dlg, DWLP_USER);
        if (infoPtr == NULL)
            return FALSE;
    }

    switch (Msg)
    {
        case WM_INITDIALOG:
        {
            static const struct {
                int ControlId;
                UINT StringId;
            } ToolTipAssociations[] = {
                {IDC_BACK, HLP_BACKCOLORC},
                {IDC_FONT, HLP_FONTCOLORC},
                {ID_SHOWBUILDTIME, HLP_SBUILDTIME},
                {ID_USECCACHE, HLP_CCACHEUSED},
                {ID_STRIP, HLP_STRIPEDEXE},
				{ID_NOSTRIP, HLP_NOSTRIP},
				{ID_MODULES, HLP_MODULES},
                {ID_MGWDIR, HLP_FINDMGWDIR},
                {ID_BROWSEMGW, HLP_FINDMGWDIR},
                {ID_LOGDIR, HLP_FINDLOGDIR},
                {ID_BROWSE, HLP_FINDLOGDIR},
                {ID_SAVELOGS, HLP_FINDLOGDIR},
                {ID_OBJDIR, HLP_FINDOBJDIR},
                {ID_BROWSEOBJ, HLP_FINDOBJDIR},
                {ID_OTHEROBJ, HLP_FINDOBJDIR},
                {ID_OUTDIR, HLP_FINDOUTDIR},
                {ID_BROWSEOUT, HLP_FINDOUTDIR},
                {ID_OTHEROUT, HLP_FINDOUTDIR},
                {ID_OK, HLP_SAVEBUTTON},
                {ID_CANCEL, HLP_QUITBUTTON},
            };
            int i;
            UINT j;
            LOGFONT lf;
            WCHAR TempColor[256];

            infoPtr = (POPTIONS_DLG)lParam;
            infoPtr->hwndDlg = Dlg;

            SetWindowLongPtr(Dlg, DWLP_USER, (LONG_PTR)infoPtr);

            infoPtr->hwndToolTip = CreateWindowEx(0,
                                                  TOOLTIPS_CLASS,
                                                  NULL,
                                                  WS_POPUP | TTS_ALWAYSTIP,
                                                  CW_USEDEFAULT,
                                                  CW_USEDEFAULT,
                                                  CW_USEDEFAULT,
                                                  CW_USEDEFAULT,
                                                  infoPtr->hwndDlg,
                                                  NULL,
                                                  hInstance,
                                                  NULL);

            for (i = 0; i < sizeof(ToolTipAssociations) / sizeof(ToolTipAssociations[0]); i++)
                AssociateToolWithControl(infoPtr, ToolTipAssociations[i].ControlId, ToolTipAssociations[i].StringId);

            infoPtr->hIcon = LoadImage( hInstance,
                                        MAKEINTRESOURCE(ID_OPTICON),
                                        IMAGE_ICON,
                                        GetSystemMetrics(SM_CXSMICON),
                                        GetSystemMetrics(SM_CYSMICON),
                                        0);
            if(infoPtr->hIcon)
                SendMessage(Dlg, WM_SETICON, ICON_SMALL, (LPARAM)infoPtr->hIcon);

            GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(LOGFONT),  &lf);
            lf.lfWeight = FW_BOLD;
            infoPtr->hFont = CreateFont(lf.lfHeight, lf.lfWidth,  lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
            lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet,  lf.lfOutPrecision,
            lf.lfClipPrecision, lf.lfQuality,  lf.lfPitchAndFamily, lf.lfFaceName);

            for(j = 0; j < 16; j++)
            {
                LoadString(hInstance, j, TempColor, 256);
                SendDlgItemMessageW(Dlg, IDC_BACK, CB_ADDSTRING, 0, (LPARAM) TempColor);
                SendDlgItemMessageW(Dlg, IDC_FONT, CB_ADDSTRING, 0, (LPARAM) TempColor);
            }
            LoadSettings(infoPtr);
            return TRUE;
        }

        case WM_COMMAND:
        {
            if ((HIWORD(wParam) == CBN_SELCHANGE) && ((LOWORD(wParam) == IDC_FONT) || (LOWORD(wParam) == IDC_BACK)))
            {
                InvalidateRect(GetDlgItem(Dlg, ID_EXAMPLE), NULL, FALSE);
            }
            else
            {
                switch (wParam)
                {
                    case ID_OK:
                    {
                       if (!WriteSettings(infoPtr))
                            break;
                    }
                    case ID_CANCEL:
                    {
                        WCHAR BoxMsg[256], BoxTitle[128];
                        if ((IsWindowEnabled(GetDlgItem(Dlg, ID_OK))) && (wParam == ID_CANCEL))
                        {
                            LoadString(hInstance, MSG_WARNINGBOX, BoxTitle, 128);
                            LoadString(hInstance, MSG_EXITCHANGE, BoxMsg, 256);
                            if (MessageBox(Dlg, BoxMsg, BoxTitle, MB_ICONWARNING | MB_YESNO) == IDNO)
                                break;
                        }
                        PostMessage(Dlg, WM_CLOSE, 0, 0);
                        break;
                    }
                    case ID_BROWSE:
                    case ID_BROWSEMGW:
                    case ID_BROWSEOBJ:
                    case ID_BROWSEOUT:
                    {
                        BROWSEINFO PathInfo;
                        LPITEMIDLIST pidl;
                        LPMALLOC pMalloc;
                        INT Control = ID_LOGDIR;
                        INT IDText = MSG_FINDLOGDIR;
                        WCHAR path[MAX_PATH];
                        WCHAR Text[512];

                        if (SHGetMalloc(&pMalloc) == NOERROR)
                        {
                            ZeroMemory(&PathInfo, sizeof(BROWSEINFO));
                            PathInfo.hwndOwner = Dlg;
                            PathInfo.ulFlags = BIF_EDITBOX | BIF_VALIDATE;
                            PathInfo.lpfn = (BFFCALLBACK)BrowseProc;
                            PathInfo.lParam = ID_LOGDIR;
                            PathInfo.pidlRoot = NULL;
                            if ((wParam == ID_BROWSEMGW) || (wParam == ID_BROWSEOBJ) || (wParam == ID_BROWSEOUT))
                            {
                                Control = ID_MGWDIR;
                                IDText = MSG_FINDMGWDIR;
                                if (wParam == ID_BROWSEOBJ)
                                {
                                    Control = ID_OBJDIR;
                                    IDText = MSG_FINDOBJDIR;
                                }
                                else if (wParam == ID_BROWSEOUT)
                                {
                                    Control = ID_OUTDIR;
                                    IDText = MSG_FINDOUTDIR;
                                }
                                PathInfo.lParam = Control;
                            }
                            LoadString(hInstance, IDText, Text, 512);
                            PathInfo.lpszTitle = Text;
                            if ((pidl = SHBrowseForFolder(&PathInfo)) != NULL)
                            {
                                if (SHGetPathFromIDList(pidl, path))
                                    SetDlgItemText(Dlg, Control, path);
                                pMalloc->lpVtbl->Free(pMalloc, pidl);
                            }
                            pMalloc->lpVtbl->Release(pMalloc);
                        }
                        break;
                    }
                    case ID_OTHEROBJ:
                    case ID_OTHEROUT:
                    case ID_SAVELOGS:
                    {
                        BOOL WriteLogSet;
                        INT Dialog1 = ID_BROWSE;
                        INT Dialog2 = ID_LOGDIR;
                        WriteLogSet = (SendDlgItemMessageW(Dlg, (INT)wParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
                        if (wParam == ID_OTHEROBJ)
                        {
                            Dialog1 = ID_BROWSEOBJ;
                            Dialog2 = ID_OBJDIR;
                        }
                        else if (wParam == ID_OTHEROUT)
                        {
                            Dialog1 = ID_BROWSEOUT;
                            Dialog2 = ID_OUTDIR;
                        }
                        EnableWindow(GetDlgItem(Dlg, Dialog1), WriteLogSet);
                        EnableWindow(GetDlgItem(Dlg, Dialog2), WriteLogSet);
                        break;
                    }
                }

            }
            SetSaveState(infoPtr);
            return FALSE;
        }

        case WM_CTLCOLORSTATIC:
        {
            HFONT hFontOld;
            // note: do not change the order - it matches to previous order
            static const COLORREF ColorsRGB[] = { 0x00000000, 0x00800000, 0x00008000, 0x00808000, 0x00000080, 0x00800080,
                0x00008080, 0x00c0c0c0, 0x00808080, 0x00ff0000, 0x0000ff00,
                0x00ffff00, 0x000000ff, 0x00ff00ff, 0x0000ffff, 0x00ffffff
            };

            if((HWND)lParam == GetDlgItem(Dlg, ID_EXAMPLE))
            {
                hFontOld = SelectObject((HDC)wParam, infoPtr->hFont);
                SetTextColor((HDC)wParam, ColorsRGB[SendDlgItemMessageW(Dlg, IDC_FONT, CB_GETCURSEL, 0, 0)]);
                SetBkColor((HDC)wParam, ColorsRGB[SendDlgItemMessageW(Dlg, IDC_BACK, CB_GETCURSEL, 0, 0)]);
                return (INT_PTR)CreateSolidBrush(ColorsRGB[SendDlgItemMessageW(Dlg, IDC_BACK, CB_GETCURSEL, 0, 0)]);
            }
            break;
        }

        case WM_DESTROY:
        {
            if (infoPtr->hIcon)
                DestroyIcon(infoPtr->hIcon);
            if (infoPtr->hFont)
                DeleteObject(infoPtr->hFont);

        }

        case WM_CLOSE:
        {
            EndDialog(Dlg, 0);
            return TRUE;
        }
    }
    return FALSE;
}

int WINAPI
WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdline, int cmdshow)
{
    POPTIONS_DLG OptionsDlgInfo;
    hInstance = hInst;

    OptionsDlgInfo = HeapAlloc(GetProcessHeap(), 0, sizeof(*OptionsDlgInfo));
    if (OptionsDlgInfo != NULL)
    {
        ZeroMemory(OptionsDlgInfo, sizeof(OPTIONS_DLG));
        DialogBoxParam(hInst, MAKEINTRESOURCE(ID_DIALOG), 0, DlgProc, (LPARAM)OptionsDlgInfo);
        HeapFree(GetProcessHeap(), 0, OptionsDlgInfo);
    }
    return 0;
}
