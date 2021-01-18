#include "pch.h"
#include "../model/model.h"
#include <windows.h>
HINSTANCE hInstance;

class MainPresenterWin : public MainPresenter {
public:
  AETHER_OBJECT(MainPresenterWin);
  AETHER_SERIALIZE(MainPresenterWin, MainPresenter);
  AETHER_INTERFACES(MainPresenterWin, MainPresenter);
  template <typename T> void Serializator(T& s, int flags) { }
  virtual bool OnEvent(const aether::Event::ptr& event);
  virtual void OnLoaded();
  HWND hWnd;
};
static std::map<HWND, MainPresenterWin*> presenters_;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_EXITSIZEMOVE:
    RECT rc;
    GetWindowRect(hwnd, &rc);
    presenters_[hwnd]->OnMove(rc.left, rc.top);
    return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void MainPresenterWin::OnLoaded() {
  const wchar_t kWndClassName[] = L"AetherDocMainWindowClass";
  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = kWndClassName;
  RegisterClass(&wc);
  hWnd = CreateWindowEx(0, kWndClassName, L"Aether::Doc", WS_OVERLAPPEDWINDOW,
    main_->x_, main_->y_, main_->w_, main_->h_, NULL, NULL, hInstance, NULL);
  if (hWnd == NULL) return;
  presenters_[hWnd] = this;
  ShowWindow(hWnd, SW_SHOW);
}

bool MainPresenterWin::OnEvent(const aether::Event::ptr& event) {
  switch (event->GetClassId()) {
  case EventPos::class_id_:
    SetWindowPos(hWnd, NULL, EventPos::ptr(event)->x_, EventPos::ptr(event)->y_, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
    return false;
  default:
    return aether::Obj::OnEvent(event);
  }
}
AETHER_IMPLEMENTATION(MainPresenterWin);

#ifdef _DEBUG
int __cdecl main() {
  hInstance = GetModuleHandle(NULL);
#else
int APIENTRY wWinMain(HINSTANCE hInstance1, HINSTANCE, LPWSTR, int) {
  hInstance = hInstance1;
#endif
  App::ptr app = App::Create();
  MSG msg = {};
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  App::Release(std::move(app));
  return 0;
}
