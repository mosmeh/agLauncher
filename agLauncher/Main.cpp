namespace ag {

struct App {
	String title, desc;
	FilePath exec;
	Texture thumb;

	App(const JSONValue& jsonValue) {
		title = jsonValue[L"title"].getString();
		desc = jsonValue[L"desc"].getString();
		exec = jsonValue[L"exec"].getString();
		thumb = Texture(jsonValue[L"thumb"].getString());
	}
};

void drawThumbnail(const Texture& texture, double relativeXPos, double alpha = 1.0) {
	const auto size = Window::Size();
	const auto scale = 0.237 * Window::Size().y / texture.size.y;
	texture.scale(scale).drawAt({relativeXPos * size.x, 0.451 * size.y}, AlphaF(alpha));
}

size_t wrapIndex(size_t index, size_t size, int delta) {
	if (delta >= 0) {
		return (index + delta) % size;
	} else {
		return size - 1 - (size - index - 1 - delta) % size;
	}
}

}

void Main() {
	using namespace ag;

	SetProcessDPIAware();

	Window::Resize(1280, 720);
	Window::SetStyle(WindowStyle::Sizeable);
	Addon::Register<FixedAspectRatio>();
	Window::Maximize();

	const Font font(34);
	const Texture background(L"background.png");

	std::vector<App> apps;

	JSONReader reader(L"apps.json");
	for (const auto& value : reader.root().getArray()) {
		apps.emplace_back(value);
	}
	
	size_t selectedAppIndex = 0;

	while (System::Update()) {
		selectedAppIndex = wrapIndex(selectedAppIndex, apps.size(), Input::KeyRight.clicked - Input::KeyLeft.clicked);

		const auto width = Window::Size().x;
		const auto height = Window::Size().y;

		const auto& selectedApp = apps.at(selectedAppIndex);
		const auto& prevApp = apps.at(wrapIndex(selectedAppIndex, apps.size(), -1));
		const auto& nextApp = apps.at(wrapIndex(selectedAppIndex, apps.size(), 1));

		background.resize(width, height).draw();

		// selected app - card background and thumbnail
		const s3d::RoundRect selectedItemRect({0.345 * width, 0.225 * height}, {0.31 * width, 0.4 * height}, 0.00385 * width);
		selectedItemRect.draw();
		selectedItemRect.drawFrame(0.0, 0.002 * width, Color(255, 119, 5));
		drawThumbnail(selectedApp.thumb, 0.5);

		// previous and next app - thumbnail
		drawThumbnail(prevApp.thumb, 0.2, 0.5);
		drawThumbnail(nextApp.thumb, 0.8, 0.5);

		// side - background
		Rect({0, 0.169 * height}, {0.0495 * width, 0.532 * height}).draw(AlphaF(0.6));
		Rect({0.951 * width, 0.169 * height}, {0.0495 * width, 0.532 * height}).draw(AlphaF(0.6));

		// side - triangle
		Triangle({0.022 * width, 0.419 * height}, 0.04 * width).rotated(-Math::HalfPi).draw(Color(215, 58, 89));
		Triangle({0.978 * width, 0.419 * height}, 0.04 * width).rotated(Math::HalfPi).draw(Color(215, 58, 89));

		// description - background
		Rect({0, 0.768 * height}, {width, 0.231 * height}).draw(AlphaF(0.9));

		// description - line
		Line({0, 0.768 * height}, {width, 0.768 * height}).draw(0.00488 * height, Color(0, 162, 154));

		static const Vec2 REF_SIZE(1920, 1080);
		Graphics2D::SetTransform(Mat3x2(
			width / REF_SIZE.x, 0,
			0, height / REF_SIZE.y,
			0, 0
		));

		// selected app - title
		font.drawCenter(selectedApp.title, {0.5 * REF_SIZE.x, 0.293 * REF_SIZE.y}, Color(215, 58, 89));

		// previous and next app - title
		font.drawCenter(prevApp.title, {0.2 * REF_SIZE.x, 0.293 * REF_SIZE.y}, Color(215, 58, 89, 128));
		font.drawCenter(nextApp.title, {0.8 * REF_SIZE.x, 0.293 * REF_SIZE.y}, Color(215, 58, 89, 128));

		// description - text
		font.drawCenter(selectedApp.desc, {0.5 * REF_SIZE.x, 0.884 * REF_SIZE.y}, Palette::Black);

		Graphics2D::ClearTransform();
	}
}
