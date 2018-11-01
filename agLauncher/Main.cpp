#include "Common.h"
#include "LauncherScene.h"
#include "BreakSuggestionScene.h"

HANDLE hHandle = NULL;

void Main() {
    using namespace ag;

    hHandle = CreateMutex(NULL, TRUE, L"agLauncherMultipleInstancePreventionMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return;
    }
    std::atexit([] {
        if (hHandle) {
            ReleaseMutex(hHandle);
            CloseHandle(hHandle);
        }
    });

    SetProcessDPIAware();

    System::SetExitEvent(WindowEvent::CloseButton);
    Window::SetTitle(L"agLauncher");
    Window::SetStyle(WindowStyle::Sizeable);
    Graphics::SetBackground(Palette::Black);
    ScalableWindow::Setup(width, height);
    Window::Maximize();

    App manager;
    manager.add<LauncherScene>(SceneKey::Launcher);
    manager.add<BreakSuggestionScene>(SceneKey::BreakSuggestion);
    manager.changeScene(SceneKey::Launcher, 0, false);

    while (System::Update()) {
        if (!manager.updateAndDraw()) {
            break;
        }
    }
}
