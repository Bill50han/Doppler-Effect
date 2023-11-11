// 多普勒效应演示.cpp : 定义应用程序的入口点。
//
#define _CRT_SECURE_NO_WARNINGS

#include "framework.h"
#include "多普勒效应演示.h"

#define MAX_LOADSTRING 100

typedef struct _Circle
{
    POINT p;
    ULONG64 r;
} Circle, *PCircle;

struct _Speaker
{
    POINT location;
    HBRUSH redPen;
    HBRUSH bluePen;
    HBRUSH blackPen;
    char isSpeak;
    char isChangeSpeak; //0:没,1:正在,2:完
} speaker = { {220,175}, NULL, NULL, NULL, false, false };

// 定义一些常量
#define SAMPLE_RATE 44100
#define AMPLITUDE 32760
#define PI 3.14159265358979323846

// WaveOut全局变量
HWAVEOUT hWaveOut;

#define DrawCircle(hdc, c) Arc(hdc, (c).p.x - (c).r, (c).p.y - (c).r, (c).p.x + (c).r, (c).p.y + (c).r, (c).p.x - (c).r, (c).p.y, (c).p.x - (c).r, (c).p.y)


short* buffer = NULL;
HWND hStart;
HWND hMove;

volatile bool isMoving = false;

int audioTimeID;
int videoTimeID;

// 全局变量:
HINSTANCE hInst;                                // 当前实例
HWND g_hwnd;
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

DWORD WINAPI StartPlay(_In_ LPVOID lpParameter);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    // TODO: 在此处放置代码。
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    speaker.redPen = CreateSolidBrush(RGB(255, 0, 0));
    speaker.bluePen = CreateSolidBrush(RGB(0, 0, 255));
    speaker.blackPen = CreateSolidBrush(RGB(0, 0, 0));

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
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
    //wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MY);
    wcex.lpszClassName  = szWindowClass;
    //wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   g_hwnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 1280, 728, nullptr, nullptr, hInstance, nullptr);

   if (!g_hwnd)
   {
      return FALSE;
   }

   ShowWindow(g_hwnd, nCmdShow);
   UpdateWindow(g_hwnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        //创建按钮
        hStart = CreateWindowW(L"Button", L"开始", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            100, 550, 80, 45, hWnd, (HMENU)IDC_START, hInst, NULL);
        hMove = CreateWindowW(L"Button", L"移动", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            250, 550, 80, 45, hWnd, (HMENU)IDC_MOVE, hInst, NULL);
    }
    return 0;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDC_START:
                SetWindowLongW(hMove, GWL_STYLE, GetWindowLongW(hMove, GWL_STYLE) & ~WS_DISABLED);
                SetWindowPos(hMove, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                SetWindowLongW(hStart, GWL_STYLE, GetWindowLongW(hStart, GWL_STYLE) | WS_DISABLED);
                SetWindowPos(hStart, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

                CreateThread(NULL, 0, StartPlay, hWnd, 0, NULL);
                break;
            case IDC_MOVE:
                isMoving = true;
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    /*case WM_MOUSEMOVE:
    {
        POINT p = { 0 };
        GetCursorPos(&p);
        ScreenToClient(hWnd, &p);
        printf("x: %d , y: %d\n", p.x, p.y);

        break;
    }*/
    case MM_WOM_DONE:
    {
        waveOutRestart();
    }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            SelectObject(hdc, speaker.redPen);
            Ellipse(hdc, 210, 165, 230, 185);
            SelectObject(hdc, speaker.bluePen);
            Ellipse(hdc, 1030, 165, 1050, 185);
            

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        //timeKillEvent(audioTimeID);
        timeKillEvent(videoTimeID);
        timeEndPeriod(1);

        g_pDSBuffer?g_pDSBuffer->Release():0;
        g_pDS?g_pDS->Release():0;
        free(buffer);
        PostQuitMessage(0);
        break;
    case WM_QUIT:
        //waveOutSetVolume(NULL, g_vol);
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
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

DSBUFFERDESC bufferDesc;
void PlayEveryTime(
    UINT      uTimerID,
    UINT      uMsg,
    DWORD_PTR dwUser,
    DWORD_PTR dw1,
    DWORD_PTR dw2
);
void DrawMoveCircle(
    UINT      uTimerID,
    UINT      uMsg,
    DWORD_PTR dwUser,
    DWORD_PTR dw1,
    DWORD_PTR dw2
);

DWORD WINAPI StartPlay(
    _In_ LPVOID lpParameter
)
{
    int duration = 900;           // 持续时间（以毫秒为单位）
    int sampleRate = 44100;     // 采样率（每秒样本数）
    int numSamples = (duration * sampleRate) / 1000;
    double frequencyStart = 200;     // 起始频率（Hz）
    double frequencyEnd = 600;       // 结束频率（Hz）

    // 计算每个样本的频率增量
    double frequencyIncrement = (frequencyEnd - frequencyStart) / numSamples;
    double currentFrequency = frequencyStart;

    // 分配缓冲区存储样本
    buffer = (short*)malloc(numSamples * sizeof(short));

    // 生成正弦波样本
    for (int i = 0; i < numSamples; i++)
    {
        double time = (double)i / sampleRate;
        double value = sin(2 * PI * currentFrequency * time);
        buffer[i] = (short)(value * SHRT_MAX);

        // 更新当前频率
        currentFrequency += frequencyIncrement;
    }

    // 播放声音
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = sampleRate;
    wfx.wBitsPerSample = sizeof(short) * 8;
    wfx.nBlockAlign = (wfx.wBitsPerSample / 8) * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)g_hwnd, 0, CALLBACK_WINDOW) != MMSYSERR_NOERROR)
    {
        fprintf(stderr, "无法打开音频设备\n");
        return 1;
    }

    // 写入声音数据
    MMRESULT t;
    WAVEHDR whdr = {
        (LPSTR)buffer,
        numSamples * sizeof(short),
        0,0,0,0,0,0
    };
    if (t = waveOutPrepareHeader(hWaveOut, (WAVEHDR*)&whdr, sizeof(WAVEHDR)))
    {
        fprintf(stderr, "无法准备音频数据\n");
        return 1;
    }
    if (waveOutWrite(hWaveOut, (WAVEHDR*)&whdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
    {
        fprintf(stderr, "无法播放音频数据\n");
        return 1;
    }
    Sleep(duration);

    // 清理资源
    waveOutUnprepareHeader(hWaveOut, (WAVEHDR*)buffer, sizeof(WAVEHDR));
    waveOutClose(hWaveOut);

    timeBeginPeriod(1);
    videoTimeID = timeSetEvent(20, 0, DrawMoveCircle, 0, TIME_PERIODIC);

    return 0;
}

/*
void PlayEveryTime(
    UINT      uTimerID,
    UINT      uMsg,
    DWORD_PTR dwUser,
    DWORD_PTR dw1,
    DWORD_PTR dw2
)
{
    //g_pDSBuffer?g_pDSBuffer->Stop():0;
    /*
    if (speaker.isChangeSpeak == 1)
    {
        frequency += 1;
        //frequency = 450;
    }

    if (speaker.isSpeak == 2 && frequency > 0)
    {
        frequency--;
    }

    GenerateSineWave(frequency, duration, buffer);

    // 写入缓冲区
    void* pDSLockedBuffer = NULL;
    DWORD dwDSLockedBufferSize = 0;
    if (FAILED(g_pDSBuffer->Lock(0, bufferDesc.dwBufferBytes, &pDSLockedBuffer, &dwDSLockedBufferSize, NULL, NULL, 0))) {
        printf("Failed to lock DirectSound buffer\n");
        return;
    }
    memcpy(pDSLockedBuffer, buffer, duration * SAMPLE_RATE / 1000 * sizeof(short));
    g_pDSBuffer->Unlock(pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0);

    // 播放缓冲区
    if (FAILED(g_pDSBuffer->SetCurrentPosition(0))) {
        printf("Failed to set DirectSound buffer position\n");
        return;
    }
    HRESULT r2 = g_pDSBuffer->SetVolume(DSBVOLUME_MAX);
    LONG vol = -1;
    HRESULT r = g_pDSBuffer->GetVolume(&vol);
    DWORD vol2 = 0xFFFF;
    waveOutGetVolume(NULL, &vol2);
    printf("%d %u %X %X\n", vol, vol2, r, r2);
    if (FAILED(g_pDSBuffer->Play(0, 0, DSBPLAY_LOOPING))) {
        printf("Failed to play 
        buffer\n");
        return;
    }
}
*/
void DrawMoveCircle(
    UINT      uTimerID,
    UINT      uMsg,
    DWORD_PTR dwUser,
    DWORD_PTR dw1,
    DWORD_PTR dw2
)
{
    static ULONG64 g_i = 0;
    //g_i %= 50;

    static int circleNum = 0;
    static PCircle c[11] = { 0 };

    static int circleStartMoving = 9999; //[1,10]
    static ULONG64 circleStartMoving_i = 0;  //[0, ULONG64]

    HDC rhdc = GetDC(g_hwnd);
    static HDC chdc = CreateCompatibleDC(rhdc);
    SelectObject(chdc, CreateCompatibleBitmap(rhdc, 1280, 728));
    RECT bg = { 0,0,1280,728 };
    FillRect(chdc, &bg, (HBRUSH)(COLOR_WINDOW + 1));


    /*static Circle c1 = {{220, 175}, 10};
    DrawCircle(chdc, c1);
    c1.r += 10;*/

    if (g_i%50 == 0)
    {
        c[circleNum] = (PCircle)malloc(sizeof(Circle));
        c[circleNum]->p.x = speaker.location.x;
        c[circleNum]->p.y = speaker.location.y;
        c[circleNum]->r = 0;

        circleNum++;
    }

    if (isMoving)
    {
        speaker.location.x += 1;

        if (circleStartMoving == 9999)
        {
            circleStartMoving = circleNum != 0 ? circleNum - 1 : 10;
            //circleStartMoving = circleNum;//!= 10 ? circleNum + 1 : 0;
        }
        else
        {
            if (c[circleStartMoving]->r + c[circleStartMoving]->p.x >= 1120 && 0 == speaker.isChangeSpeak)
            {
                speaker.isChangeSpeak = 1;
                circleStartMoving_i = g_i;
            }
        }
    }

    if (speaker.location.x >= 870)
    {
        isMoving = false;
        //timeKillEvent(audioTimeID);
        //frequency = 0;
        //duration = 0;
        //g_pDSBuffer->Stop();
        //g_pDSBuffer->SetVolume(0);
        speaker.isSpeak = 2;
        //PlayEveryTime(0, 0, 0, 0, 0);
        //waveOutSetVolume(NULL, 0);

        //timeKillEvent(videoTimeID);
        g_i = 0;
    }

    //printf("i:%d\n", (int)g_i);

    if (speaker.isChangeSpeak && g_i - circleStartMoving_i >= 20)
    {
        speaker.isChangeSpeak = 3;
        //frequency = 450;
    }


    //约0.9s：频率变化时间

    for (size_t i = 0; i < 11; i++)
    {
        if (c[i] != 0)
        {
            DrawCircle(chdc, *c[i]);
            c[i]->r += 2;
            
            /*if (c[i]->r + c[i]->p.x >= 1040 && speaker.isSpeak == 0)
            {
                audioTimeID = timeSetEvent(duration, 0, PlayEveryTime, 0, TIME_PERIODIC);
                //speaker.isSpeak = 1;
            }*/
        }
    }
    if (circleNum == 11)
    {
        free(c[0]);
        circleNum = 0;
    }

    /*
    2.8s圈离开屏幕，创建一个队列包含2.8s内能在屏幕内出现的最大圈圈数量（10圈顶多）
    */

    

    SelectObject(chdc, speaker.redPen);
    Ellipse(chdc, speaker.location.x - 10, speaker.location.y - 10,
        speaker.location.x + 10, speaker.location.y + 10);
    SelectObject(chdc, speaker.bluePen);
    Ellipse(chdc, 1030, 165, 1050, 185);
    BitBlt(rhdc, 0, 0, 1280, 550, chdc, 0, 0, SRCCOPY);

    g_i++;
    return;
}
