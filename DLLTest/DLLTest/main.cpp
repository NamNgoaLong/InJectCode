#include <windows.h>
#include "SimpleH.h"
#include "iostream"
#include <tlhelp32.h>
#include <tchar.h>
#include <vector>  
#include "strsafe.h"
#include <Psapi.h>
#pragma warning (disable : 4786)
#define IDC_LIST 1
#define IDC_STATIC 2
#define IDC_STATIC_LIST 3
#define IDC_STATIC_ID 4
#define IDC_STATIC_PATH 5
#define IDC_BT_OPENFILE 1001
#define IDC_BT_INJECT 1002
#define IDC_BT_CLOSE 1003
//Local Parametter
typedef struct _myDataType
{
	WCHAR lpExeName[MAX_PATH];
	DWORD processID;
} myDataType ;
typedef struct _dataTypePath
{
	WCHAR lpExeName[1024];
} typePath ;

void eventScroll( HWND hwnd, SCROLLINFO si, WPARAM wParam);
void createSrcoll(HWND hwnd, SCROLLINFO si, int nMin, int nMax);
LPTSTR  openFile(HWND hwnd);
HINSTANCE g_hinst;
std::vector<_myDataType> listExeName;
std::vector<_dataTypePath> listPathFile;

void notifiInt(int msg, int strlen);
void eventScroll( HWND hwnd, SCROLLINFO si, WPARAM wParam);
void createSrcoll(HWND hwnd, SCROLLINFO si, int nMin, int nMax);
LPTSTR  openFile(HWND hwnd);

void injectCode(myDataType process);
BOOL SetPrivilege( HANDLE hToken,LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
void notifi(LPCSTR msg);
void notifiInt(int msg, int strlen);
void notifiF(float msg, int strlen);
LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

int pX =10, pY =10, pW =750, pH =550;
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow )
{		
	char szMsg[256] = {0} ;
	sprintf_s( szMsg , 256 , " %d ", sizeof(int)) ;
	::OutputDebugStringA(szMsg) ;		

	HANDLE hToken = NULL ;
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	LPCTSTR lpszPrivilege = TEXT("SE_INCREASE_QUOTA_NAME");
	BOOL bEnablePrivilege = TRUE;
	BOOL bResult = OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken) ;
	BOOL bReSuleSet = SetPrivilege(hToken,lpszPrivilege, bEnablePrivilege);
	if(!bReSuleSet){
		char szMsg[256] = {0} ;
		sprintf_s( szMsg , 256 , " SetPrivilege() errer!") ;
		::OutputDebugStringA(szMsg) ;		
	}	
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE){
		char szMsg[256] = {0} ;
		sprintf_s( szMsg , 256 , " Errer open Snapshot!") ;
		::OutputDebugStringA(szMsg) ;
		return( FALSE );
	}
	pe32.dwSize = sizeof( PROCESSENTRY32 );
	BOOL cont = Process32First( hProcessSnap, &pe32 );	
	LPWSTR lpExeName = new WCHAR[1024];
	DWORD lpdwSize = 1024; 	
	while(cont)	{
		if((DWORD)pe32.th32ProcessID == 0){			
			notifi(" ID process =0");
		}else{
			myDataType newItem ;
			wcscpy( newItem.lpExeName , pe32.szExeFile); 
			newItem.processID = pe32.th32ProcessID;
			listExeName.push_back(newItem); 
/*
			HANDLE hProcessInject = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_QUERY_INFORMATION,FALSE,pe32.th32ProcessID);
			if(hProcessInject ==NULL){
				notifi(" OpenProcess...ERROR ");		
				notifiInt((int)GetLastError(),5);				
			}else{					

				//BOOL bResult = GetProcessImageFileName(hProcessInject, lpExeName, lpdwSize);
				BOOL bResult = QueryFullProcessImageName(hProcessInject,0,lpExeName,&lpdwSize);
				if(bResult==FALSE){
					notifi(" QueryFullProcessImageName...Error ");
					notifiInt( GetLastError(),5);
				}else{
					typePath p;
					wcscpy(p.lpExeName,lpExeName);
					listPathFile.push_back(p);
//						notifi(" QueryFullProcessImageName...OK ");
//						::OutputDebugString(listPathFile.back().lpExeName) ;		
				}	
			}
			//			injectCode((DWORD)pe32.th32ProcessID);
*/
		}

		cont = Process32Next(hProcessSnap, &pe32);		
	}	
	MSG msg ;
	WNDCLASS wc = {0};
	wc.lpszClassName = TEXT( "Application" );
	wc.hInstance = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpfnWndProc = WndProc;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	g_hinst = hInstance;
	RegisterClass(&wc);
	HWND hwndPer = CreateWindow( wc.lpszClassName, TEXT("Inject Code"),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		pX, pY, pW, pH, 0, 0, hInstance, 0);
	if(hwndPer == NULL){
		notifi("Create window Perrents ERROR ");
		notifiInt(GetLastError(),5);
	}
	ShowWindow(hwndPer, SW_SHOW);
	UpdateWindow(hwndPer);
	while( GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	CloseHandle( hProcessSnap );          // clean the snapshot object
	return( FALSE );
}
	int cX =40, cY =60, cW =300, cH =350;
	int bOkX = 380, bOkY =90;
	static int yPos;
	static int xPos;
	static int yChar = 8;
	static SCROLLINFO si;
	static typePath file;
	static HWND hwndList;
	static HWND hwndStaticID;
	static HWND hwndStaticPath;
	static HWND hwndStaticNotifi;
	static HWND hwndLbList;
	static HWND hwndLbPath;
	static HWND hwndLbID;
	static int sel =-1;
	static BOOL flag_openFile = FALSE;
	TCHAR buff[300];
LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ){	
	static int nMin = 0;
	static int nMax = 300;

	switch(msg){
	case WM_SIZE:
		createSrcoll(hwndList, si, nMin, nMax);
		break;
	case WM_CREATE:
		hwndLbList = CreateWindow(TEXT("static") , TEXT("List EXE Name:"), WS_CHILD |
			WS_VISIBLE | LBS_NOTIFY,
			cX, cY -30, 200, 50, hwnd,(HMENU) IDC_STATIC_LIST, g_hinst, NULL);
		if( hwndLbList == NULL){
			DWORD errorx = GetLastError();
			notifi(" Create child window Label List ERROR ");
			notifiInt(errorx ,5 );
		}
		hwndLbID = CreateWindow(TEXT("static") , TEXT("ID Process:"), WS_CHILD |
			WS_VISIBLE | LBS_NOTIFY,
			bOkX, bOkY +70, 80, 20, hwnd,(HMENU) IDC_STATIC_ID, g_hinst, NULL);
		if( hwndLbID == NULL){
			DWORD errorx = GetLastError();
			notifi(" Create child window Label ID ERROR ");
			notifiInt(errorx ,5 );
		}
		hwndStaticID = CreateWindow(TEXT("static") , NULL, WS_CHILD |
			WS_VISIBLE | LBS_NOTIFY,
			bOkX +110, bOkY +70, 30, 20, hwnd,(HMENU) IDC_STATIC, g_hinst, NULL);
		if( hwndStaticID == NULL){
			DWORD errorx = GetLastError();
			notifi(" Create child window static ID ERROR ");
			notifiInt(errorx ,5 );
		}
		hwndLbPath = CreateWindow(TEXT("static") , TEXT("Path File .dll:"), WS_CHILD |
			WS_VISIBLE | LBS_NOTIFY,
			bOkX -30, bOkY +120, 150, 20, hwnd,(HMENU) IDC_STATIC_PATH, g_hinst, NULL);
		if( hwndLbPath == NULL){
			DWORD errorx = GetLastError();
			notifi(" Create child window Label Path ERROR ");
			notifiInt(errorx ,5 );
		}
		hwndStaticPath = CreateWindow(TEXT("static") , NULL, WS_CHILD |
			WS_VISIBLE | LBS_NOTIFY,
			bOkX, bOkY +140, cW, 20, hwnd,(HMENU) IDC_STATIC_PATH, g_hinst, NULL);
		if( hwndStaticPath == NULL){
			DWORD errorx = GetLastError();
			notifi(" Create child window Static Path ERROR ");
			notifiInt(errorx ,5 );
		}
		hwndStaticNotifi = CreateWindow(TEXT("static") , NULL, WS_CHILD |
			WS_VISIBLE | LBS_NOTIFY,
			bOkX, bOkY +180, cW, 20, hwnd,(HMENU) IDC_STATIC_PATH, g_hinst, NULL);
		if( hwndStaticNotifi == NULL){
			DWORD errorx = GetLastError();
			notifi(" Create child window Notifi ERROR ");
			notifiInt(errorx ,5 );
		}		
		hwndList = CreateWindow(TEXT("listbox") , NULL, WS_CHILD |
			WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL,
			cX, cY, cW, cH, hwnd,(HMENU) IDC_LIST, g_hinst, NULL);
		if(hwndList == NULL){
			DWORD errorx = GetLastError();
			notifi(" Create child window listbox ERROR ");
			notifiInt(errorx ,5 );
		}
		ShowWindow(hwndList, SW_SHOW);
		UpdateWindow(hwndList);
		for (int i = 0; i < (int)listExeName.size(); i++){
			myDataType p = (myDataType)listExeName[i];
			SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)p.lpExeName);
		}
		CreateWindow(TEXT("button"), TEXT("Open .dll"),
			WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			bOkX, bOkY, 100, 40,
			hwnd, (HMENU) IDC_BT_OPENFILE, NULL, NULL);
		CreateWindow(TEXT("button"), TEXT("Start Injection"),
			WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			bOkX + 120, bOkY, 100, 40,
			hwnd, (HMENU) IDC_BT_INJECT, NULL, NULL);
		CreateWindow(TEXT("button"), TEXT("Exit"),
			WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			bOkX + 240, bOkY, 60, 40,
			hwnd, (HMENU) IDC_BT_CLOSE, NULL, NULL);
		ShowWindow(hwndList, SW_SHOW);
		UpdateWindow(hwnd);
		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
		break;
	case WM_VSCROLL:
		eventScroll(hwndList, si, wParam);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_LIST){
			if (HIWORD(wParam) == LBN_SELCHANGE){
				sel = (int) SendMessage(hwndList, LB_GETCURSEL, 0, 0);
				StringCbPrintf(buff, ARRAYSIZE(buff), TEXT("%d"),listExeName.at(sel).processID);
				SetWindowText(hwndStaticID, buff);
			}
		}		
		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
		if (LOWORD(wParam) == IDC_BT_OPENFILE){			
			if(sel == -1){
				StringCbPrintf(buff, ARRAYSIZE(buff), TEXT("%s"),TEXT("Name Process NULL,Place you choose again!"));
				SetWindowText(hwndStaticNotifi, buff);		
				break;
			}else{
				wcscpy(file.lpExeName,openFile(hwnd));	
				StringCbPrintf(buff, ARRAYSIZE(buff), TEXT("%s"),file.lpExeName);
				SetWindowText(hwndStaticPath, buff);
			}
		}
		if (LOWORD(wParam) == IDC_BT_INJECT){
			if(sel == -1){
				StringCbPrintf(buff, ARRAYSIZE(buff), TEXT("%s"),TEXT("Select Name Process and file DLL before Injection!"));
				SetWindowText(hwndStaticNotifi, buff);	
				break;
			}else{
				if(flag_openFile == FALSE){
					StringCbPrintf(buff, ARRAYSIZE(buff), TEXT("%s"),TEXT("Choose file .dll before Injection!"));
					SetWindowText(hwndStaticNotifi, buff);	
				}else
					injectCode((myDataType)listExeName.at(sel));
			}
		}
		if (LOWORD(wParam) == IDC_BT_CLOSE){			
			SendMessage(hwnd, WM_CLOSE, 0, 0);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return (DefWindowProc(hwnd, msg, wParam, lParam));
	
}


BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege){
	TOKEN_PRIVILEGES tp;
	LUID luid;
	if(LookupPrivilegeValue(NULL,SE_DEBUG_NAME/*SE_INCREASE_QUOTA_NAME*/,&luid) == FALSE){ // SE_INCREASE_QUOTA_NAME == "SeSecurityPrivilege"
		char szMsg[256] = {0} ;
		sprintf_s( szMsg , 256 , "LookupPrivilegeValue() error:") ;
		::OutputDebugStringA(szMsg) ;
		return FALSE;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if(bEnablePrivilege){
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		//		printf("tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED\n");
	}
	else{
		tp.Privileges[0].Attributes = 0;
		//		printf("tp.Privileges[0].Attributes = 0\n");
	}

	// Enable the privilege (or disable all privileges).
	if(AdjustTokenPrivileges(hToken,FALSE,&tp, sizeof(TOKEN_PRIVILEGES),(PTOKEN_PRIVILEGES) NULL,(PDWORD) NULL)== FALSE){
		char szMsg[256] = {0} ;
		sprintf_s( szMsg , 256 , "AdjustTokenPrivileges() error:") ;
		//		::OutputDebugStringA(szMsg) ;
		return FALSE;
	}
	return TRUE;
}
void injectCode(myDataType process){
	HANDLE hThread = NULL;
	HMODULE hmod =NULL;	
	LPVOID  lpFileName;
	unsigned long pa;	
	HANDLE hProcessInject = OpenProcess(PROCESS_ALL_ACCESS,FALSE,process.processID);
	if(hProcessInject ==NULL){
		StringCbPrintf(buff, ARRAYSIZE(buff), TEXT("%s"),TEXT("Openprocess errer!"));
		SetWindowText(hwndStaticNotifi, buff);				
	}
	hmod = GetModuleHandle(TEXT("kernel32"));
	pa = (unsigned long)GetProcAddress(hmod,"LoadLibraryW");
	lpFileName = (LPVOID)VirtualAllocEx(hProcessInject, NULL, sizeof(file.lpExeName) , MEM_COMMIT, PAGE_READWRITE );
	DWORD dwLastErr = 0 ;
	if (lpFileName == NULL)
	{
		dwLastErr = ::GetLastError() ;
		StringCbPrintf(buff, ARRAYSIZE(buff), TEXT("%s:%d"),TEXT(" VirtualAllocEx ERRER"), ::GetLastError());
		SetWindowText(hwndStaticNotifi, buff);				
	}
	BOOL bResult = WriteProcessMemory( hProcessInject,lpFileName, file.lpExeName, sizeof(file.lpExeName) , NULL );
	if(bResult== FALSE){
		StringCbPrintf(buff, ARRAYSIZE(buff), TEXT("%s:%d"),TEXT(" WriteProcessMemory ERRER"), ::GetLastError());
		SetWindowText(hwndStaticNotifi, buff);				
	}

	//lp
	hThread = CreateRemoteThread(hProcessInject, NULL, 0, (LPTHREAD_START_ROUTINE)pa, (LPVOID)lpFileName, 0, NULL );
	if(hThread== NULL){
		StringCbPrintf(buff, ARRAYSIZE(buff), TEXT("%s:%d"),TEXT("CreateRemoteThread ERRER"), ::GetLastError());
		SetWindowText(hwndStaticNotifi, buff);				
	}else{
		StringCbPrintf(buff, ARRAYSIZE(buff), TEXT("%s"),TEXT("Inject succeeds!"));
		SetWindowText(hwndStaticNotifi, buff);			
	}
}

void notifi(LPCSTR msg){
	::OutputDebugStringA(msg) ;		
}
void notifiInt(int msg, int strlen){
	char* buff;	
	strlen = strlen +4;
	buff = new char[strlen];
	sprintf_s( buff , strlen , "%d !",msg);
	::OutputDebugStringA(buff) ;		
}

void notifiF(float msg, int strlen){
	char* buff;	
	strlen = strlen +8;
	buff = new char[strlen];
	sprintf_s( buff , strlen , "%.4f !\n",msg);
	::OutputDebugStringA(buff) ;		
}

LPTSTR openFile(HWND hwnd){
		OPENFILENAME ofn;       // common dialog box structure
		char szFile[260];       // buffer for file name
		HANDLE hf;              // file handle
		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = hwnd;
		ofn.lpstrFile = (LPWSTR)szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = TEXT("DLL\0*.dll\0All\0*.*\0");
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		// Display the Open dialog box. 
		if (GetOpenFileName(&ofn)==TRUE){
			hf = CreateFile(ofn.lpstrFile, 
				GENERIC_READ,
				0,
				(LPSECURITY_ATTRIBUTES) NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				(HANDLE) NULL);
			flag_openFile = TRUE;
			CloseHandle(hf);
		}else
			flag_openFile = FALSE;
		return ofn.lpstrFile;
}
void createSrcoll(HWND hwnd, SCROLLINFO si, int nMin, int nMax){
//		hwnd, si, nMax, nPage
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = nMin;
		si.nMax = nMax;// 100 -1
		si.nPage = (nMax/2)+1;
		SetScrollInfo (hwnd, SB_HORZ | SB_VERT | SB_CTL, &si, TRUE);
}
void eventScroll( HWND hwnd, SCROLLINFO si, WPARAM wParam){
		// Get all the vertical scroll bar information
		//hwnd, si, wParam
		si.cbSize = sizeof (si);
		si.fMask  = SIF_ALL;
		GetScrollInfo (hwnd, SB_VERT, &si);
		// Save the position for comparison later on
		yPos = si.nPos;
		switch (LOWORD (wParam)){
			// user clicked the HOME keyboard key
		case SB_TOP:
			si.nPos = si.nMin;
			break;

			// user clicked the END keyboard key
		case SB_BOTTOM:
			si.nPos = si.nMax;
			break;

			// user clicked the top arrow
		case SB_LINEUP:
			si.nPos -= 1;
			break;

			// user clicked the bottom arrow
		case SB_LINEDOWN:
			si.nPos += 1;
			break;

			// user clicked the scroll bar shaft above the scroll box
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;

			// user clicked the scroll bar shaft below the scroll box
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;

			// user dragged the scroll box
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;

			// user positioned the scroll box
			// This message is the one used by Windows Touch
		case SB_THUMBPOSITION:
			si.nPos = HIWORD(wParam);
			break;

		default:
			ShowWindow(hwnd, SW_SHOW);
			UpdateWindow(hwnd);
			break; 
		}
		// Set the position and then retrieve it.  Due to adjustments
		//   by Windows it may not be the same as the value set.
		si.fMask = SIF_POS;
		SetScrollInfo (hwnd, SB_VERT, &si, TRUE);
		GetScrollInfo (hwnd, SB_VERT, &si);
		// If the position has changed, scroll window and update it
		if (si.nPos != yPos){                    
			ScrollWindow(hwnd, 0, yChar * (yPos - si.nPos), NULL, NULL);
			ShowWindow(hwnd, SW_SHOW);
			UpdateWindow(hwnd);
		}
}
