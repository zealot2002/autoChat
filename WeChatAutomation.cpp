#include "WeChatAutomation.h"
#include <thread>

// 静态成员变量初始化
const std::wstring WeChatAutomation::WECHAT_WINDOW_CLASS = L"WeChatMainWndForPC";
const std::wstring WeChatAutomation::WECHAT_WINDOW_NAME = L"微信";

WeChatAutomation::WeChatAutomation()
    : m_weChatWindow(NULL)
    , m_currentState(WeChatAutomationState::Idle)
    , m_stateChangeCallback(nullptr)
{
}

WeChatAutomation::~WeChatAutomation()
{
}

bool WeChatAutomation::Initialize()
{
    m_weChatWindow = NULL;
    m_currentState = WeChatAutomationState::Idle;
    return true;
}

bool WeChatAutomation::FindAndActivateWeChat(int timeoutMs)
{
    SetState(WeChatAutomationState::FindingWeChat, L"查找微信窗口...");
    
    // 尝试通过类名和窗口名查找微信窗口
    m_weChatWindow = m_appFinder.FindWindowByClass(WECHAT_WINDOW_CLASS);
    if (!m_weChatWindow)
    {
        m_weChatWindow = m_appFinder.FindWindowByName(WECHAT_WINDOW_NAME);
    }
    
    // 如果仍未找到，尝试等待微信窗口出现
    if (!m_weChatWindow)
    {
        m_weChatWindow = m_appFinder.WaitForWindow(WECHAT_WINDOW_NAME, timeoutMs);
    }
    
    if (!m_weChatWindow)
    {
        SetState(WeChatAutomationState::Failed, L"未找到微信窗口");
        return false;
    }
    
    SetState(WeChatAutomationState::ActivatingWindow, L"激活微信窗口...");
    
    // 激活窗口并居中显示
    if (!m_windowController.ActivateWindow(m_weChatWindow))
    {
        SetState(WeChatAutomationState::Failed, L"无法激活微信窗口");
        return false;
    }
    
    m_windowController.CenterWindow(m_weChatWindow);
    
    // 等待窗口完全激活
    if (!m_windowController.WaitForWindowActive(m_weChatWindow, timeoutMs))
    {
        SetState(WeChatAutomationState::Failed, L"激活微信窗口超时");
        return false;
    }
    
    // 加入短暂延迟，确保窗口完全就绪
    InputSimulator::Sleep(DEFAULT_WAIT_TIME);
    
    return true;
}

bool WeChatAutomation::SearchContact(const std::wstring& contactName, int timeoutMs)
{
    if (!m_weChatWindow)
    {
        SetState(WeChatAutomationState::Failed, L"微信窗口未初始化");
        return false;
    }
    
    SetState(WeChatAutomationState::OpeningSearch, L"打开搜索框...");
    
    // 获取窗口位置
    WindowPosition pos = m_windowController.GetWindowPosition(m_weChatWindow);
    
    // 点击搜索框
    int searchX = pos.x + SEARCH_BUTTON_X_OFFSET;
    int searchY = pos.y + SEARCH_BUTTON_Y_OFFSET;
    if (!m_inputSimulator.ClickLeftMouse(searchX, searchY))
    {
        SetState(WeChatAutomationState::Failed, L"点击搜索框失败");
        return false;
    }
    
    // 确保搜索框被点击
    InputSimulator::Sleep(DEFAULT_WAIT_TIME);
    
    SetState(WeChatAutomationState::SearchingContact, L"搜索联系人: " + contactName);
    
    // 输入联系人名称
    if (!m_inputSimulator.SendText(contactName))
    {
        SetState(WeChatAutomationState::Failed, L"输入联系人名称失败");
        return false;
    }
    
    // 等待搜索结果
    InputSimulator::Sleep(timeoutMs);
    
    return true;
}

bool WeChatAutomation::SelectFirstSearchResult()
{
    if (!m_weChatWindow)
    {
        SetState(WeChatAutomationState::Failed, L"微信窗口未初始化");
        return false;
    }
    
    SetState(WeChatAutomationState::SelectingContact, L"选择第一个联系人...");
    
    // 获取窗口位置
    WindowPosition pos = m_windowController.GetWindowPosition(m_weChatWindow);
    
    // 点击第一个搜索结果
    int resultX = pos.x + FIRST_RESULT_X_OFFSET;
    int resultY = pos.y + FIRST_RESULT_Y_OFFSET;
    if (!m_inputSimulator.ClickLeftMouse(resultX, resultY))
    {
        SetState(WeChatAutomationState::Failed, L"点击联系人失败");
        return false;
    }
    
    // 等待聊天窗口加载
    InputSimulator::Sleep(DEFAULT_WAIT_TIME);
    
    return true;
}

bool WeChatAutomation::FocusInputBox()
{
    if (!m_weChatWindow)
    {
        SetState(WeChatAutomationState::Failed, L"微信窗口未初始化");
        return false;
    }
    
    SetState(WeChatAutomationState::FocusingInputBox, L"聚焦输入框...");
    
    // 获取窗口位置
    WindowPosition pos = m_windowController.GetWindowPosition(m_weChatWindow);
    
    // 点击输入框
    int inputX = pos.x + INPUT_BOX_X_OFFSET;
    int inputY = pos.y + INPUT_BOX_Y_OFFSET;
    if (!m_inputSimulator.ClickLeftMouse(inputX, inputY))
    {
        SetState(WeChatAutomationState::Failed, L"点击输入框失败");
        return false;
    }
    
    // 等待输入框聚焦
    InputSimulator::Sleep(DEFAULT_WAIT_TIME / 2);
    
    return true;
}

bool WeChatAutomation::SendMessage(const std::wstring& message)
{
    if (!m_weChatWindow)
    {
        SetState(WeChatAutomationState::Failed, L"微信窗口未初始化");
        return false;
    }
    
    SetState(WeChatAutomationState::InputtingMessage, L"输入消息...");
    
    // 确保输入框已聚焦
    if (!FocusInputBox())
        return false;
    
    // 发送文本消息
    if (!m_inputSimulator.SendText(message))
    {
        SetState(WeChatAutomationState::Failed, L"输入消息失败");
        return false;
    }
    
    InputSimulator::Sleep(DEFAULT_WAIT_TIME / 2);
    
    SetState(WeChatAutomationState::SendingMessage, L"发送消息...");
    
    // 发送Enter键发送消息
    if (!m_inputSimulator.SendKey(VK_RETURN))
    {
        SetState(WeChatAutomationState::Failed, L"发送消息失败");
        return false;
    }
    
    // 或者，如果需要点击发送按钮
    /*
    WindowPosition pos = m_windowController.GetWindowPosition(m_weChatWindow);
    int sendX = pos.x + SEND_BUTTON_X_OFFSET;
    int sendY = pos.y + SEND_BUTTON_Y_OFFSET;
    if (!m_inputSimulator.ClickLeftMouse(sendX, sendY))
    {
        SetState(WeChatAutomationState::Failed, L"点击发送按钮失败");
        return false;
    }
    */
    
    SetState(WeChatAutomationState::Completed, L"消息发送完成");
    return true;
}

bool WeChatAutomation::ExecuteSendMessage(const std::wstring& contactName, const std::wstring& message, WeChatAutomationCallback callback)
{
    // 保存当前的回调函数
    WeChatAutomationCallback originalCallback = m_stateChangeCallback;
    
    // 设置新的回调函数
    if (callback)
    {
        m_stateChangeCallback = callback;
    }
    
    // 执行发送消息的完整流程
    bool result = FindAndActivateWeChat() &&
                  SearchContact(contactName) &&
                  SelectFirstSearchResult() &&
                  FocusInputBox() &&
                  SendMessage(message);
    
    // 恢复原来的回调函数
    m_stateChangeCallback = originalCallback;
    
    return result;
}

WeChatAutomationState WeChatAutomation::GetCurrentState() const
{
    return m_currentState;
}

void WeChatAutomation::SetStateChangeCallback(WeChatAutomationCallback callback)
{
    m_stateChangeCallback = callback;
}

void WeChatAutomation::SetState(WeChatAutomationState newState, const std::wstring& message)
{
    m_currentState = newState;
    
    // 调用回调函数（如果已设置）
    if (m_stateChangeCallback)
    {
        m_stateChangeCallback(newState, message);
    }
} 