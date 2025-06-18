// Projekt4_RamieRobota.cpp : Definiuje punkt wejścia dla aplikacji.
//
#include "framework.h"
#include "Projekt4_RamieRobota.h"
#include <cmath>
#include <objidl.h>
#include <gdiplus.h>
#include <functional>

#include <iostream>
#include <chrono>
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
    //float velocity =1.0f;
    ArmModule shoulder;
    ArmModule forearm;

    RobotArm(float vel = 1.0f)
        :
        shoulder(0, []() { return 0.0f; }, []() { return 0.0f; }, []() { return 0.0f; }),
        forearm(0, []() { return 0.0f; }, []() { return 0.0f; }, []() { return 0.0f; })
    {
        setVelocity(vel);
        shoulder = ArmModule(length,
            [this]() { return angleShoulder; },
            [this]() { return xBase; },
            [this]() { return yBase; });

        forearm = ArmModule(length,
            [this]() { return angleShoulder + angleForearm; },
            [this]() { return shoulder.xEnd(); },
            [this]() { return shoulder.yEnd(); });
    }

    float getVelocity() const { return velocity; }
    void setVelocity(float v) {
        if (v < 0.0f) velocity = 0.0f;
        else if (v > 100.0f) velocity = 100.0f;
        else velocity = v;
    }

    void MoveArm(bool up, bool down, bool right, bool left) {
        if (right) {
            angleShoulder += velocity*0.01;
        }
        if (left) {
            angleShoulder -= velocity*0.01;
        }
        if (up) {
            angleForearm -= velocity*0.01;
        }
        if (down) {
            angleForearm += velocity*0.01;
        }
    }
private:
    float length = 200;
    float xBase = 300;
    float yBase = 400;
    static float angleShoulder;
    static float angleForearm;
    float velocity;
};

float RobotArm::angleShoulder = -45.0f;
float RobotArm::angleForearm = 70.0f;

bool MovePossible(RobotArm &RobotArm) {
    if (RobotArm.forearm.yEnd() <= 410) {
        return true;
    }
    else return true;
}


// Zmienne globalne:
HINSTANCE hInst;                                // bieżące wystąpienie
WCHAR szTitle[MAX_LOADSTRING];                  // Tekst paska tytułu
WCHAR szWindowClass[MAX_LOADSTRING];            // nazwa klasy okna głównego
bool arrowUP = false;
bool arrowDOWN = false;
bool arrowLEFT = false;
bool arrowRIGHT = false;

// Przekaż dalej deklaracje funkcji dołączone w tym module kodu:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

VOID OnPaint(HDC hdc, RobotArm &RobotArm)
{
    Graphics graphics(hdc);
    Pen pen(Color(255, 0, 0, 255), 5);
    float baseX = 300;
    float baseY = 400;

    graphics.DrawLine(&pen, RobotArm.shoulder.xStart(), RobotArm.shoulder.yStart(), RobotArm.shoulder.xEnd(), RobotArm.shoulder.yEnd());
    graphics.DrawLine(&pen, RobotArm.forearm.xStart(), RobotArm.forearm.yStart(), RobotArm.forearm.xEnd(), RobotArm.forearm.yEnd());

    // Podstawa
    graphics.DrawRectangle(&pen, baseX - 20.0f, baseY, 40.0f, 10.0f);
    graphics.DrawLine(&pen, 0.0f, baseY+10, 1920.0f, baseY+10);
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

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
    static RobotArm MainArm;

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
        if (MovePossible(MainArm)) {
            MainArm.MoveArm(arrowUP, arrowDOWN, arrowRIGHT, arrowLEFT);
        }
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
            HDC hdcMemory = CreateCompatibleDC(hdc); //podwojne buforowanie bo mi migało

            RECT rect;
            GetClientRect(hWnd, &rect);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);


            HBITMAP oldBitmap = (HBITMAP)SelectObject(hdcMemory, memBitmap);
            FillRect(hdcMemory, &rect, (HBRUSH)(COLOR_WINDOW + 1));
            OnPaint(hdcMemory, MainArm);
            BitBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hdcMemory, 0, 0, SRCCOPY);

            SelectObject(hdcMemory, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(hdcMemory);

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
