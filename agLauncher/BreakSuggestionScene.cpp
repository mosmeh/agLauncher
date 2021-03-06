#include "BreakSuggestionScene.h"

namespace ag {

BreakSuggestionScene::BreakSuggestionScene() : font(40) {}

void BreakSuggestionScene::init() {
    Graphics::SetBackground(Palette::White);
}

void BreakSuggestionScene::update() {}

void BreakSuggestionScene::draw() const {
    font(L"今回はここまで!\n次の人に交代してください").drawCenter(Window::Center(), Palette::Black);
}

}
