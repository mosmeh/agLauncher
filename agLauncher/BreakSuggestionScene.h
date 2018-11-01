#pragma once

#include "Common.h"

namespace ag {

class BreakSuggestionScene : public App::Scene {
public:
    BreakSuggestionScene();
    virtual ~BreakSuggestionScene() = default;

    void init() override;
    void update() override;
    void draw() const override;

private:
    const Font font;
};

}
