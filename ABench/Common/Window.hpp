#pragma once


namespace ABench {
namespace Common {

class Window
{
#ifdef WIN32
    HINSTANCE mInstance;
    HWND mHWND;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#elif defined(__linux__) | defined(__LINUX__)

#else
#error "Target platform not supported"
#endif
    int mWidth;
    int mHeight;
    bool mOpened;
    std::wstring mClassName;

protected:
    bool mKeys[255];
    bool mMouseButtons[3];

    // callbacks
    virtual void OnInit();
    virtual void OnOpen();
    virtual void OnClose();
    virtual void OnKeyDown(int key);
    virtual void OnKeyUp(int key);
    virtual void OnUpdate(float deltaTime);
    virtual void OnMouseDown(int key);
    virtual void OnMouseMove(int deltaX, int deltaY);
    virtual void OnMouseUp(int key);

public:
    Window();
    ~Window();

    bool Init();
    bool Open(int x, int y, int width, int height, const std::string& title);
    bool SetTitle(const std::wstring& title);
    bool SetTitle(const std::string& title);
    void Update(float deltaTime);
    void Close();

#ifdef WIN32
    ABENCH_INLINE HINSTANCE GetInstance() const
    {
        return mInstance;
    }

    ABENCH_INLINE HWND GetHandle() const
    {
        return mHWND;
    }
#endif

    ABENCH_INLINE uint32_t GetWidth() const
    {
        return mWidth;
    }

    ABENCH_INLINE uint32_t GetHeight() const
    {
        return mHeight;
    }

    ABENCH_INLINE bool IsOpen() const
    {
        return mOpened;
    }
};

} // namespace Common
} // namespace ABench
