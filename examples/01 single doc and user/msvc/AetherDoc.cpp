// Copyright 2016 Aether authors. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//   http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================

#include "pch.h"
#include "../model/model.h"
#include <windows.h>

namespace {
  // Convert a wide Unicode string to an UTF8 string
  std::string WcsToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
    std::string str_to(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), str_to.data(), size_needed, NULL, NULL);
    return str_to;
  }

  std::wstring Utf8ToWcs(const std::string& str) {
    if (str.empty()) return {};
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0);
    std::wstring wstr_to(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), wstr_to.data(), size_needed);
    return wstr_to;
  }
}

MainWindowPresenter* presenter;
HINSTANCE hInstance;
HWND hWndEdit;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_COMMAND: {
    switch (HIWORD(wParam)) {
    case EN_CHANGE: {
      auto num_symbols = SendMessage(hWndEdit, WM_GETTEXTLENGTH, 0, 0);
      std::wstring wide_string;
      wide_string.resize(num_symbols);
      SendMessage(hWndEdit, WM_GETTEXT, num_symbols * sizeof(wchar_t), (LPARAM)wide_string.data());
      presenter->OnTextChanged(0, static_cast<int>(num_symbols), WcsToUtf8(wide_string));
      break;
    }
    }
    break;
  }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

class MainWindowPresenterWin : public MainWindowPresenter {
public:
  AETHER_OBJ(MainWindowPresenterWin, MainWindowPresenter);
  template <typename T> void Serializator(T& s) { }
  bool OnEvent(const aether::Event::ptr& event) {
    switch (event->GetId()) {
    case EventTextChanged::kId: {
      // Text is already inserted by Windows so do nothing.
      return false;
    }
    default:
      return aether::Obj::OnEvent(event);
    }
  }

  void OnLoaded() {
    presenter = this;

    const wchar_t kWndClassName[] = L"AetherDocMainWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kWndClassName;
    RegisterClass(&wc);
    hWnd = CreateWindowEx(0, kWndClassName, L"Aether::Doc", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, hInstance, NULL);
    hWndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), TEXT("test"),
      WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
      10, 10, 260, 240, hWnd, NULL, hInstance, NULL);
    SendMessage(hWndEdit, WM_SETTEXT, 0, (LPARAM)Utf8ToWcs(text_->string_).c_str());
    ShowWindow(hWnd, SW_SHOW);
  }
  HWND hWnd;
};
AETHER_IMPL(MainWindowPresenterWin);


#ifdef _DEBUG
int __cdecl main() {
  hInstance = GetModuleHandle(NULL);
#else
int APIENTRY wWinMain(HINSTANCE hInstance1, HINSTANCE, LPWSTR, int) {
  hInstance = hInstance1;
#endif
  std::string subpath;
  int nArgs;
  LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
  if (szArglist && nArgs > 1) subpath = WcsToUtf8(szArglist[1]);

  App::ptr app = App::Create("state", subpath);
  MSG msg = {};
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  App::Release(std::move(app));
  return 0;
}
