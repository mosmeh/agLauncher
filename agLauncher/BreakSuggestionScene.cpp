#include "BreakSuggestionScene.h"

namespace ag {

BreakSuggestionScene::BreakSuggestionScene() : font(40) {}

void BreakSuggestionScene::init() {
    Graphics::SetBackground(Palette::White);
}

void BreakSuggestionScene::update() {}

void BreakSuggestionScene::draw() const {
    font(L"����͂����܂�!\n���̐l�Ɍ�サ�Ă�������").drawCenter(Window::Center(), Palette::Black);
}

}
