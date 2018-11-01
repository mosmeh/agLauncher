#include "BreakSuggestionScene.h"

namespace ag {

BreakSuggestionScene::BreakSuggestionScene() : font(40) {}

void BreakSuggestionScene::init() {
    Graphics::SetBackground(Palette::White);
}

void BreakSuggestionScene::update() {}

void BreakSuggestionScene::draw() const {
    font(L"¡‰ñ‚Í‚±‚±‚Ü‚Å!\nŸ‚Ìl‚ÉŒğ‘ã‚µ‚Ä‚­‚¾‚³‚¢").drawCenter(Window::Center(), Palette::Black);
}

}
