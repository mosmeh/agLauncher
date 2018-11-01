#pragma once

namespace ag {

enum class SceneKey {
	Launcher,
	BreakSuggestion
};

struct Data {};

using App = SceneManager<SceneKey, Data>;

constexpr auto width = 1920;
constexpr auto height = 1080;

}
