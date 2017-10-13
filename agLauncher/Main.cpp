namespace ag {

static const Color textColor(215, 58, 89);

struct App {
	String title, desc;
	FilePath exec;
	Texture thumb;

	App(const JSONValue& jsonValue) :
		title(jsonValue[L"title"].getString()),
		desc(jsonValue[L"desc"].getString()),
		thumb(Texture(jsonValue[L"thumb"].getString())),
		exec(jsonValue[L"exec"].getString()) {}

	void draw(double relativeXPos, double alpha = 1.0) const {
		const auto size = Window::BaseSize();
		const auto scale = 0.237 * size.y / thumb.size.y;
		thumb.scale(scale).drawAt({relativeXPos * size.x, 0.451 * size.y}, AlphaF(alpha));

		static const Font font(34);
		font.drawCenter(title, {relativeXPos * size.x, 0.293 * size.y}, textColor);
	}
};

enum class Region {
	Left, Center, Right
};

class DemoAnimation : public IEffect {
public:
	DemoAnimation(const App& app, int startIndex, int endIndex, double duration = 0.1) :
		app_(app),
		startIndex_(startIndex),
		start_(relativeIndexToXPos(startIndex)),
		end_(relativeIndexToXPos(endIndex)),
		duration_(duration) {}

	bool update(double t) override {
		static const Font font(34);
		const auto xPos = start_ + (end_ - start_) * t / duration_;
		app_.draw(xPos, startIndex_ == 1 ? 1.0 : 0.6);

		return t <= duration_;
	}

private:
	const App& app_;
	int startIndex_;
	double start_, end_;
	const double duration_;

	static double relativeIndexToXPos(int relIndex) {
		const auto width = 0.237;
		if (relIndex <= -2) {
			return -width / 2;
		} else if (relIndex == -1) {
			return 0.2;
		} else if (relIndex == 0) {
			return 0.5;
		} else if (relIndex == 1) {
			return 0.8;
		} else {
			return 1.0 + width / 2;
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
			index_  = size_ - 1 - (size_ - index_ - 1 - delta) % size_;
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


bool userIsActive() {
	if (Input::AnyKeyPressed()) {
		return true;
	}

	if (Mouse::Delta() != Point::Zero || Input::MouseL.pressed || Input::MouseR.pressed || Input::MouseM.pressed) {
		return true;
	}

	Gamepad gamepad(0);
	if (gamepad.isConnected()) {
		if (gamepad.povLeft.pressed || gamepad.povRight.pressed || gamepad.povBackward.pressed || gamepad.povForward.pressed) {
			return true;
		}
		for (s3d::uint32 i = 0; i < gamepad.num_buttons; ++i) {
			if (gamepad.button(i).pressed) {
				return true;
			}
		}
	}

	return false;
}

}

void Main() {
	using namespace ag;

	SetProcessDPIAware();

	Window::SetTitle(L"agLauncher");
	Graphics::SetBackground(Palette::Black);

	const Font font(34);
	const Texture background(L"/200");

	std::vector<App> apps;

	constexpr auto APPS_JSON = L"apps.json";
	if (!FileSystem::Exists(APPS_JSON)) {
#undef MessageBox
		MessageBox::Show(L"apps.jsonが見つかりません");
	}

	JSONReader reader(APPS_JSON);
	for (const auto& value : reader.root().getArray()) {
		apps.emplace_back(value);
	}

	constexpr auto width = 1920;
	constexpr auto height = 1080;
	ScalableWindow::Setup(width, height);

	WCHAR path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	const HWND hwnd = FindWindowW(FileSystem::NormalizedPath(path).c_str(),
		Window::GetTitle().c_str());

	Window::SetStyle(WindowStyle::Sizeable);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	SetWindowLong(hwnd, GWL_STYLE, 0);
	Window::Maximize();
	System::SetExitEvent(WindowEvent::CloseButton);

	Effect effect;

	const Gamepad gamepad(0);
	Optional<ProcessInfo> process;
	WrappedIndex selectedAppIndex(apps.size());

	bool inDemoMode = true;
	auto lastActiveTime = Time::GetMillisec();

	while (System::Update()) {
		const auto transformer = ScalableWindow::CreateTransformer();

		if (userIsActive() || (process && process->isRunning())) {
			lastActiveTime = Time::GetMillisec();
			if (inDemoMode) {
				effect.clear();
				inDemoMode = false;
			}
		} else if (!inDemoMode && Time::GetMillisec() > lastActiveTime + 3 * 1000) {
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
			if (gamepad.isConnected()) {
				if (gamepad.povLeft.clicked || gamepad.povLeft.pressedDuration > delay) {
					--delta;
				}
				if (gamepad.povRight.clicked || gamepad.povRight.pressedDuration > delay) {
					++delta;
				}
			}

			if (delta != 0) {
				delta = std::max(-1, std::min(1, delta));
				if (delta == 1) {
					for (int i = -1; i < 3; ++i) {
						effect.add<DemoAnimation>(apps.at((selectedAppIndex + i).get()), i, i - 1);
					}
				} else if (delta == -1) {
					for (int i = -2; i < 2; ++i) {
						effect.add<DemoAnimation>(apps.at((selectedAppIndex + i).get()), i, i + 1);
					}
				} else {
					assert(false);
				}
				selectedAppIndex += delta;
			}
		}

		const auto& selectedApp = apps.at(selectedAppIndex.get());

		if (process && process->isRunning()) {
			ShowWindow(hwnd, SW_HIDE);
		} else {
			ShowWindow(hwnd, SW_SHOW);

			if (!effect.hasEffects()) {
				if ((Input::KeyEnter | Input::KeySpace).clicked ||
					(Input::MouseL.clicked && mouseRegion == Region::Center) ||
					(gamepad.isConnected() && (gamepad.button(0) | gamepad.button(1) | gamepad.button(2) | gamepad.button(3)).clicked)) {
#undef CreateProcess
					process = System::CreateProcess(selectedApp.exec);
				}
			}
		}

		background.resize(width, height).draw();

		// selected app
		const s3d::RoundRect selectedItemRect({0.345 * width, 0.225 * height}, {0.31 * width, 0.4 * height}, 0.00385 * width);
		selectedItemRect.draw();
		selectedItemRect.drawFrame(0.0, 0.002 * width, Color(255, 119, 5));

		if (!effect.hasEffects()) {
			if (inDemoMode) {
				for (int i = -1; i < 3; ++i) {
					effect.add<DemoAnimation>(apps.at((selectedAppIndex + i).get()), i, i - 1, 8.0);
				}
				selectedAppIndex += 1;
			} else {
				selectedApp.draw(0.5, mouseRegion == Region::Center ? 0.8 : 1.0);

				for (int delta : {-1, 1}) {
					apps.at((selectedAppIndex + delta).get()).draw(0.5 + 0.3 * delta, 0.6);
				}
			}
		}
		effect.update();

		// side - background
		const Rect leftBar({0, 0.169 * height}, {0.0495 * width, 0.532 * height});
		const Rect rightBar({0.951 * width, 0.169 * height}, {0.0495 * width, 0.532 * height});
		leftBar.draw(AlphaF(mouseRegion == Region::Left ? 0.9 : 0.6));
		rightBar.draw(AlphaF(mouseRegion == Region::Right ? 0.9 : 0.6));

		// side - triangle
		const auto leftTriangle = Triangle({0.022 * width, 0.419 * height}, 0.04 * width).rotated(-Math::HalfPi);
		const auto rightTriangle = Triangle({0.978 * width, 0.419 * height}, 0.04 * width).rotated(Math::HalfPi);
		leftTriangle.draw(textColor);
		rightTriangle.draw(textColor);

		// description - background
		Rect({0, 0.768 * height}, {width, 0.231 * height}).draw(AlphaF(0.9));

		// description - line
		Line({0, 0.768 * height}, {width, 0.768 * height}).draw(0.00488 * height, Color(0, 162, 154));

		// description - text
		const auto descLines = selectedApp.desc.split(L'\n');
		double p = (0.77 + (1.0 - 0.77) / (descLines.size() + 1)) * height;
		for (const auto& line : descLines) {
			font.drawCenter(line, {0.5 * width, p}, Palette::Black);
			p += font.region(line).h;
		}

		font(selectedAppIndex.get() + 1, L"/", apps.size()).drawCenter({0.9 * width, 0.7 * height}, Palette::Black);
	}
}
