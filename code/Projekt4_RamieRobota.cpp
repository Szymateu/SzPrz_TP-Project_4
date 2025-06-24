// Projekt4_RamieRobota.cpp : Definiuje punkt wejścia dla aplikacji.
//
#include "framework.h"
#include "Projekt4_RamieRobota.h"

using namespace Gdiplus;
#pragma comment (lib, "Gdiplus.lib")

#define MAX_LOADSTRING 100
#define FLOOR_LEVEL 550.0f // ground level from top of screen
#define BASE_LEVEL 20.0f // RobotArm y position relative to ground
#define ROBOT_X 500.0f // RobotArm x position
int velocityGlobal = 1;

enum SortType {
    minMax,
    maxMin,
    minMaxMin
};

struct ArmModule {
    std::function<float()> xStartJoint;
    std::function<float()> yStartJoint;
    std::function<float()> angleMod;

    float length;

    ArmModule(float l, std::function<float()> fi, std::function<float()> xsj, std::function<float()> ysj) : length(l), angleMod(fi), xStartJoint(xsj), yStartJoint(ysj) {};

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
        return yStart() - length * sin(angle());
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
    bool autoMoved = false;

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

    bool operator==(const Block& other) const {
        return height == other.height &&
            weight == other.weight &&
            xPoint == other.xPoint &&
            yPoint == other.yPoint &&
            width == other.width &&
            bType == other.bType &&
            autoMoved == other.autoMoved;
    }
};

struct TargetAngle {
    float angleShoulder = 0.0f;
    float angleForearm = 0.0f;
    TargetAngle() = default;
    TargetAngle(float angle1, float angle2) : angleShoulder(angle1), angleForearm(angle2) {}
};

class KeyManager { //wsm to po prostu zmienne globalne tu przechowuje xd
public:
    bool arrowUP = false;
    bool arrowDOWN = false;
    bool arrowLEFT = false;
    bool arrowRIGHT = false;
    bool catching = false;
    bool savingData = false;
    bool playData = false;
    bool moveToStart = false;
    float shoulderStart = 1.0f;
    float forearmStart = 1.3f;
    bool angleMove = false;
    bool towerBuild = false;
    BlockType towerType = BlockType::Rectangle;
    float xTower = 0;
    float yBrick = 0;
    float lastBlockHeightInTower = FLOOR_LEVEL;
    float towerHeight = 0;
    //Sortowanieee
    bool searchingForBlock = false;
    int searchBlockIndex = 0;
    std::vector<std::tuple<Block, float>> heightOfBlocksVec;
    SortType sortType = SortType::maxMin;

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

enum class BuildTower {
    GoToStart,
    GoToBrick,
    CatchBrick,
    MoveArmUp,
    MoveBrickToTower,
    ReleaseBrick
};

enum class SortBlocks {
    MeasureBlocks,
    MoveBlocks
};

class RobotArm {
public:
    Block* heldBlock = nullptr;
    ArmModule shoulder;
    ArmModule forearm;
    bool checkCollisions = true;

    RobotArm()
        :
        shoulder(0, []() { return 0.0f; }, []() { return 0.0f; }, []() { return 0.0f; }),
        forearm(0, []() { return 0.0f; }, []() { return 0.0f; }, []() { return 0.0f; })
    {
        shoulder = ArmModule(length,
            [this]() { return angleShoulder; },
            [this]() { return ROBOT_X; },
            [this]() { return (FLOOR_LEVEL - BASE_LEVEL); });

        forearm = ArmModule(length,
            [this]() { return angleShoulder + angleForearm; },
            [this]() { return shoulder.xEnd(); },
            [this]() { return shoulder.yEnd(); });
    }

    RobotArm(float setLengthFloat)
        :
        shoulder(0, []() { return 0.0f; }, []() { return 0.0f; }, []() { return 0.0f; }),
        forearm(0, []() { return 0.0f; }, []() { return 0.0f; }, []() { return 0.0f; })
    {
        length = setLengthFloat;

        shoulder = ArmModule(setLengthFloat,
            [this]() { return angleShoulder; },
            [this]() { return ROBOT_X; },
            [this]() { return (FLOOR_LEVEL - BASE_LEVEL); });

        forearm = ArmModule(setLengthFloat,
            [this]() { return angleShoulder + angleForearm; },
            [this]() { return shoulder.xEnd(); },
            [this]() { return shoulder.yEnd(); });
    }

    void MoveArm(bool up, bool down, bool right, bool left) {
        while (angleShoulder > 6.283185) angleShoulder -= 6.283185;
        while (angleShoulder < 0) angleShoulder += 6.283185;
        while (angleForearm > 3.141593) angleForearm -= 6.283185;
        while (angleForearm < -3.141593) angleForearm += 6.283185;

        if (right
            && (!checkCollisions
                || angleShoulder >= 0.1f
                && (forearm.yEnd() <= FLOOR_LEVEL
                    || angleShoulder >= 1.570796f))) {
            angleShoulder -= velocityGlobal * 0.002f + 0.01f;
        }
        if (left
            && (!checkCollisions
                || angleShoulder <= 3.0416f
                && (forearm.yEnd() <= FLOOR_LEVEL
                    || angleShoulder <= 1.570796f))) {
            angleShoulder += velocityGlobal * 0.002f + 0.01f;
        }
        if (up
            && (!checkCollisions
                || angleForearm >= -3.1f
                && (forearm.yEnd() <= FLOOR_LEVEL
                    || (angleShoulder <= 1.570796f && angleShoulder + angleForearm <= -1.570796f)
                    || (angleShoulder >= 1.570796f && angleShoulder + angleForearm <= 4.7124f)))) {
            angleForearm -= velocityGlobal * 0.002f + 0.01f;
        }
        if (down
            && (!checkCollisions
                || angleForearm <= 3.1f
                && (forearm.yEnd() <= FLOOR_LEVEL
                    || (angleShoulder <= 1.570796f && angleShoulder + angleForearm >= -1.570796f)
                    || (angleShoulder >= 1.570796f && angleShoulder + angleForearm >= 4.7124f)))) {
            angleForearm += velocityGlobal * 0.002f + 0.01f;
        }
    }

    void MoveAngle(TargetAngle& a, KeyManager& key, bool& confirmation) {
        while (a.angleShoulder > 6.283185) a.angleShoulder -= 6.283185;
        while (a.angleShoulder < 0) a.angleShoulder += 6.283185;
        while (a.angleForearm > 3.141593) a.angleForearm -= 6.283185;
        while (a.angleForearm < -3.141593) a.angleForearm += 6.283185;
        while (angleShoulder > 6.283185) angleShoulder -= 6.283185;
        while (angleShoulder < 0) angleShoulder += 6.283185;
        while (angleForearm > 3.141593) angleForearm -= 6.283185;
        while (angleForearm < -3.141593) angleForearm += 6.283185;

        if (std::abs(angleShoulder - a.angleShoulder) > 0.02f) {
            if (angleShoulder < a.angleShoulder) key.arrowLEFT = true;
            else if (angleShoulder > a.angleShoulder) key.arrowRIGHT = true;
        }
        else {
            key.arrowRIGHT = false;
            key.arrowLEFT = false;
        }
        if (std::abs(angleForearm - a.angleForearm) > 0.02f) {
            if (angleForearm < a.angleForearm) key.arrowDOWN = true;
            else if (angleForearm > a.angleForearm) key.arrowUP = true;
        }
        else {
            key.arrowDOWN = false;
            key.arrowUP = false;
        }
        if (key.arrowRIGHT == false && key.arrowLEFT == false && key.arrowDOWN == false && key.arrowUP == false) confirmation = false; //bo jak jest true to kontynuuje ruch
        else confirmation = true;
    }

    std::optional<std::tuple<TargetAngle, TargetAngle>> PointsToAngle(float x, float y) {
        float L1 = shoulder.length;
        float L2 = forearm.length;

        x = x - ROBOT_X;
        y = y - (FLOOR_LEVEL - BASE_LEVEL);

        float r_squared = x * x + y * y;
        float r = std::sqrt(r_squared);

        if (r > L1 + L2 || r < std::abs(L1 - L2)) {
            return std::nullopt;
        }

        float cosTheta2 = (r_squared - L1 * L1 - L2 * L2) / (2 * L1 * L2);
        if (cosTheta2 > 1.0f) cosTheta2 = 1.0f;
        if (cosTheta2 < -1.0f) cosTheta2 = -1.0f;

        float theta2_1 = std::acos(cosTheta2);
        float theta2_2 = -theta2_1;

        float k1 = L1 + L2 * std::cos(theta2_1);
        float k2 = L2 * std::sin(theta2_1);
        float theta1_1 = std::atan2(-y, x) - std::atan2(k2, k1);

        k1 = L1 + L2 * std::cos(theta2_2);
        k2 = L2 * std::sin(theta2_2);
        float theta1_2 = std::atan2(-y, x) - std::atan2(k2, k1);

        TargetAngle Angles1 = { theta1_1, theta2_1 };
        TargetAngle Angles2 = { theta1_2, theta2_2 };

        return std::make_optional(std::make_tuple(Angles1, Angles2));
    }

    bool TryCatch(BlockManager& blocks) {
        if (heldBlock == nullptr) {
            for (Block& b : blocks.BlocksCollection) {
                float dx = b.xPoint - forearm.xEnd();
                float dy = b.yPoint - forearm.yEnd();
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist < 20.0f) {
                    if (b.weight < maxLoad) {
                        heldBlock = &b;
                        return true;
                    }
                    else {
                        return false; // i włącz jakiś alarm
                    }
                }
            }
        }
        return true;
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
    float xBase = ROBOT_X;
    static float angleShoulder;
    static float angleForearm;
    float maxLoad = 80.0f;
};

// program start - RobotArm positions
float RobotArm::angleShoulder = 0.785f;
float RobotArm::angleForearm = 1.57f;

struct SaveMove {
    std::deque<float> angleShoulder;
    std::deque<float> angleForearm;
    std::deque<bool> tryCatch;
    std::deque<BlockManager> blockSavedPositions;
    bool catching = false;

    SaveMove() = default;

    void createMemory(RobotArm& MainArm, BlockManager& blockActual) {
        angleShoulder.push_back(MainArm.getShoulderAngle());
        angleForearm.push_back(MainArm.getForearmAngle());
        blockSavedPositions.push_back(blockActual);
        if (MainArm.heldBlock == nullptr) catching = false;
        else catching = true;
        tryCatch.push_back(catching);
    }

    void PlayMemory(RobotArm& MainArm, BlockManager& blocksActual, KeyManager& keys) {
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

class Automation {
public:
    RobotArm& MainArm;

    Automation(RobotArm& RobotArm) : MainArm(RobotArm) {}

    TargetAngle Tower(BlockType& type, KeyManager& keys, BuildTower& State, BlockManager& Blocks) {
        for (Block& b : Blocks.BlocksCollection) {
            if (b.bType == type && b.autoMoved == false) {
                switch (State) {
                case BuildTower::GoToStart:
                    keys.xTower = b.xPoint;
                    keys.yBrick = b.yPoint;
                    b.autoMoved = true;
                    break;
                case BuildTower::GoToBrick:
                {
                    auto anglesOpt2 = MainArm.PointsToAngle(b.xPoint, b.yPoint);
                    if (anglesOpt2.has_value()) {
                        float ang0 = std::get<0>(*anglesOpt2).angleShoulder;
                        float ang1 = std::get<1>(*anglesOpt2).angleShoulder;
                        if (ang0 >= 1.570796f && ang0 < 4.712389f) ang0 = 3.141593f - ang0;
                        if (ang0 >= 4.712389f) ang0 -= 6.283185f;
                        if (ang1 >= 1.570796f && ang1 < 4.712389f) ang1 = 3.141593f - ang1;
                        if (ang1 >= 4.712389f) ang1 -= 6.283185f;
                        if (ang1 > ang0) {
                            return std::get<1>(*anglesOpt2);
                        }
                        else return std::get<0>(*anglesOpt2);
                    }
                }break;
                case BuildTower::MoveBrickToTower:
                {
                    keys.yBrick = keys.yBrick - b.height;
                    auto anglesOpt3 = MainArm.PointsToAngle((keys.xTower), (keys.yBrick)); // szymon: +1 żeby przesunąć o histereze - wsm można by regulować histereze dla prędkośc i wtedy bardzopowoli bedzie bardzo dokładnie, ale czasu nie ma
                        // przemek: usunąłem bo to nic nie zmienia xd
                    b.autoMoved = true;
                    if (anglesOpt3.has_value()) {
                        float ang0 = std::get<0>(*anglesOpt3).angleShoulder;
                        float ang1 = std::get<1>(*anglesOpt3).angleShoulder;
                        if (ang0 >= 1.570796f && ang0 < 4.712389f) ang0 = 3.141593f - ang0;
                        if (ang0 >= 4.712389f) ang0 -= 6.283185f;
                        if (ang1 >= 1.570796f && ang1 < 4.712389f) ang1 = 3.141593f - ang1;
                        if (ang1 >= 4.712389f) ang1 -= 6.283185f;
                        if (ang1 > ang0) {
                            return std::get<1>(*anglesOpt3);
                        }
                        else return std::get<0>(*anglesOpt3);
                    }
                }break;
                }
                auto anglesOpt = MainArm.PointsToAngle(keys.xTower, keys.yBrick);
                if (anglesOpt.has_value()) {
                    return std::get<1>(*anglesOpt);
                }
            }
        }
        return TargetAngle(0.785f, 1.57f);
    }

    TargetAngle minMaxSort(BlockManager& blocks, SortType& sortType, KeyManager& Keys, int& blockIndex) {
        std::vector<Block> sortedBlocks;
        TargetAngle moveAngle;
        for (Block& b : blocks.BlocksCollection) {
            if (b == blocks.BlocksCollection[blockIndex]) {
                if (1) {//tu trzeba zrobić żeby wybierał czy ramieniem w góre czy w dół
                    moveAngle = std::get<1>(*MainArm.PointsToAngle(b.xPoint, b.yPoint));
                    Keys.heightOfBlocksVec.push_back(std::make_tuple(b, (FLOOR_LEVEL - MainArm.forearm.yEnd())));
                }
                else if (0) moveAngle = std::get<0>(*MainArm.PointsToAngle(b.xPoint, b.yPoint));
                //no tu jest zepsute bo to trzeba poza funkcją żeby teraz się ruszało i wgl - bleee
                break;
            }
        }

        return moveAngle;


        ////to na razie odcinam - chyba druga funkcja będzie
        // 
        //if (0) {
        //    switch (sortType) {
        //    case SortType::minMax: {
        //        std::sort(Keys.heightOfBlocksVec.begin(), Keys.heightOfBlocksVec.end(),
        //            [](const std::tuple<Block, float>& a, const std::tuple<Block, float>& b) {
        //                return std::get<1>(a) < std::get<1>(b); // sortuj jeśli a mniejsze od b to a będzie pierwsze
        //            });
        //        break;
        //    }
        //    case SortType::maxMin: {
        //        std::sort(Keys.heightOfBlocksVec.begin(), Keys.heightOfBlocksVec.end(),
        //            [](const std::tuple<Block, float>& a, const std::tuple<Block, float>& b) {
        //                return std::get<1>(a) > std::get<1>(b);
        //            });
        //        break;
        //    }
        //    case SortType::minMaxMin: {
        //        //na razie nie można xd
        //        break;
        //    }
        //    }
        //    for (auto& tuple : Keys.heightOfBlocksVec) {
        //        sortedBlocks.push_back(std::get<0>(tuple));
        //    }
            //return sortedBlocks;
        //}
    }
};

bool MovePossible(RobotArm& RobotArm, KeyManager Keys) {
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

VOID OnPaint(HDC hdc, RobotArm& RobotArm, BlockManager& Blocks, KeyManager keys)
{
    Graphics graphics(hdc);
    Color blue(255, 0, 0, 255);
    Color red(255, 255, 0, 0);
    Color green(255, 0, 255, 0);
    SolidBrush redBrush(red);
    SolidBrush greenBrush(green);
    Pen pen(Color(255, 0, 0, 255), 5);
    Pen blockPen(blue, 5);


    graphics.DrawLine(&pen, RobotArm.shoulder.xStart(), RobotArm.shoulder.yStart(), RobotArm.shoulder.xEnd(), RobotArm.shoulder.yEnd());
    graphics.DrawLine(&pen, RobotArm.forearm.xStart(), RobotArm.forearm.yStart(), RobotArm.forearm.xEnd(), RobotArm.forearm.yEnd());

    // Podstawa
    graphics.DrawRectangle(&pen, ROBOT_X - 30.0f, FLOOR_LEVEL - BASE_LEVEL, 60.0f, BASE_LEVEL);
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
        case (BlockType::Square):
            graphics.DrawRectangle(&blockPen, (b.xPoint - 0.5f * b.width), b.yPoint, b.width, b.height);
            break;
        case (BlockType::Triangle):
            PointF points[3] = {
            PointF(b.xPoint, b.yPoint),
            PointF(b.xPoint - 0.5f * b.width,b.yPoint + b.height),
            PointF(b.xPoint + 0.5f * b.width,b.yPoint + b.height)
            };
            graphics.DrawPolygon(&blockPen, points, 3);
            break;
        }
    }
    if (keys.savingData == true) graphics.FillEllipse(&redBrush, 20, 10, 25, 25);
    if (keys.playData == true) {
        Point playTriangle[3] = { Point(20, 10), Point(20, 35), Point(45, 22) };
        graphics.FillPolygon(&greenBrush, playTriangle, 3);
    }
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
    return (int)msg.wParam;
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

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    HWND hScrollBar = CreateWindowEx(
        0,
        L"SCROLLBAR",
        NULL,
        WS_CHILD | WS_VISIBLE | SBS_HORZ,
        100, 105,
        300, 20,
        hWnd,
        (HMENU)1,
        hInstance,
        NULL
    );

    SetScrollRange(hScrollBar, SB_CTL, 0, 10, TRUE);
    SetScrollPos(hScrollBar, SB_CTL, velocityGlobal, TRUE);

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

    static BuildTower Building;
    static SortBlocks Sorting;
    static KeyManager Keys;
    static RobotArm MainArm(250);
    static BlockManager BlockManager;
    static SaveMove SaveMove;
    static Automation Automation(MainArm);
    static TargetAngle StartPosition(Keys.shoulderStart, Keys.forearmStart);
    static TargetAngle MovePosition;
    static bool MoveInProgress;

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

        CreateWindow(L"BUTTON", L"Play", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            20, 100, 60, 30,
            hWnd, (HMENU)IDC_BUTTON_PLAY, hInst, NULL);

        ////////////     ADD BLOCKS 
        HWND comboShapeAddBlock = CreateWindowW(L"COMBOBOX", NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            900, 10, 120, 100,
            hWnd, (HMENU)IDC_COMBO_SHAPE, hInst, NULL);
        SendMessage(comboShapeAddBlock, CB_ADDSTRING, 0, (LPARAM)L"Rectangle");
        SendMessage(comboShapeAddBlock, CB_ADDSTRING, 0, (LPARAM)L"Circle");
        SendMessage(comboShapeAddBlock, CB_ADDSTRING, 0, (LPARAM)L"Square");
        SendMessage(comboShapeAddBlock, CB_ADDSTRING, 0, (LPARAM)L"Triangle");
        SendMessage(comboShapeAddBlock, CB_SETCURSEL, 0, 0);

        CreateWindowW(L"EDIT", L"100",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            900, 40, 120, 20,
            hWnd, (HMENU)IDC_HEIGHT, hInst, NULL);

        CreateWindowW(L"EDIT", L"20",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            900, 70, 120, 20,
            hWnd, (HMENU)IDC_WEIGHT, hInst, NULL);

        CreateWindowW(L"EDIT", L"200",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            900, 100, 120, 20,
            hWnd, (HMENU)IDC_XPOS, hInst, NULL);

        CreateWindowW(L"BUTTON", L"Create",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            900, 130, 120, 30,
            hWnd, (HMENU)IDC_BUTTON_CREATE, hInst, NULL);

        CreateWindowW(L"STATIC", L"Shape", WS_CHILD | WS_VISIBLE,
            840, 12, 50, 20, hWnd, NULL, hInst, NULL);
        CreateWindowW(L"STATIC", L"Height", WS_CHILD | WS_VISIBLE,
            840, 42, 50, 20, hWnd, NULL, hInst, NULL);
        CreateWindowW(L"STATIC", L"Weight", WS_CHILD | WS_VISIBLE,
            840, 72, 50, 20, hWnd, NULL, hInst, NULL);
        CreateWindowW(L"STATIC", L"Position", WS_CHILD | WS_VISIBLE,
            840, 102, 60, 20, hWnd, NULL, hInst, NULL);
        CreateWindow(L"BUTTON", L"CLEAR", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            750, 10, 60, 30,
            hWnd, (HMENU)IDC_BUTTON_CLEAR, hInst, NULL);

        /// AUTOMATION
        CreateWindowW(L"STATIC", L"Automation", WS_CHILD | WS_VISIBLE,
            178, 12, 80, 20, hWnd, NULL, hInst, NULL);

        HWND comboTowerShape = CreateWindowW(L"COMBOBOX", NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
            260, 10, 140, 100,
            hWnd, (HMENU)IDC_COMBO_TOWER, hInst, NULL);
        SendMessage(comboTowerShape, CB_ADDSTRING, 0, (LPARAM)L"Tower Rectangle");
        SendMessage(comboTowerShape, CB_ADDSTRING, 0, (LPARAM)L"Tower Circle");
        SendMessage(comboTowerShape, CB_ADDSTRING, 0, (LPARAM)L"Tower Square");
        SendMessage(comboTowerShape, CB_ADDSTRING, 0, (LPARAM)L"Tower Triangle");
        SendMessage(comboTowerShape, CB_SETCURSEL, 0, 0);

        CreateWindow(L"BUTTON", L"START", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            260, 35, 60, 30,
            hWnd, (HMENU)IDC_BUTTON_START, hInst, NULL);

        Block KlocekTest(100.0f, 70.0f, 700.0f, BlockType::Rectangle); //na razie w Create, później za pomocą przycisków różne ustawienia klocków można zapisać - albo dodać dodawnaie klocków
        Block KlocekTest2(80.0f, 70.0f, 800.0f, BlockType::Circle); //Block(float height, float weight, float x, BlockType type)
        Block KlocekTest3(70.0f, 70.0f, 300.0f, BlockType::Triangle);
        Block KlocekTest4(50.0f, 70.0f, 100.0f, BlockType::Square);
        BlockManager.AddBlock(KlocekTest);
        BlockManager.AddBlock(KlocekTest2);
        BlockManager.AddBlock(KlocekTest3);
        BlockManager.AddBlock(KlocekTest4);
        SetTimer(hWnd, 1, 8, NULL);
    }
    return 0;
    case WM_KEYDOWN:
        if (wParam == VK_UP) Keys.arrowUP = true;
        if (wParam == VK_DOWN) Keys.arrowDOWN = true;
        if (wParam == VK_RIGHT) Keys.arrowRIGHT = true;
        if (wParam == VK_LEFT) Keys.arrowLEFT = true;
        if (wParam == 'T') Keys.moveToStart = true;
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
        // tutaj można dac funkcje która wykonuje się co klatke
        if (MovePossible(MainArm, Keys)) {
            MainArm.MoveArm(Keys.arrowUP, Keys.arrowDOWN, Keys.arrowRIGHT, Keys.arrowLEFT);
        }
        if (MainArm.heldBlock != nullptr) MainArm.updateHeldBlock();

        if (Keys.savingData) SaveMove.createMemory(MainArm, BlockManager);
        if (Keys.playData) SaveMove.PlayMemory(MainArm, BlockManager, Keys);

        if (Keys.moveToStart) MainArm.MoveAngle(StartPosition, Keys, Keys.moveToStart);
        if (MoveInProgress)  MainArm.MoveAngle(MovePosition, Keys, MoveInProgress);

        if (Keys.towerHeight > 5) {
            Keys.towerBuild = false;
            Building = BuildTower::GoToStart;
            Keys.towerHeight = 0;
            for (Block& b : BlockManager.BlocksCollection)
            {
                if (b.bType == Keys.towerType)
                    b.autoMoved = false;
            }
        }
        if (Keys.searchingForBlock) MainArm.MoveAngle(MovePosition, Keys, Keys.searchingForBlock);

        if (Keys.towerBuild) {
            switch (Building) {
            case(BuildTower::GoToStart):
            {
                Keys.moveToStart = true;
                MovePosition = Automation.Tower(Keys.towerType, Keys, Building, BlockManager); // najpierw znajdujemy pierwszy klocek
                Building = BuildTower::GoToBrick;
                break;
            }
            case(BuildTower::GoToBrick):
            {
                if (Keys.moveToStart == false) {
                    MovePosition = Automation.Tower(Keys.towerType, Keys, Building, BlockManager); // teraz koordynaty drugiego
                    MoveInProgress = true;
                    Building = BuildTower::CatchBrick;
                }
                break;
            }
            case(BuildTower::CatchBrick):
            {
                if (MoveInProgress == false) {
                    MainArm.TryCatch(BlockManager);
                    Building = BuildTower::MoveBrickToTower;
                    MovePosition = Automation.Tower(Keys.towerType, Keys, Building, BlockManager); //teraz przenosimy drugi na pierwszy
                }
                break;
            }
            case(BuildTower::MoveArmUp):
            {
                //na razie skip xd
                break;
            }
            case(BuildTower::MoveBrickToTower):
            {
                MoveInProgress = true;
                Building = BuildTower::ReleaseBrick;
                break;
            }
            case(BuildTower::ReleaseBrick): {
                if (MoveInProgress == false) {
                    MainArm.releaseBlock();
                    Keys.towerHeight++;
                    Building = BuildTower::GoToBrick;
                }
                break;
            }
            }
        }

        if (Keys.searchingForBlock) {
            switch (Sorting) {
            case (SortBlocks::MeasureBlocks):
                //jeśli moveposition jest - jak sie skoncza bloki to przejdz do nastepnego krokuu
                if (Keys.searchBlockIndex <= BlockManager.BlocksCollection.size() && Keys.searchingForBlock == false) {
                    MovePosition = Automation.minMaxSort(BlockManager, Keys.sortType, Keys, Keys.searchBlockIndex);
                    Keys.searchingForBlock = true;
                }
                else if (Keys.searchBlockIndex > BlockManager.BlocksCollection.size() && Keys.searchingForBlock == false) {
                    Sorting = SortBlocks::MoveBlocks;
                }

                break;
            case (SortBlocks::MoveBlocks):
                break;
            }
        }
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);

            switch (wmId)
            {
            case IDC_BUTTON_CREATE:
            {
                // Pobierz dane z pól
                HWND addCombo = GetDlgItem(hWnd, IDC_COMBO_SHAPE);
                int index = (int)SendMessage(addCombo, CB_GETCURSEL, 0, 0);

                wchar_t heightStr[16], weightStr[16], xStr[16];
                GetWindowText(GetDlgItem(hWnd, IDC_HEIGHT), heightStr, 16);
                GetWindowText(GetDlgItem(hWnd, IDC_WEIGHT), weightStr, 16);
                GetWindowText(GetDlgItem(hWnd, IDC_XPOS), xStr, 16);

                float height = wcstof(heightStr, nullptr);
                float weight = wcstof(weightStr, nullptr);
                float xPos = wcstof(xStr, nullptr);

                BlockType type;
                switch (index) {
                case 0: type = BlockType::Rectangle; break;
                case 1: type = BlockType::Circle; break;
                case 2: type = BlockType::Square; break;
                case 3: type = BlockType::Triangle; break;
                default: type = BlockType::Rectangle; break;
                }

                Block newBlock(height, weight, xPos, type);
                BlockManager.AddBlock(newBlock);

                float xPosNew = (type == BlockType::Rectangle || type == BlockType::Triangle) ? xPos + 40.0f : (xPos + height + 10.0f);
                wchar_t xPosNewStr[16];
                swprintf(xPosNewStr, 16, L"%.2f", xPosNew);
                SetWindowText(GetDlgItem(hWnd, IDC_XPOS), xPosNewStr);

                SetFocus(hWnd);
                break;
            }
            case IDC_BUTTON_CLEAR:
                BlockManager.Clear();
                SetFocus(hWnd);
                break;
            case IDC_BUTTON_START:
            {
                bool tower, sort;
                HWND towerCombo = GetDlgItem(hWnd, IDC_COMBO_TOWER);
                int index = (int)SendMessage(towerCombo, CB_GETCURSEL, 0, 0);
                BlockType type;
                SortType sortType;


                switch (index) {
                case 0: type = BlockType::Rectangle; tower = true;  break;
                case 1: type = BlockType::Circle; tower = true;  break;
                case 2: type = BlockType::Square; tower = true;  break;
                case 3: type = BlockType::Triangle; tower = true;  break;
                case 4: sortType = SortType::minMax; sort = true;  break;
                case 5: sortType = SortType::maxMin; sort = true;  break;
                case 6: sortType = SortType::minMaxMin; sort = true;  break;
                default: break;
                }

                if (tower) {
                    Keys.towerType = type;
                    Keys.towerBuild = true;
                }
                else if (sort) {
                    Keys.sortType = sortType;
                    Keys.searchingForBlock = true;
                }
                SetFocus(hWnd);
            }
                break;
            case IDC_BUTTON_RECORD:
                Keys.savingData = true;
                SetFocus(hWnd);
                break;
            case IDC_BUTTON_STOP:
                Keys.savingData = false;
                SetFocus(hWnd);
                break;
            case IDC_BUTTON_PLAY:
                Keys.playData = true;
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
       
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(hdcMemory, memBitmap);

            FillRect(hdcMemory, &rect, (HBRUSH)(COLOR_WINDOW + 1));
            OnPaint(hdcMemory, MainArm, BlockManager, Keys);
            BitBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hdcMemory, 0, 0, SRCCOPY);

            SelectObject(hdcMemory, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(hdcMemory);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_HSCROLL:
    {
        HWND hScroll = (HWND)lParam;
        if (GetDlgCtrlID(hScroll) == 1) {
            int code = LOWORD(wParam);
            int pos = HIWORD(wParam);

            switch (code) {
            case SB_LINELEFT:
                velocityGlobal = max(0, velocityGlobal - 1);
                break;
            case SB_LINERIGHT:
                velocityGlobal = min(100, velocityGlobal + 1);
                break;
            case SB_PAGELEFT:
                velocityGlobal = max(0, velocityGlobal - 10);
                break;
            case SB_PAGERIGHT:
                velocityGlobal = min(100, velocityGlobal + 10);
                break;
            case SB_THUMBPOSITION:
            case SB_THUMBTRACK:
                velocityGlobal = pos;
                break;
            }

            SetScrollPos(hScroll, SB_CTL, velocityGlobal, TRUE);
            InvalidateRect(hWnd, NULL, FALSE); // odśwież rysowanie
        }
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