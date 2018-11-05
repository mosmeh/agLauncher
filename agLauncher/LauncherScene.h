#pragma once

#include "Common.h"

namespace ag {

static const Color textColor(215, 58, 89);

struct Game {
    String title, desc;
    FilePath exec, browserPath;
    Texture thumb;
    Optional<ProcessInfo> process;

    Game(const JSONValue& jsonValue) :
        title(jsonValue[L"title"].getString()),
        desc(jsonValue[L"desc"].getString()),
        thumb(Texture(jsonValue[L"thumb"].getString())),
        exec(jsonValue[L"exec"].getString()) {}

    void launch() {
#undef CreateProcess
        process = System::CreateProcess(exec);
    }

    void draw(double relativeXPos, double alpha = 1.0) const {
        const auto size = Window::BaseSize();
        const auto scale = 0.237 * size.y / thumb.size.y;
        thumb.scale(scale).drawAt({ relativeXPos * size.x, 0.451 * size.y }, AlphaF(alpha));

        static const Font font(34);
        font.drawCenter(title, { relativeXPos * size.x, 0.293 * size.y }, textColor);
    }
};

enum class Region {
    Left, Center, Right
};

class DemoAnimation : public IEffect {
public:
    DemoAnimation(const Game& game, int startIndex, int endIndex, double duration = 0.1) :
        game_(game),
        startIndex_(startIndex),
        start_(relativeIndexToXPos(startIndex)),
        end_(relativeIndexToXPos(endIndex)),
        duration_(duration) {}

    bool update(double t) override {
        static const Font font(34);
        const auto xPos = start_ + (end_ - start_) * t / duration_;
        game_.draw(xPos, startIndex_ == 1 ? 1.0 : 0.6);

        return t <= duration_;
    }

private:
    const Game& game_;
    int startIndex_;
    double start_, end_;
    const double duration_;

    static double relativeIndexToXPos(int relIndex) {
        const auto imgWidth = 0.237;
        if (relIndex <= -2) {
            return -imgWidth / 2;
        } else if (relIndex == -1) {
            return 0.2;
        } else if (relIndex == 0) {
            return 0.5;
        } else if (relIndex == 1) {
            return 0.8;
        } else {
            return 1.0 + imgWidth / 2;
        }
    }
};

class WrappedIndex {
public:
    WrappedIndex(size_t size) : size_(size) {}

    size_t get() const {
        return index_;
    }

    void operator+=(int delta) {
        if (delta >= 0) {
            index_ = (index_ + delta) % size_;
        } else {
            index_ = size_ - 1 - (size_ - index_ - 1 - delta) % size_;
        }
    }

    WrappedIndex operator+(int delta) const {
        auto lhs = *this;
        lhs += delta;
        return lhs;
    }

private:
    size_t size_;
    size_t index_ = 0;
};

class LauncherScene : public App::Scene {
public:
    LauncherScene();
    virtual ~LauncherScene() = default;

    void init() override;
    void update() override;
    void draw() const override;

private:
    const Font font;
    const Texture background;
    Effect effect;
    std::vector<Game> games;
    WrappedIndex selectedGameIndex;

    bool inDemoMode;
    s3d::uint32 lastActiveTime;
    Stopwatch stopwatch;
};

}
