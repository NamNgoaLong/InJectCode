#include "LibH.h"
#include <windows.h>
#include "iostream"
#include <tchar.h>
#include <vector>  
#include "strsafe.h"
#include <psapi.h> 	
#include "memory"
#include "strsafe.h"
#include <stdlib.h>
#include <Shlwapi.h>
#include <string.h>
#include <vector>
#pragma comment( lib, "psapi.lib" )

#define SIZE 1024
#define IDC_LIST 1
#define IDC_LB_OFFSET	1
#define IDC_LB_NUMBYTE	2
#define IDC_LB_HEX		3
#define IDC_LB_RESULT	4
#define IDC_ET_OFFSET	5
#define IDC_ET_NUMBYTE	6
#define IDC_ET_HEX		7
#define IDC_ET_RESULT	8
#define IDC_LB_NOTIFI	10
#define IDC_BT_SEARCH	1001
#define IDC_BT_EXIT	1002
using namespace std;
typedef struct _myDataType
{
	byte dataBaseDll[1000000];
} myDataType ;
static DWORD addressFinded = 0;
static BOOL fgEqual = FALSE;	
HINSTANCE g_hinst;
static int cX =40, cY =60, cW =600, cH =450;
static int bX = 200, bY =190, bW =60, bH =30;
static int lbW =100, lbH =30, space = 15;
void eventScroll( HWND hwnd, SCROLLINFO si, WPARAM wParam);
void createSrcoll(HWND hwnd, SCROLLINFO si, int nMin, int nMax);
BOOL search( vector<int> vtHex, DWORD size, DWORD address);
void OutDebugStringInLine32(const unsigned char* lpSrc, const int len ,const char* lpPre /*= NULL*/);
int httoi(const TCHAR *value);
LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
BOOL WINAPI DllMain(HINSTANCE hInstDLL,DWORD fdwReason,PVOID lpvReserved)
{	
	MSG msg ;
	WNDCLASS wc = {0};
	wc.lpszClassName = TEXT( "Application" );
	wc.hInstance = hInstDLL;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpfnWndProc = WndProc;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	g_hinst = hInstDLL;
	HWND hwndPer;
	switch(fdwReason){
	  case DLL_PROCESS_ATTACH:
		  RegisterClass(&wc);
		  hwndPer = CreateWindow( wc.lpszClassName, TEXT("Windows DLL"),
			  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			  cX, cY, cW, cH, 0, 0, hInstDLL, 0);
		  if(hwndPer == NULL){
			  ::OutputDebugStringA(" Loi hwndPer ") ;
		  }
		  ShowWindow(hwndPer, SW_SHOWNORMAL);
		  UpdateWindow(hwndPer);
		  while( GetMessage(&msg, NULL, 0, 0)) {
			  TranslateMessage(&msg);
			  DispatchMessage(&msg);
		  }
//		  show();
		  break;
	  case DLL_THREAD_ATTACH:
		  break;
	  case DLL_THREAD_DETACH:
		  break;
	  case DLL_PROCESS_DETACH:
		  break;
	}

	return TRUE;
}

	

	//static int yPos;
	//static int xPos;
	//static int yChar = 5;
	//static SCROLLINFO si;
	//static typePath file;
	static HWND hwndLbOffset;
	static HWND hwndLbByte;
	static HWND hwndLbHex;
	static HWND hwndLbResult;
	static HWND hwndEtOffset;
	static HWND hwndEtNumByte;
	static HWND hwndEtHex;
	static HWND hwndEtResult;
	static HWND hwndStaticNotifi;
	static HWND hwndBtSearch;
	static HWND hwndBtExit;
	static SCROLLINFO si1;
	static SCROLLINFO si2;
	static int yPos;
	static int xPos;
	static int yChar = 5;
//	TCHAR buff[300];
LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ){	
	static int nMin = 0;
	static int nMax = 200;
	switch(msg){
	case WM_SIZE:
		createSrcoll(hwndEtHex, si1, nMin, nMax);
		break;
	case WM_CREATE:
		hwndLbOffset = CreateWindow(TEXT("static") , TEXT("Offset Address:"), 
			WS_CHILD | WS_VISIBLE | LBS_NOTIFY,
			cX +10, cY +space, lbW, lbH, hwnd,(HMENU)IDC_LB_OFFSET, g_hinst, NULL);
		if( hwndLbOffset == NULL){
//			DWORD errorx = GetLastError();
			::OutputDebugStringA(" Error hwndLbOffset ") ;
		}
		hwndEtOffset = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, WS_CHILD|WS_VISIBLE,
			cX +10 +lbW +space, cY +space, lbW, lbH, hwnd, (HMENU)IDC_ET_OFFSET, g_hinst, NULL);
		if( hwndEtOffset == NULL){
//			DWORD errorx = GetLastError();
			::OutputDebugStringA(" Error hwndEtOffset ") ;
		}
		hwndLbByte = CreateWindow(TEXT("static") , TEXT("Size of Bytes:"), 
			WS_CHILD | WS_VISIBLE | LBS_NOTIFY,
			cX +2*lbW +10 + space, cY +space, lbW, lbH, hwnd,(HMENU) IDC_LB_NUMBYTE, g_hinst, NULL);
		if( hwndLbByte == NULL){
//			DWORD errorx = GetLastError();
			::OutputDebugStringA(" Error hwndLbByte ") ;
		}
		hwndEtNumByte = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, WS_CHILD|WS_VISIBLE,
			cX +3*lbW +10 +space, cY +space, lbW, lbH, hwnd, (HMENU)IDC_ET_NUMBYTE, g_hinst, NULL);
		if( hwndEtNumByte == NULL){
//			DWORD errorx = GetLastError();
			::OutputDebugStringA(" Error hwndEtNumByte ") ;
		}

		hwndLbHex = CreateWindow(TEXT("static") , TEXT("Code Hex:"), 
			WS_CHILD | WS_VISIBLE | LBS_NOTIFY,
			cX +10, cY +lbH + 2*space, lbW, lbH, hwnd,(HMENU) IDC_LB_HEX, g_hinst, NULL);
		if( hwndLbHex == NULL){
//			DWORD errorx = GetLastError();
			::OutputDebugStringA(" Error hwndLbHex ") ;
		}
		hwndEtHex=CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL,
			cX +lbW +10 +space, cY +lbH + 2*space, 200, 50, hwnd, (HMENU)IDC_ET_HEX, g_hinst, NULL);
		if( hwndEtHex == NULL){
//			DWORD errorx = GetLastError();
			::OutputDebugStringA(" Error hwndLbOffset ") ;
		}
		hwndLbResult = CreateWindow(TEXT("static") , TEXT("Result:"), 
			WS_CHILD | WS_VISIBLE | LBS_NOTIFY,
			cX +10, cH - 195, lbW, lbH, hwnd,(HMENU) IDC_LB_RESULT, g_hinst, NULL);
		if( hwndLbResult == NULL){
//			DWORD errorx = GetLastError();
			::OutputDebugStringA(" Error hwndLbResult ") ;
		}
		hwndEtResult = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL,
			cX +lbW +10 +space, cH - 195, 3*lbW + 3*space, 150, hwnd, (HMENU)IDC_ET_RESULT, g_hinst, NULL);
		if( hwndEtResult == NULL){
//			DWORD errorx = GetLastError();
			::OutputDebugStringA(" Error hwndEtResult ") ;
		}

		hwndStaticNotifi = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL,
			cX +10, cH -160, lbW, lbH, hwnd, (HMENU)IDC_LB_NOTIFI, g_hinst, NULL);
		if( hwndStaticNotifi == NULL){
//			DWORD errorx = GetLastError();
			::OutputDebugStringA(" Error hwndStaticNotifi ") ;
		}


		hwndBtSearch = CreateWindow(TEXT("button"), TEXT("Search"),
			WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			bX, bY, bW, bH,
			hwnd, (HMENU) IDC_BT_SEARCH, NULL, NULL);
		if( hwndBtSearch == NULL){
//			DWORD errorx = GetLastError();
			::OutputDebugStringA(" hwndStaticNotifi Error") ;
		}		
		hwndBtExit = CreateWindow(TEXT("button"), TEXT("EXIT"),
			WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			bX + bW +3*space, bY, bW, bH,
			hwnd, (HMENU) IDC_BT_EXIT, NULL, NULL);
		if( hwndBtExit == NULL){
//			DWORD errorx = GetLastError();
			::OutputDebugStringA(" hwndBtExit Error") ;
		}		
		ShowWindow(hwnd, SW_SHOWNORMAL);
		UpdateWindow(hwnd);
		break;
	case WM_VSCROLL:
//		eventScroll(hwndEtHex, si, wParam);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BT_SEARCH){			
			DWORD size;
			DWORD offSet;
			LPWSTR pOffset = new WCHAR[SIZE/2];
			GetWindowText(hwndEtNumByte, pOffset, SIZE/2);
			offSet = _wtoi(pOffset);
			LPWSTR pSize = new WCHAR[SIZE/2];
			GetWindowText(hwndEtNumByte, pSize, SIZE/2);
			size = _wtoi(pSize);
			LPWSTR pHex = new WCHAR[SIZE];
			GetWindowText(hwndEtHex, pHex, SIZE); 		
			static LPWSTR pch;
			static LPWSTR pSig = TEXT(" ,.-");
			static vector<int> listToSearch;
			pch = wcstok(pHex,pSig);
			while (pch != NULL){
				listToSearch.push_back(httoi((TCHAR *)pch));
				pch = wcstok(NULL, pSig);
			}
			if(search(listToSearch, 0, 0) == TRUE){				
				TCHAR buf[50];
				_stprintf(buf, _T("%x"), addressFinded);
				SendMessage(hwndStaticNotifi, WM_SETTEXT, 0, (LPARAM)buf);
				MessageBox(NULL, TEXT("Search OK"), TEXT("Notifications"), MB_ICONINFORMATION);
			}else{
				SendMessage(hwndStaticNotifi, WM_SETTEXT, 0, (LPARAM)TEXT(""));
				MessageBox(NULL, TEXT("Search failed"), TEXT("Notifications"), MB_ICONINFORMATION);
			}		
			listToSearch.clear();
			ShowWindow(hwnd, SW_SHOW);
			UpdateWindow(hwnd);
			if (LOWORD(wParam) == IDC_BT_EXIT){			
				SendMessage(hwnd, WM_CLOSE, 0, 0);
			}
		}
		if (LOWORD(wParam) == IDC_BT_EXIT){			
			SendMessage(hwnd, WM_CLOSE, 0, 0);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return (DefWindowProc(hwnd, msg, wParam, lParam));	
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

//		BOOL bResultDLL = ReadProcessMemory(hProcess, minfo.lpBaseOfDll, entryDLL.taBaseDll, NumberOfBytesToRead, &NumberOfBytesActuallyRead);
//		std::uninitialized_copy(minfo.lpBaseOfDll,minfo.lpBaseOfDll + NumberOfBytesActuallyRead, entryDLL.dataBaseDll);

BOOL search( vector<int> vtHex, DWORD size, DWORD address){
	MODULEINFO  minfo ={0};
	HANDLE hProcess = GetCurrentProcess();	
	BOOL bResultMoDu = GetModuleInformation(hProcess, GetModuleHandle(NULL), &minfo, sizeof(minfo));
	if(bResultMoDu == FALSE){
		::OutputDebugStringA(" GetModuleInformation Error ");
	}else{			
		BYTE* dataDll = (BYTE*)minfo.lpBaseOfDll;
//		dataDll = dataDll + 0x1000;	
		if(size ==0)
			size = minfo.SizeOfImage;
/*
		if(address !=0){
			i =i + address;
		}else{
			i =0;
		}
*/
		char szMsg[256] = {0} ;
		sprintf_s( szMsg , 256, "Image base : 0x%08x , size cua tui : 0x%08x", (DWORD)minfo.lpBaseOfDll , size );
		::OutputDebugStringA(szMsg) ;
		//::OutputDebugStringA("Data Dll: ") ;

		::OutputDebugStringA("vtHex day: ") ;	
		for(int k=0; k< (int)vtHex.size(); k++){
			char szMsg[256] = {0} ;
			sprintf_s( szMsg , 256, " 0x%02x", vtHex[k]);
			::OutputDebugStringA(szMsg) ;			
		}

		::OutputDebugStringA("Dll Hex day: ") ;	
		OutDebugStringInLine32(dataDll, 4, NULL);
		static BOOL fgStop = FALSE;
		for(DWORD i =0; i< size; i++){
			if(!fgStop){			
				if((int)dataDll[i] == (int)vtHex.at(0)){
					if((size - i) >= (int)vtHex.size()){
						DWORD k =i +1;				
						int j=1;
						for(j; j< (int)vtHex.size(); j++){
							if((int)dataDll[k] != (int)vtHex.at(j)){							
								break;
							}						
							k++;
						}
						if(j >= (int)vtHex.size()){
							fgEqual = TRUE;
							addressFinded = (DWORD)dataDll +i;
							fgStop = TRUE;
						}else{
							fgEqual = FALSE;							
							fgStop = FALSE;
						}
					}
				}
			}else{
				fgStop = FALSE;
				break;
			}
		}
	}
	return fgEqual;
}
	
void OutDebugStringInLine32(const unsigned char* lpSrc, const int len ,const char* lpPre /*= NULL*/)
{
	if ( !lpSrc || len <= 0 )
		return ;
	char outmsgbuff[256] = {0} ;
	char* outLine = outmsgbuff ;
	memset( outmsgbuff,' ',sizeof(outmsgbuff)/sizeof(outmsgbuff[0])) ;
	int iPre = 0 ;
	if ( lpPre )
	{
		iPre = strlen(lpPre) ;
		if ( iPre < 100 )
		{
			outLine = &outmsgbuff[iPre] ;
			memcpy(outmsgbuff,lpPre,iPre) ;
		}
		else
		{
			OutputDebugStringA("OutDebugStringInLine32 pre string is to long.") ;
		}
	}

	int iSrc = 0 , iInLine = 0 ;
	char asii[33] = {0} ;
	memset(asii,' ',32) ;

	for ( ; iSrc < len ; iSrc++ )
	{
		asii[iInLine] =  lpSrc[iSrc] > 0x1f ? lpSrc[iSrc] : '.' ;
		unsigned char hi = ( lpSrc[iSrc] & 0xF0 ) >> 4 ;
		hi += hi > 0x9 ? 0x37 : 0x30 ;
		unsigned char lo = ( lpSrc[iSrc] & 0x0F ) ;
		lo += lo > 0x9 ? 0x37 : 0x30 ;
		int bEx = 0 ;
		if ( iInLine >= 16 )
			bEx = 3 ;
		outLine[iInLine*3+bEx] = hi ;
		outLine[iInLine*3+1+bEx] = lo ;
		outLine[iInLine*3+2+bEx] = ' ' ;
		if ( iInLine >= 31 )
		{
			memcpy( &outLine[105] , asii, 33 ) ;
			OutputDebugStringA(outmsgbuff) ;

			memset( outLine,' ',256-iPre) ;
			memset(asii,' ',32) ;
			iInLine = 0 ;
		}
		else
			iInLine++ ;
	}
	if ( iInLine != 0)
	{
		memcpy( &outLine[105] , asii, 33 ) ;
		OutputDebugStringA(outmsgbuff) ;
	}
}

int httoi(const TCHAR *value)
{
  struct CHexMap
  {
    TCHAR chr;
    int value;
  };
  const int HexMapL = 16;
  CHexMap HexMap[HexMapL] =
  {
    {'0', 0}, {'1', 1},
    {'2', 2}, {'3', 3},
    {'4', 4}, {'5', 5},
    {'6', 6}, {'7', 7},
    {'8', 8}, {'9', 9},
    {'A', 10}, {'B', 11},
    {'C', 12}, {'D', 13},
    {'E', 14}, {'F', 15}
  };
  TCHAR *mstr = _tcsupr(_tcsdup(value));
  TCHAR *s = mstr;
  int result = 0;
  if (*s == '0' && *(s + 1) == 'X') s += 2;
  bool firsttime = true;
  while (*s != '\0')
  {
    bool found = false;
    for (int i = 0; i < HexMapL; i++)
    {
      if (*s == HexMap[i].chr)
      {
        if (!firsttime) result <<= 4;
        result |= HexMap[i].value;
        found = true;
        break;
      }
    }
    if (!found) break;
    s++;
    firsttime = false;
  }
  free(mstr);
  return result;
}


/*

int memcmp (const void *s1, const void *s2, size_t n)

/*
//			GetDlgItemText(hwnd,IDC_ET_HEX,pStr,sizeof(pStr));
//			SendMessage(hwndEtNumByte, WM_GETTEXT, (WPARAM)sizeof(pSize), (LPARAM)pSize);

		void* addressFinded = memchr(dataDll, pHex[0], size);
		if(addr!=NULL){
			::OutputDebugStringA("Khong bang nhau") ;			
//		CopyMemory(dataDll, minfo.lpBaseOfDll, size);	
			if(memcmp(dataDll, pHex, 2) == 0){
				::OutputDebugStringA("Bang nhau") ;			
			}else{
			char szMsg[256] = {0} ;
			sprintf_s( szMsg , 256, " %d", dataDll[i]);
			::OutputDebugStringA(szMsg) ;			
				
			}
*/


