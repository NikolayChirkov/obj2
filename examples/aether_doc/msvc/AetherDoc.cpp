#include "pch.h"
#include "../model/model.h"
#include <windows.h>
HINSTANCE hInstance;

class MainPresenterWin : public MainPresenter {
public:
  AETHER_OBJ(MainPresenterWin, MainPresenter);
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
  HWND hWndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), TEXT("test"), WS_CHILD | WS_VISIBLE | WS_VSCROLL |
    ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, 10, 10, 140, 60, hWnd, NULL, NULL, NULL);
  const wchar_t* tt = L"lpszLatin";
  SendMessage(hWndEdit, WM_SETTEXT, 0, (LPARAM)tt);

  presenters_[hWnd] = this;
  ShowWindow(hWnd, SW_SHOW);
}

bool MainPresenterWin::OnEvent(const aether::Event::ptr& event) {
  switch (event->GetId()) {
  case EventPos::kId:
    SetWindowPos(hWnd, NULL, EventPos::ptr(event)->x_, EventPos::ptr(event)->y_, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
    return false;
  default:
    return aether::Obj::OnEvent(event);
  }
}
AETHER_IMPL(MainPresenterWin);

class TextPresenterWin : public TextPresenter {
public:
  AETHER_OBJ(TextPresenterWin, TextPresenter);
  template <typename T> void Serializator(T& s, int flags) {}
  virtual bool OnEvent(const aether::Event::ptr& event);
  virtual void OnLoaded();
//  NSTextView* text_view_;
};
AETHER_IMPL(TextPresenterWin);

void TextPresenterWin::OnLoaded() {
  //NSString *s = [NSString stringWithCString : text_->string_.c_str() encoding : [NSString defaultCStringEncoding]];
  //[text_view_ setString : s];
}

bool TextPresenterWin::OnEvent(const aether::Event::ptr& event) {
  switch (event->GetId()) {
  case EventTextChanged::kId: {
    //EventTextChanged::ptr e(event);
    //[text_view_ setSelectedRange : NSMakeRange(e->cursor_pos_, e->num_symbols_)];
    //      NSAttributedString* s = [[NSAttributedString alloc] initWithHTML:[@"<font color=#FF0000>text</font>" dataUsingEncoding:NSUTF8StringEncoding]documentAttributes:NULL];
    //NSString *s = [NSString stringWithCString : e->inserted_text_.c_str() encoding : [NSString defaultCStringEncoding]];
    //[text_view_ setString : s];
    //[textView insertText:s];
    return false;
  }
  default:
    return aether::Obj::OnEvent(event);
  }
}


#ifdef _DEBUG
int __cdecl main() {
  hInstance = GetModuleHandle(NULL);
#else
int APIENTRY wWinMain(HINSTANCE hInstance1, HINSTANCE, LPWSTR, int) {
  hInstance = hInstance1;
#endif
  App::ptr app = App::Create("state");
  MSG msg = {};
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  App::Release(std::move(app));
  return 0;
}
