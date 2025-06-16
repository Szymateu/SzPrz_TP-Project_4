// Projekt4_RamieRobota.cpp : Definiuje punkt wejścia dla aplikacji.
//
#include "framework.h"
#include "Projekt4_RamieRobota.h"
#include <cmath>
#include <objidl.h>
#include <gdiplus.h>
#include <functional>
using namespace Gdiplus;
#pragma comment (lib, "Gdiplus.lib")

#define MAX_LOADSTRING 100

struct ArmModule {
    std::function<float()> xStartJoint;
    std::function<float()> yStartJoint;
    std::function<float()> angleMod;

    float length;

    ArmModule(float l,std::function<float()> fi, std::function<float()> xsj, std::function<float()> ysj) : length(l), angleMod(fi), xStartJoint(xsj), yStartJoint(ysj) {};
    
    float xStart() const {
        return xStartJoint();
    }

    float yStart() const {
        return yStartJoint();
    }

    float angle() const {
        return angleMod();
    }

    float xEnd() const {
        return  xStart() + length * cos(angle());
    }
    float yEnd() const {
        return yStart() + length * sin(angle());
    }
};

struct Klocek {
    float height;
    float weight;
    int type; //1- koło 2- trójkąt itd??
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

    void angleChange(ArmModule module) {
        
    }

};


// Zmienne globalne:
HINSTANCE hInst;                                // bieżące wystąpienie
WCHAR szTitle[MAX_LOADSTRING];                  // Tekst paska tytułu
WCHAR szWindowClass[MAX_LOADSTRING];            // nazwa klasy okna głównego

// Przekaż dalej deklaracje funkcji dołączone w tym module kodu:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


VOID OnPaint(HDC hdc, ArmModule &baseModule, ArmModule &secondModule) //generated paint func
{

    Graphics graphics(hdc);
    Pen pen(Color(255, 0, 0, 255), 5); // Czerwony, gruby długopis
    SolidBrush jointBrush(Color(255, 0, 0, 255)); // Czerwony pędzel do przegubów
    int baseX = 300;
    float baseXf = 300;
    int baseY = 400;
    float baseYf = 400;

    float l = 300;

    graphics.DrawLine(&pen, baseModule.xStart(), baseModule.yStart(), baseModule.xEnd(), baseModule.yEnd());
    graphics.DrawLine(&pen, secondModule.xStart(), secondModule.yStart(), secondModule.xEnd(), secondModule.yEnd());

    // Pozycje bazowe


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

bool arrowUP = false;
bool arrowDOWN = false;
bool arrowLEFT = false;
bool arrowRIGHT = false;

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
    static float angle = 0;
    static float angle2 = 0;
    static const float xBase = 300;
    static const float yBase = 400;
    static ArmModule shoulder(200,
        [&]() {return angle; },
        [&]() {return xBase; },
        [&]() {return yBase; });
    static ArmModule forearm(200,
        [&]() {return angle2; },
        [&]() {return shoulder.xEnd(); },
        [&]() {return shoulder.yEnd(); });

    RobotArm MainArm(angle, angle, 2, shoulder, forearm);

    switch (message)
    {
    case WM_KEYDOWN:
        if (wParam == VK_UP) arrowUP = true;
        if (wParam == VK_DOWN) arrowDOWN = true;
        if (wParam == VK_RIGHT) arrowRIGHT = true;
        if (wParam == VK_LEFT) arrowLEFT = true;
        return 0;
    case WM_KEYUP:
        if (wParam == VK_UP) arrowUP = false;
        if (wParam == VK_DOWN) arrowDOWN = false;
        if (wParam == VK_RIGHT) arrowRIGHT = false;
        if (wParam == VK_LEFT) arrowLEFT = false;
        return 0;
    case WM_CREATE:
       {
            SetTimer(hWnd, 1, 8, NULL);
       }
       return 0;
    case WM_TIMER:
        // tutaj można da funkcje która wykonuje się co ileś czasu
        if (arrowUP) {
            angle += 0.01;
        }
        if (arrowDOWN) angle -= 0.01;
        if (arrowLEFT) angle2 += 0.01;
        if (arrowRIGHT) angle2 -= 0.01;
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
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
            HDC memory = CreateCompatibleDC(hdc); //podwojne buforowanie bo mi migało

            RECT rect;
            GetClientRect(hWnd, &rect);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);


            HBITMAP oldBitmap = (HBITMAP)SelectObject(memory, memBitmap);
            FillRect(memory, &rect, (HBRUSH)(COLOR_WINDOW + 1));
            OnPaint(memory, shoulder, forearm);
            BitBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, memory, 0, 0, SRCCOPY);

            SelectObject(memory, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memory);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_ERASEBKGND:
        return TRUE;
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
