// autoChat.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "autoChat.h"
#include "WeChatAutomation.h"
#include <string>
#include <windowsx.h>
#include <CommCtrl.h>

#pragma comment(lib, "Comctl32.lib")

#define MAX_LOADSTRING 100

// 自定义消息
#define WM_AUTOMATION_STATE_CHANGE (WM_USER + 1)

// 控件ID
#define IDC_STATIC_CONTACT_LABEL 1001
#define IDC_EDIT_CONTACT 1002
#define IDC_STATIC_MESSAGE_LABEL 1003
#define IDC_EDIT_MESSAGE 1004
#define IDC_BUTTON_SEND 1005
#define IDC_STATUS_BAR 1006
#define IDC_PROGRESS_BAR 1007

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 自动化操作相关
WeChatAutomation* g_pAutomation = nullptr;        // 微信自动化对象

// 控件句柄
HWND g_hEditContact = NULL;                     // 联系人输入框
HWND g_hEditMessage = NULL;                     // 消息输入框
HWND g_hButtonSend = NULL;                      // 发送按钮
HWND g_hStatusBar = NULL;                       // 状态栏
HWND g_hProgressBar = NULL;                     // 进度条

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// 创建窗口控件
void CreateControls(HWND hWnd);

// 初始化自动化对象
bool InitializeAutomation();

// 清理资源
void CleanupResources();

// 自动化状态变更回调
void CALLBACK OnAutomationStateChange(WeChatAutomationState state, const std::wstring& message);

// 开始发送消息
void StartSendMessage();

// 更新UI
void UpdateUserInterface(HWND hWnd, WeChatAutomationState state, const std::wstring& message);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 初始化公共控件
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_AUTOCHAT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AUTOCHAT));

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

    // 清理资源
    CleanupResources();

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AUTOCHAT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_AUTOCHAT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 500, 400, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // 初始化自动化操作
   if (!InitializeAutomation())
   {
       MessageBox(hWnd, L"初始化自动化功能失败", L"错误", MB_OK | MB_ICONERROR);
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

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
        // 创建控件
        CreateControls(hWnd);
        break;

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
            case IDC_BUTTON_SEND:
                // 开始发送消息
                StartSendMessage();
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_AUTOMATION_STATE_CHANGE:
        {
            // 处理自动化状态变更消息
            WeChatAutomationState state = (WeChatAutomationState)wParam;
            const wchar_t* message = (const wchar_t*)lParam;
            UpdateUserInterface(hWnd, state, message ? message : L"");
            
            // 释放由OnAutomationStateChange中分配的消息内存
            if (message)
                LocalFree((HLOCAL)message);
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_SIZE:
        // 调整状态栏大小
        if (g_hStatusBar)
            SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// "关于"框的消息处理程序。
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

// 创建窗口控件
void CreateControls(HWND hWnd)
{
    // 获取窗口客户区大小
    RECT rect;
    GetClientRect(hWnd, &rect);
    int width = rect.right - rect.left;
    
    // 创建联系人标签
    CreateWindowW(L"STATIC", L"联系人:", WS_VISIBLE | WS_CHILD | SS_LEFT,
        20, 20, 80, 20, hWnd, (HMENU)IDC_STATIC_CONTACT_LABEL, hInst, NULL);

    // 创建联系人输入框
    g_hEditContact = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        100, 20, width - 120, 20, hWnd, (HMENU)IDC_EDIT_CONTACT, hInst, NULL);

    // 创建消息标签
    CreateWindowW(L"STATIC", L"消息内容:", WS_VISIBLE | WS_CHILD | SS_LEFT,
        20, 50, 80, 20, hWnd, (HMENU)IDC_STATIC_MESSAGE_LABEL, hInst, NULL);

    // 创建消息输入框（多行）
    g_hEditMessage = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL,
        20, 80, width - 40, 180, hWnd, (HMENU)IDC_EDIT_MESSAGE, hInst, NULL);

    // 创建发送按钮
    g_hButtonSend = CreateWindowW(L"BUTTON", L"发送", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        width / 2 - 40, 270, 80, 30, hWnd, (HMENU)IDC_BUTTON_SEND, hInst, NULL);

    // 创建进度条
    g_hProgressBar = CreateWindowW(PROGRESS_CLASS, NULL,
        WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
        20, 310, width - 40, 20, hWnd, (HMENU)IDC_PROGRESS_BAR, hInst, NULL);
    SendMessage(g_hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessage(g_hProgressBar, PBM_SETSTEP, 1, 0);
    SendMessage(g_hProgressBar, PBM_SETPOS, 0, 0);

    // 创建状态栏
    g_hStatusBar = CreateWindowW(STATUSCLASSNAME, L"就绪",
        WS_VISIBLE | WS_CHILD | SBARS_SIZEGRIP,
        0, 0, 0, 0, hWnd, (HMENU)IDC_STATUS_BAR, hInst, NULL);
}

// 初始化自动化对象
bool InitializeAutomation()
{
    // 创建自动化对象
    if (!g_pAutomation)
    {
        g_pAutomation = new WeChatAutomation();
        if (!g_pAutomation)
            return false;
    }
    
    // 初始化自动化对象
    bool result = g_pAutomation->Initialize();
    
    // 设置状态变更回调
    g_pAutomation->SetStateChangeCallback(OnAutomationStateChange);
    
    return result;
}

// 清理资源
void CleanupResources()
{
    // 释放自动化对象
    if (g_pAutomation)
    {
        delete g_pAutomation;
        g_pAutomation = nullptr;
    }
}

// 自动化状态变更回调
void CALLBACK OnAutomationStateChange(WeChatAutomationState state, const std::wstring& message)
{
    // 发送自定义消息到主窗口
    // 注意：需要复制消息字符串，因为消息可能在线程中传递
    wchar_t* pMessage = NULL;
    if (!message.empty())
    {
        pMessage = (wchar_t*)LocalAlloc(LMEM_FIXED, (message.length() + 1) * sizeof(wchar_t));
        if (pMessage)
            wcscpy_s(pMessage, message.length() + 1, message.c_str());
    }
    
    // 发送状态变更消息
    HWND hWnd = FindWindowW(szWindowClass, NULL);
    if (hWnd)
    {
        PostMessage(hWnd, WM_AUTOMATION_STATE_CHANGE, (WPARAM)state, (LPARAM)pMessage);
    }
    else if (pMessage)
    {
        // 如果无法找到窗口，释放分配的内存
        LocalFree(pMessage);
    }
}

// 开始发送消息
void StartSendMessage()
{
    if (!g_pAutomation)
        return;
    
    // 获取联系人和消息
    wchar_t contactBuffer[256] = { 0 };
    GetWindowTextW(g_hEditContact, contactBuffer, 256);
    std::wstring contact = contactBuffer;
    
    wchar_t messageBuffer[4096] = { 0 };
    GetWindowTextW(g_hEditMessage, messageBuffer, 4096);
    std::wstring message = messageBuffer;
    
    // 检查输入是否为空
    if (contact.empty())
    {
        MessageBox(NULL, L"请输入联系人", L"提示", MB_OK | MB_ICONINFORMATION);
        return;
    }
    
    if (message.empty())
    {
        MessageBox(NULL, L"请输入消息内容", L"提示", MB_OK | MB_ICONINFORMATION);
        return;
    }
    
    // 禁用发送按钮，防止重复点击
    EnableWindow(g_hButtonSend, FALSE);
    
    // 重置进度条
    SendMessage(g_hProgressBar, PBM_SETPOS, 0, 0);
    
    // 在新线程中执行自动化操作
    std::thread([contact, message]() {
        if (g_pAutomation)
        {
            g_pAutomation->ExecuteSendMessage(contact, message);
            
            // 操作完成后，重新启用发送按钮
            HWND hWnd = FindWindowW(szWindowClass, NULL);
            if (hWnd && g_hButtonSend)
            {
                EnableWindow(g_hButtonSend, TRUE);
            }
        }
    }).detach();  // 分离线程，由系统自动回收资源
}

// 更新UI
void UpdateUserInterface(HWND hWnd, WeChatAutomationState state, const std::wstring& message)
{
    // 更新状态栏
    if (g_hStatusBar && !message.empty())
    {
        SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)message.c_str());
    }
    
    // 根据状态更新进度条
    if (g_hProgressBar)
    {
        int progress = 0;
        switch (state)
        {
        case WeChatAutomationState::Idle:
            progress = 0;
            break;
        case WeChatAutomationState::FindingWeChat:
            progress = 10;
            break;
        case WeChatAutomationState::ActivatingWindow:
            progress = 20;
            break;
        case WeChatAutomationState::OpeningSearch:
            progress = 30;
            break;
        case WeChatAutomationState::SearchingContact:
            progress = 40;
            break;
        case WeChatAutomationState::SelectingContact:
            progress = 60;
            break;
        case WeChatAutomationState::FocusingInputBox:
            progress = 70;
            break;
        case WeChatAutomationState::InputtingMessage:
            progress = 80;
            break;
        case WeChatAutomationState::SendingMessage:
            progress = 90;
            break;
        case WeChatAutomationState::Completed:
            progress = 100;
            break;
        case WeChatAutomationState::Failed:
            progress = 0;
            break;
        }
        
        SendMessage(g_hProgressBar, PBM_SETPOS, progress, 0);
    }
    
    // 如果操作完成或失败，启用发送按钮
    if (state == WeChatAutomationState::Completed || state == WeChatAutomationState::Failed)
    {
        if (g_hButtonSend)
        {
            EnableWindow(g_hButtonSend, TRUE);
        }
        
        // 显示操作结果消息框
        if (state == WeChatAutomationState::Completed)
        {
            MessageBox(hWnd, L"消息发送成功！", L"成功", MB_OK | MB_ICONINFORMATION);
        }
        else if (state == WeChatAutomationState::Failed && !message.empty())
        {
            std::wstring errorMsg = L"操作失败: " + message;
            MessageBox(hWnd, errorMsg.c_str(), L"错误", MB_OK | MB_ICONERROR);
        }
    }
}
