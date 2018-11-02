#include "LauncherScene.h"

namespace ag {

LauncherScene::LauncherScene() :
    font(34),
    background(L"/200"),
    selectedGameIndex(0),
    inDemoMode(true) {}

void LauncherScene::init() {
    constexpr auto GAMES_JSON = L"games/games.json";
    if (!FileSystem::Exists(GAMES_JSON)) {
#undef MessageBox
        MessageBox::Show(L"games.json‚ªŒ©‚Â‚©‚è‚Ü‚¹‚ñ");
    }

    JSONReader reader(GAMES_JSON);
    for (const auto& value : reader.root().getArray()) {
        games.emplace_back(value);
    }

    selectedGameIndex = WrappedIndex(games.size());
    lastActiveTime = Time::GetMillisec();
}

void LauncherScene::update() {
    const auto transformer = ScalableWindow::CreateTransformer();

    auto& selectedGame = games.at(selectedGameIndex.get());
    const bool processRunning = selectedGame.process && selectedGame.process->isRunning();
    if (processRunning) {
        Window::Minimize();
    } else {
        Window::Maximize();
    }

    if (userIsActive() || processRunning) {
        lastActiveTime = Time::GetMillisec();
        if (inDemoMode) {
            effect.clear();
            inDemoMode = false;
        }
    } else if (!inDemoMode && Time::GetMillisec() > lastActiveTime + 30 * 1000) {
        inDemoMode = true;
    }

    Region mouseRegion;
    const auto mouseX = Mouse::Pos().x;
    if (mouseX < 0.345 * width) {
        mouseRegion = Region::Left;
    } else if (mouseX <= 0.66 * width) {
        mouseRegion = Region::Center;
    } else {
        mouseRegion = Region::Right;
    }

    const auto gp = getGamepad();

    if (!processRunning) {
        if (Window::Focused()) {
            if (!effect.hasEffects()) {
                int delta = 0;
                static const auto delay = 500;
                if (Input::KeyLeft.clicked || Input::KeyLeft.pressedDuration > delay) {
                    --delta;
                }
                if (Input::KeyRight.clicked || Input::KeyRight.pressedDuration > delay) {
                    ++delta;
                }
                if (Input::MouseL.clicked) {
                    switch (mouseRegion) {
                    case Region::Left:
                        --delta;
                        break;
                    case Region::Right:
                        ++delta;
                        break;
                    }
                }

                if (gp.has_value()) {
                    if (gp->povLeft.clicked || gp->povLeft.pressedDuration > delay) {
                        --delta;
                    }
                    if (gp->povRight.clicked || gp->povRight.pressedDuration > delay) {
                        ++delta;
                    }
                }

                if (delta != 0) {
                    delta = std::max(-1, std::min(1, delta));
                    if (delta == 1) {
                        for (int i = -1; i < 3; ++i) {
                            effect.add<DemoAnimation>(games.at((selectedGameIndex + i).get()), i, i - 1);
                        }
                    } else if (delta == -1) {
                        for (int i = -2; i < 2; ++i) {
                            effect.add<DemoAnimation>(games.at((selectedGameIndex + i).get()), i, i + 1);
                        }
                    } else {
                        assert(false);
                    }
                    selectedGameIndex += delta;
                }
            }
        }

        if (stopwatch.min() >= 30) {
            changeScene(SceneKey::BreakSuggestion);
            return;
        }

        if (!effect.hasEffects()) {
            if ((Input::KeyEnter | Input::KeySpace).released ||
                (Input::MouseL.released && mouseRegion == Region::Center) ||
                (gp.has_value() && (gp->button(0) | gp->button(1) | gp->button(2) | gp->button(3)).released)) {
                games.at(selectedGameIndex.get()).launch();
                stopwatch.start();
            }
        }
    }

    background.resize(width, height).draw();

    // selected game
    const s3d::RoundRect selectedItemRect({ 0.345 * width, 0.225 * height }, { 0.31 * width, 0.4 * height }, 0.00385 * width);
    selectedItemRect.draw();
    selectedItemRect.drawFrame(0.0, 0.002 * width, Color(255, 119, 5));

    if (!effect.hasEffects()) {
        if (inDemoMode) {
            for (int i = -1; i < 3; ++i) {
                effect.add<DemoAnimation>(games.at((selectedGameIndex + i).get()), i, i - 1, 8.0);
            }
            selectedGameIndex += 1;
        } else {
            selectedGame.draw(0.5, mouseRegion == Region::Center ? 0.8 : 1.0);

            for (int delta : {-1, 1}) {
                games.at((selectedGameIndex + delta).get()).draw(0.5 + 0.3 * delta, 0.6);
            }
        }
    }
    effect.update();

    // side - background
    const Rect leftBar({ 0, 0.169 * height }, { 0.0495 * width, 0.532 * height });
    const Rect rightBar({ 0.951 * width, 0.169 * height }, { 0.0495 * width, 0.532 * height });
    leftBar.draw(AlphaF(mouseRegion == Region::Left ? 0.9 : 0.6));
    rightBar.draw(AlphaF(mouseRegion == Region::Right ? 0.9 : 0.6));

    // side - triangle
    const auto leftTriangle = Triangle({ 0.022 * width, 0.419 * height }, 0.04 * width).rotated(-Math::HalfPi);
    const auto rightTriangle = Triangle({ 0.978 * width, 0.419 * height }, 0.04 * width).rotated(Math::HalfPi);
    leftTriangle.draw(textColor);
    rightTriangle.draw(textColor);

    // description - background
    Rect({ 0, 0.768 * height }, { width, 0.231 * height }).draw(AlphaF(0.9));

    // description - line
    Line({ 0, 0.768 * height }, { width, 0.768 * height }).draw(0.00488 * height, Color(0, 162, 154));

    // description - text
    const auto descLines = selectedGame.desc.split(L'\n');
    double p = (0.77 + (1.0 - 0.77) / (descLines.size() + 1)) * height;
    for (const auto& line : descLines) {
        font.drawCenter(line, { 0.5 * width, p }, Palette::Black);
        p += font.region(line).h;
    }

    font(selectedGameIndex.get() + 1, L"/", games.size()).drawCenter({ 0.9 * width, 0.7 * height }, Palette::Black);

    font(Pad(stopwatch.min(), { 2, L'0' }), L":", Pad(stopwatch.s() % 60, { 2, L'0' })).drawCenter({ 0.1 * width, 0.1 * height }, Palette::Black);
}

void LauncherScene::draw() const {}

}

