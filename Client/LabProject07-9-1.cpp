// LabProject07-9-1.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "LabProject07-9-1.h"
#include "GameFramework.h"

//Server
#include "Network.h"

#define MAX_LOADSTRING 100

HINSTANCE						ghAppInstance;
TCHAR							szTitle[MAX_LOADSTRING];
TCHAR							szWindowClass[MAX_LOADSTRING];

CGameFramework					gGameFramework;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	//==================================================
	//					Server Code
	//==================================================
	string tempIP;
	cout << "접속할 서버의 IP주소를 입력해주세요: ";
	cin >> tempIP;

	SERVERIP = const_cast<char*>(tempIP.c_str());

	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return -1;
	}

	// 클라-로그인서버 통신담당 스레드 생성
	HANDLE h_networkLS_th = CreateThread(NULL, 0, Network_WithLS_ThreadFunc, NULL, 0, NULL);
	SetEvent(h_thread_event_LS);

	// 로그인 서버로부터 게임 로그인이 허가될 때까지 대기.
	while (!g_gamestart) {
		Sleep(100);
	}
	cout << "Game Start" << endl;

	// 클라-게임서버 통신담당 스레드 생성
	HANDLE h_networkGS_th = CreateThread(NULL, 0, Network_WithGS_ThreadFunc, NULL, 0, NULL);
	SetEvent(h_thread_event_GS);

	// 게임 서버로부터 객체 초기정보를 받을 때까지 대기.
	while (players_info[myID].m_state != OBJ_ST_RUNNING) {
		Sleep(100);
	}
	gGameFramework.Login_ID = myID;

	//==================================================



	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_LABPROJECT0791, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) return(FALSE);

	hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABPROJECT0791));

	bool already_print = false;

	WCHAR UserName[20];
	_tcscpy_s(UserName, _T("User's name: "));
	wcscpy(gGameFramework.m_InputName, UserName);

	wcscat(gGameFramework.m_InputName, myname);

	while (1)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		else
		{
			//==================================================
			//			서버에게 패킷을 전송합니다.
			//==================================================
			if (!gGameFramework.is_KeyInput_Empty()) {
				short send_keyValue = gGameFramework.pop_keyvalue();									// 키입력 큐에 있는 키값 중 가장 먼저 입력된 키값을
				C2GS_KEYVALUE_PACKET keyvalue_pack;
				keyvalue_pack.type = C2GS_KEYVALUE;
				keyvalue_pack.key = send_keyValue;
				retval = send(sock_forGS, (char*)&keyvalue_pack, sizeof(C2GS_KEYVALUE_PACKET), 0);		// 서버로 전송합니다.
				if (retval == SOCKET_ERROR) {
					err_display("send()");
				}
				//cout << "Key: " << keyvalue_pack.key << endl; //test
			}
			if (!gGameFramework.is_KeyUp_Empty()) {
				short send_keyUPValue = gGameFramework.pop_keyUpvalue();									// 키입력 큐에 있는 키값 중 가장 먼저 입력된 키값을

				// protocol
				C2GS_KEYUPVALUE_PACKET keyUPvalue_pack;
				keyUPvalue_pack.type = C2GS_KEYUPVALUE;
				keyUPvalue_pack.key = send_keyUPValue;
				retval = send(sock_forGS, (char*)&keyUPvalue_pack, sizeof(C2GS_KEYUPVALUE_PACKET), 0);		// 서버로 전송합니다.
				if (retval == SOCKET_ERROR) {
					err_display("send()");
				}
				//cout << "Key: " << keyvalue_pack.key << endl; //test
			}
			//==================================================

			//==================================================
			//	    서버로부터 받은 값으로 최신화해줍니다.
			//==================================================
			// 클라이언트 정보 최신화
			for (int i = 0; i < MAX_USER; i++) {
				if (players_info[i].m_state == OBJ_ST_RUNNING) {
					gGameFramework.myFunc_SetPosition(i, players_info[i].GetPosition());
					gGameFramework.myFunc_SetVectors(i, players_info[i].GetRightVector(), players_info[i].GetUpVector(), players_info[i].GetLookVector());
					gGameFramework.myFunc_SetBoundingBox(i, players_info[i].GetPosition());
					gGameFramework.m_pPlayer->m_Boobb = objinfo.m_xoobb.PlayerOOBB = BoundingOrientedBox(players_info[i].GetPosition(), XMFLOAT3(20.0, 20.0, 20.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));
					gGameFramework.m_pScene->m_ppGameObjects[i]->m_Boobb = objinfo.m_xoobb.PlayerOOBB = BoundingOrientedBox(players_info[i].GetPosition(), XMFLOAT3(20.0, 20.0, 20.0), XMFLOAT4(0.0, 0.0, 0.0, 1.0));
				}
				else if (players_info[i].m_state == OBJ_ST_LOGOUT) {
					gGameFramework.myFunc_SetPosition(i, players_info[i].GetPosition());
					gGameFramework.myFunc_SetVectors(i, players_info[i].GetRightVector(), players_info[i].GetUpVector(), players_info[i].GetLookVector());
					players_info[i].m_state = OBJ_ST_EMPTY;
				}
			}

			// 아이템 박스 정보 최신화
			for (int i = 0; i < ITEMBOXNUM; i++) {
				gGameFramework.m_pScene->m_ppGameObjects[i + 2]->SetPosition(itembox_arr[i].GetPosition());
				gGameFramework.m_pScene->m_ppGameObjects[i + 2]->myFunc_SetVectors(itembox_arr[i].GetRightVector(), itembox_arr[i].GetUpVector(), itembox_arr[i].GetLookVector());
				gGameFramework.m_pScene->m_ppGameObjects[i + 2]->SetScale(15.0f, 15.0f, 15.0f);
			}

			// 미사일 정보 최신화
			for (int i = 0; i < MissileNum; i++) {
				if (missile_arr[i].m_state == OBJ_ST_RUNNING) {
					gGameFramework.m_pScene->m_ppGameObjects[i + 189]->SetPosition(missile_arr[i].GetPosition());
					gGameFramework.m_pScene->m_ppGameObjects[i + 189]->myFunc_SetVectors(missile_arr[i].GetRightVector(), missile_arr[i].GetUpVector(), missile_arr[i].GetLookVector());
					gGameFramework.m_pScene->m_ppGameObjects[i + 189]->SetScale(50.0, 50.0, 80.0);
					gGameFramework.m_pScene->m_ppGameObjects[i + 189]->Rotate(90.0, 0.0, 0.0);
				}
				else if (missile_arr[i].m_state == OBJ_ST_LOGOUT) {
					gGameFramework.m_pScene->m_ppGameObjects[i + 189]->SetPosition(missile_arr[i].GetPosition());
					gGameFramework.m_pScene->m_ppGameObjects[i + 189]->myFunc_SetVectors(missile_arr[i].GetRightVector(), missile_arr[i].GetUpVector(), missile_arr[i].GetLookVector());
					gGameFramework.m_pScene->m_ppGameObjects[i + 189]->SetScale(0.5, 0.5, 0.8);
					gGameFramework.m_pScene->m_ppGameObjects[i + 189]->Rotate(90.0, 0.0, 0.0);
					missile_arr[i].m_state = OBJ_ST_EMPTY;
				}
			}

			// 지뢰 정보 최신화
			for (int i = 0; i < BombNum; i++) {
				if (bomb_arr[i].m_state == OBJ_ST_RUNNING) {
					gGameFramework.m_pScene->m_ppGameObjects[i + 289]->SetPosition(bomb_arr[i].GetPosition());
					gGameFramework.m_pScene->m_ppGameObjects[i + 289]->myFunc_SetVectors(bomb_arr[i].GetRightVector(), bomb_arr[i].GetUpVector(), bomb_arr[i].GetLookVector());
					gGameFramework.m_pScene->m_ppGameObjects[i + 289]->SetScale(1.0f, 1.0f, 1.0f);
					gGameFramework.m_pScene->m_ppGameObjects[i + 289]->Rotate(0.0, 0.0, 0.0);
				}
				else if (bomb_arr[i].m_state == OBJ_ST_LOGOUT) {
					gGameFramework.m_pScene->m_ppGameObjects[i + 289]->SetPosition(bomb_arr[i].GetPosition());
					gGameFramework.m_pScene->m_ppGameObjects[i + 289]->myFunc_SetVectors(bomb_arr[i].GetRightVector(), bomb_arr[i].GetUpVector(), bomb_arr[i].GetLookVector());
					gGameFramework.m_pScene->m_ppGameObjects[i + 289]->SetScale(0.01, 0.01, 0.01);
					gGameFramework.m_pScene->m_ppGameObjects[i + 289]->Rotate(90.0, 0.0, 0.0);
					bomb_arr[i].m_state = OBJ_ST_EMPTY;
				}
			}

			// 부스터 효과 정보 최신화
			for (int i = 0; i < MAX_USER; i++) {
				if (players_info[i].m_state != OBJ_ST_RUNNING)
					continue;

				gGameFramework.SetBoosterEffect(i, players_info[i].m_boost_on);
			}
			//=====================================================
			
			//=====================================================
			//			현재 LAP 숫자를 받아 넘겨줍니다.
			//=====================================================
			wchar_t mylapbuf[20];
			_itow_s(myLapNum, mylapbuf, sizeof(mylapbuf) / 2, 10);
			wcscpy(gGameFramework.m_mylapnum, mylapbuf);

			if (myLapNum == 3) {
				WCHAR downLap[2];
				_tcscpy_s(downLap, _T("2"));

				wcscpy(gGameFramework.m_mylapnum, downLap);
			}


			if (myLapNum >= 3 && !already_print)
			{
				already_print = true;
				wchar_t endtimebuf[50];
				_itow_s(endTime, endtimebuf, sizeof(endtimebuf) / 2, 10);

				WCHAR UserTime[20];
				_tcscpy_s(UserTime, _T("Finish Time: "));
				wcscpy(gGameFramework.m_endTime, UserTime);

				wcscat(gGameFramework.m_endTime, endtimebuf);

				WCHAR msgTime[6];
				_tcscpy_s(msgTime, _T("sec"));
				wcscat(gGameFramework.m_endTime, msgTime);

				wcscpy(gGameFramework.m_myMSG, myMsg);
			}

			gGameFramework.FrameAdvance();

		}
	}
	gGameFramework.OnDestroy();

	closesocket(sock_forGS);
	CloseHandle(h_networkLS_th);
	return((int)msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LABPROJECT0791));
	wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCE(IDC_LABPROJECT0791);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return ::RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	ghAppInstance = hInstance;

	RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
	AdjustWindowRect(&rc, dwStyle, FALSE);
	HWND hMainWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!hMainWnd) return(FALSE);

	gGameFramework.OnCreate(hInstance, hMainWnd);

	::ShowWindow(hMainWnd, nCmdShow);
	::UpdateWindow(hMainWnd);

	return(TRUE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	case WM_KEYUP:
		gGameFramework.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			::DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			::DestroyWindow(hWnd);
			break;
		default:
			return(::DefWindowProc(hWnd, message, wParam, lParam));
		}
		break;
	case WM_PAINT:
		hdc = ::BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	default:
		return(::DefWindowProc(hWnd, message, wParam, lParam));
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return((INT_PTR)TRUE);
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			::EndDialog(hDlg, LOWORD(wParam));
			return((INT_PTR)TRUE);
		}
		break;
	}
	return((INT_PTR)FALSE);
}
