// Projekt4_RamieRobota.cpp : Definiuje punkt wejścia dla aplikacji.
//
#include "framework.h"
#include "Projekt4_RamieRobota.h"

using namespace Gdiplus;
#pragma comment (lib, "Gdiplus.lib")

#define MAX_LOADSTRING 100
#define FLOOR_LEVEL 550.0f
#define BASE_LEVEL 20.0f
#define ROBOT_X = 300.0f

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

enum class BlockType {
    Rectangle,
    Circle,
    Triangle,
    Square
    
};

struct Block {
    float height, weight, xPoint, width, yPoint;
    BlockType bType;

    Block(float height, float weight, float x, BlockType type)
        : height(height), weight(weight), xPoint(x), bType(type)
    {
        yPoint = FLOOR_LEVEL - height;
        switch (bType) {
        case (BlockType::Rectangle):
            width = 30.0f;
            break;
        case (BlockType::Circle):
            width = height;
            break;
        case (BlockType::Square):
            width = height;
            break;
        case (BlockType::Triangle):
            width = 30.0f;
            break;
        default:
            width = 0;
            break;
        }
    }
};
class KeyManager {
public:
    bool arrowUP = false;
    bool arrowDOWN = false;
    bool arrowLEFT = false;
    bool arrowRIGHT = false;
    bool catching = false;
    bool savingData = false;
    bool playData = false;

    //tu potem można wrzucić MoveArm i wszystkie przyciski
};

class BlockManager {
private:

public:
    std::vector<Block> BlocksCollection;
    BlockManager() = default;
    void AddBlock(Block& block) {
        BlocksCollection.push_back(block);
    }
    void Clear() {
        BlocksCollection.clear();
    }
};

class RobotArm {
public:
    Block* heldBlock;
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
            [this]() { return (FLOOR_LEVEL- BASE_LEVEL); });

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
        if (right && angleShoulder <= -0.2) {
            angleShoulder += velocity*0.01f;
        }
        if (left && angleShoulder >= -2.94) {
            angleShoulder -= velocity*0.01f;
        }
        if (up) {
            angleForearm -= velocity*0.01f;
        }
        if (down) {
            angleForearm += velocity*0.01f;
        }
    }

    void TryCatch(BlockManager& blocks) {
        if (heldBlock == nullptr) {
            for (Block& b : blocks.BlocksCollection) {
                float dx = b.xPoint - forearm.xEnd(); 
                float dy = b.yPoint - forearm.yEnd();
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist < 20.0f) {
                    heldBlock = &b;
                    break;
                }
            }
        }
        else return;
    } 

    void updateHeldBlock() {
            heldBlock->xPoint = forearm.xEnd();
            heldBlock->yPoint = forearm.yEnd();
    }

    void releaseBlock() {
        if (heldBlock != nullptr) {
            heldBlock = nullptr;
        }
    }

    float getShoulderAngle() {
        return angleShoulder;
    }
    float getForearmAngle() {
        return angleForearm;
    }
    void setShoulderAngle(float fi) {
        angleShoulder = fi;
    }
    void setForearmAngle(float fi) {
        angleForearm = fi;
    }
private:
    float length = 250;
    float xBase = 300;
    static float angleShoulder;
    static float angleForearm;
    float velocity;
};

float RobotArm::angleShoulder = -1.0f;
float RobotArm::angleForearm = 1.3f;

struct SaveMove {
    std::deque<float> angleShoulder;
    std::deque<float> angleForearm;
    std::deque<bool> tryCatch;
    std::deque<BlockManager> blockSavedPositions;
    bool catching = false;
    
    SaveMove() = default;
    //SaveMove(BlockManager blocks) : blocks(blocks) {}

    void createMemory(RobotArm& MainArm, BlockManager& blockActual) {
        angleShoulder.push_back(MainArm.getShoulderAngle());
        angleForearm.push_back(MainArm.getForearmAngle());
        blockSavedPositions.push_back(blockActual);
        if (MainArm.heldBlock == nullptr) catching = false;
        else catching = true;
        tryCatch.push_back(catching);
    }

    void PlayMemory(RobotArm& MainArm, BlockManager& blocksActual, KeyManager& keys) {
        //keys.savingData = false;
        if (blockSavedPositions.size() > 0) {
            blocksActual.Clear();
            blocksActual = blockSavedPositions[0];
            MainArm.setShoulderAngle(angleShoulder[0]);
            MainArm.setForearmAngle(angleForearm[0]);
            blockSavedPositions.pop_front();
            angleShoulder.pop_front();
            angleForearm.pop_front();
        }
        else {
            keys.playData = false;
            return;
        }
    }
};

bool MovePossible(RobotArm &RobotArm, KeyManager Keys) {
    if (Keys.playData) return false;
    else return true;
}
// Zmienne globalne:
HINSTANCE hInst;                                // bieżące wystąpienie
WCHAR szTitle[MAX_LOADSTRING];                  // Tekst paska tytułu
WCHAR szWindowClass[MAX_LOADSTRING];            // nazwa klasy okna głównego

// Przekaż dalej deklaracje funkcji dołączone w tym module kodu:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

VOID OnPaint(HDC hdc, RobotArm &RobotArm, BlockManager& Blocks, KeyManager keys)
{
    Graphics graphics(hdc); 
    Color blue(255, 0, 0, 255);
    Color red(255, 255, 0, 0);
    SolidBrush redBrush(red);
    Pen pen(Color(255, 0, 0, 255), 5);
    Pen blockPen(blue, 5);
    float baseX = 300;

    graphics.DrawLine(&pen, RobotArm.shoulder.xStart(), RobotArm.shoulder.yStart(), RobotArm.shoulder.xEnd(), RobotArm.shoulder.yEnd());
    graphics.DrawLine(&pen, RobotArm.forearm.xStart(), RobotArm.forearm.yStart(), RobotArm.forearm.xEnd(), RobotArm.forearm.yEnd());

    // Podstawa
    graphics.DrawRectangle(&pen, baseX - 30.0f, FLOOR_LEVEL- BASE_LEVEL, 60.0f, BASE_LEVEL);
    graphics.DrawLine(&pen, 0.0f, FLOOR_LEVEL, 1920.0f, FLOOR_LEVEL);

    for (Block& b : Blocks.BlocksCollection) {
        switch (b.bType)
        {
        case (BlockType::Rectangle):
            graphics.DrawRectangle(&blockPen, (b.xPoint - 0.5f * b.width), b.yPoint, b.width, b.height);
            break;
        case (BlockType::Circle):
            graphics.DrawEllipse(&blockPen, (b.xPoint - 0.5f * b.width), b.yPoint, b.width, b.height);
            break;
        case (BlockType::Triangle):
                PointF points[3] = {
                PointF(b.xPoint, b.yPoint),
                PointF(b.xPoint-0.5f*b.width,b.yPoint+b.height),
                PointF(b.xPoint+0.5f*b.width,b.yPoint+b.height)
            };
                graphics.DrawPolygon(&blockPen, points, 3);
            break;
        }
    }
    if (keys.savingData == true) graphics.FillEllipse(&redBrush, 10, 10, 25, 25);
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
    static KeyManager Keys;
    static RobotArm MainArm;
    static BlockManager BlockManager;
    static SaveMove SaveMove;

    switch (message)
    {
    case WM_CREATE:
       {
        CreateWindow(L"BUTTON", L"Record", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            20, 50, 60, 30,
            hWnd, (HMENU)IDC_BUTTON_RECORD, hInst, NULL);

        CreateWindow(L"BUTTON", L"Stop", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            100, 50, 60, 30,
            hWnd, (HMENU)IDC_BUTTON_STOP, hInst, NULL);

        Block KlocekTest(100.0f, 200.0f, 600.0f, BlockType::Rectangle); //na razie w Create, później za pomocą przycisków różne ustawienia klocków można zapisać - albo dodać dodawnaie klocków
        Block KlocekTest2(80.0f, 200.0f, 700.0f, BlockType::Circle); //Block(float height, float weight, float x, BlockType type)
        Block KlocekTest3(80.0f, 200.0f, 500.0f, BlockType::Triangle);
        BlockManager.AddBlock(KlocekTest);
        BlockManager.AddBlock(KlocekTest2);
        BlockManager.AddBlock(KlocekTest3);
        SetTimer(hWnd, 1, 8, NULL);
       }
       return 0;
    case WM_KEYDOWN:
        if (wParam == VK_UP) Keys.arrowUP = true;
        if (wParam == VK_DOWN) Keys.arrowDOWN = true;
        if (wParam == VK_RIGHT) Keys.arrowRIGHT = true;
        if (wParam == VK_LEFT) Keys.arrowLEFT = true;
        if (wParam == 'P') Keys.playData = true;
        if (Keys.playData == false) {
            if (wParam == VK_SPACE) {
                Keys.catching = true;
                MainArm.TryCatch(BlockManager);
            }
        }
        if (wParam == 'R') MainArm.releaseBlock();
        return 0;
    case WM_KEYUP:
        if (wParam == VK_UP) Keys.arrowUP = false;
        if (wParam == VK_DOWN) Keys.arrowDOWN = false;
        if (wParam == VK_RIGHT) Keys.arrowRIGHT = false;
        if (wParam == VK_LEFT) Keys.arrowLEFT = false;
        if (wParam == VK_SPACE) Keys.catching = false;
        return 0;
    case WM_TIMER:
        // tutaj można da funkcje która wykonuje się co ileś czasu
        if (MovePossible(MainArm, Keys)) {
            MainArm.MoveArm(Keys.arrowUP, Keys.arrowDOWN, Keys.arrowRIGHT, Keys.arrowLEFT);
        }
        if(MainArm.heldBlock != nullptr) MainArm.updateHeldBlock();
        if (Keys.savingData) SaveMove.createMemory(MainArm, BlockManager);
        if (Keys.playData) SaveMove.PlayMemory(MainArm, BlockManager, Keys);
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Analizuj zaznaczenia menu:
            switch (wmId)
            {
            case IDC_BUTTON_RECORD:
                Keys.savingData = true;
                SetFocus(hWnd);
                break;
            case IDC_BUTTON_STOP:
                Keys.savingData = false;
                SetFocus(hWnd);
                break;
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

            Rect buttonRect = { 0, 50, 200, 30};
       
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(hdcMemory, memBitmap);

            ExcludeClipRect(hdcMemory, buttonRect.GetLeft(), buttonRect.GetTop(), buttonRect.GetRight(), buttonRect.GetBottom());
            FillRect(hdcMemory, &rect, (HBRUSH)(COLOR_WINDOW + 1));
            OnPaint(hdcMemory, MainArm, BlockManager, Keys);
            ExcludeClipRect(hdc, buttonRect.GetLeft(), buttonRect.GetTop(), buttonRect.GetRight(), buttonRect.GetBottom());
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