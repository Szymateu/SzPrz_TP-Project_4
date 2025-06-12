// Projekt4_RamieRobota.cpp : Definiuje punkt wejścia dla aplikacji.
//
#include "framework.h"
#include "Projekt4_RamieRobota.h"
#include <cmath>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib, "Gdiplus.lib")

#define MAX_LOADSTRING 100

// Zmienne globalne:
HINSTANCE hInst;                                // bieżące wystąpienie
WCHAR szTitle[MAX_LOADSTRING];                  // Tekst paska tytułu
WCHAR szWindowClass[MAX_LOADSTRING];            // nazwa klasy okna głównego

// Przekaż dalej deklaracje funkcji dołączone w tym module kodu:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


VOID OnPaint(HDC hdc) //generated paint func
{
    Graphics graphics(hdc);
    Pen pen(Color(255, 0, 0, 255), 5); // Czerwony, gruby długopis
    SolidBrush jointBrush(Color(255, 0, 0, 255)); // Czerwony pędzel do przegubów

    // Pozycje bazowe
    int baseX = 300;
    int baseY = 400;

    // Długości segmentów
    int length1 = 100; // długość dolnego ramienia
    int length2 = 80;  // długość górnego ramienia

    // Kąty przegubów (na sztywno, w stopniach)
    float angle1 = 45.0f; // ramię 1 względem pionu
    float angle2 = -30.0f;  // ramię 2 względem ramienia 1

    // Konwersja na radiany

    float radi1 = angle1 * 3.14159f / 180.0f;
    float radi2 = angle2 * 3.14159f / 180.0f;

    // Obliczenie końcowych punktów segmentów
    int jointX = baseX + (int)(length1 * cos(radi1));
    int jointY = baseY + (int)(-length1 * sin(radi1));

    int endX = jointX + (int)(length2 * cos(radi1 + radi2));
    int endY = jointY + (int)(-length2 * sin(radi1 + radi2));

    // Podstawa
    graphics.DrawRectangle(&pen, baseX - 20, baseY, 40, 10);

    // Ramię 1
    graphics.DrawLine(&pen, baseX, baseY, jointX, jointY);
    graphics.FillEllipse(&jointBrush, baseX - 5, baseY - 5, 10, 10); // przegub u podstawy

    // Ramię 2
    graphics.DrawLine(&pen, jointX, jointY, endX, endY);
    graphics.FillEllipse(&jointBrush, jointX - 5, jointY - 5, 10, 10); // przegub środkowy

    // Chwytak
    graphics.DrawLine(&pen, endX, endY, endX + 10, endY - 10);
    graphics.DrawLine(&pen, endX, endY, endX + 10, endY + 10);

}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

    HWND                hWnd;
    MSG                 msg;
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;


    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: W tym miejscu umieść kod.

    // Inicjuj ciągi globalne
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PROJEKT4RAMIEROBOTA, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Wykonaj inicjowanie aplikacji:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROJEKT4RAMIEROBOTA));

    // Główna pętla komunikatów:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    GdiplusShutdown(gdiplusToken);
    return (int) msg.wParam;
}
//
//  FUNKCJA: MyRegisterClass()
//
//  PRZEZNACZENIE: Rejestruje klasę okna.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROJEKT4RAMIEROBOTA));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PROJEKT4RAMIEROBOTA);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));


    return RegisterClassExW(&wcex);
}

//
//   FUNKCJA: InitInstance(HINSTANCE, int)
//
//   PRZEZNACZENIE: Zapisuje dojście wystąpienia i tworzy okno główne
//
//   KOMENTARZE:
//
//        W tej funkcji dojście wystąpienia jest zapisywane w zmiennej globalnej i
//        jest tworzone i wyświetlane okno główne programu.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Przechowuj dojście wystąpienia w naszej zmiennej globalnej

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNKCJA: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PRZEZNACZENIE: Przetwarza komunikaty dla okna głównego.
//
//  WM_COMMAND  - przetwarzaj menu aplikacji
//  WM_PAINT    - Maluj okno główne
//  WM_DESTROY  - opublikuj komunikat o wyjściu i wróć
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Analizuj zaznaczenia menu:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Tutaj dodaj kod rysujący używający elementu hdc...
            OnPaint(hdc);
            /////
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Procedura obsługi komunikatów dla okna informacji o programie.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ArmModule {
    const float length;
    float angle;
    float xPrevJoint;
    float yPrevJoint;
    float xEndJoint;
    float yEndingJoint;
};


class RobotArm {
public:
    float angleShoulder;
    float angleElbow;
    float velocity;
    ArmModule forearm;
    ArmModule shoulder;

    RobotArm() = default;
    RobotArm(float angle1, float angle2, float v, ArmModule firstModule, ArmModule secondModule) : angleShoulder(angle1), angleElbow(angle2), velocity(v), shoulder(firstModule), forearm(secondModule) {};
    


};


