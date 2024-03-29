#include "BoostUnitTest.hpp"

#include "Utilities/QmlHelperDetail.hpp"

CT_BOOST_AUTO_TEST_SUITE(QmlHelpersTests)

class EmptyScreen;
class EmptyWindow;

class NoScreenGuiApp {
public:
  static auto primaryScreen() -> EmptyScreen* { return nullptr; }
};
class EmptyScreen {
public:
  [[nodiscard]] static auto logicalDotsPerInch() -> qreal { // LCOV_EXCL_LINE
    throw std::logic_error{"Should not be reachable"};      // LCOV_EXCL_LINE
  }
};
class EmptyWindow {};

using QtPiDeck::Utilities::detail::QmlHelperPrivate;

template<class Derived>
using NoScreenQmlHelper = QmlHelperPrivate<Derived, NoScreenGuiApp, EmptyWindow, EmptyScreen>;

template<template<typename> class Parent>
class Helper : public Parent<Helper<Parent>> {};

CT_BOOST_AUTO_TEST_CASE(dpiWithoutScreenShouldBeEqualToMinDpi) {
  Helper<NoScreenQmlHelper> helper;
  CT_BOOST_TEST(helper.dpi() == Helper<NoScreenQmlHelper>::minDpi);
}

template<class Screen>
class GuiAppWithScreen {
public:
  static auto primaryScreen() -> Screen* {
    static Screen screen{};
    return &screen;
  }
};

constexpr uint64_t smallDpi   = 60;
constexpr uint64_t regularDpi = 200;

class TestScreen : public QObject {
  Q_OBJECT // NOLINT
public:
  TestScreen()                  = default;
  TestScreen(const TestScreen&) = delete;
  TestScreen(TestScreen&&)      = delete;
  ~TestScreen() override        = default;
  auto operator=(const TestScreen&) -> TestScreen& = delete;
  auto operator=(TestScreen&&) -> TestScreen& = delete;

  [[nodiscard]] virtual auto logicalDotsPerInch() const -> qreal = 0;

signals:
  void logicalDotsPerInchChanged(qreal _t1);
};

class SmallDpiScreen final : public TestScreen {
public:
  [[nodiscard]] auto logicalDotsPerInch() const -> qreal final { return static_cast<qreal>(smallDpi); }
};

class RegularDpiScreen final : public TestScreen {
  Q_OBJECT // NOLINT
public:
  [[nodiscard]] auto logicalDotsPerInch() const -> qreal final { return static_cast<qreal>(regularDpi); }
signals:
  void logicalDotsPerInchChanged(qreal _t1);
};

static_assert(smallDpi < regularDpi);
static_assert(smallDpi < Helper<NoScreenQmlHelper>::minDpi);
static_assert(Helper<NoScreenQmlHelper>::minDpi < regularDpi);

template<class Derived>
using SmallDpiQmlHelper = QmlHelperPrivate<Derived, GuiAppWithScreen<SmallDpiScreen>, EmptyWindow, SmallDpiScreen>;

CT_BOOST_AUTO_TEST_CASE(dpiWithScreenAndSmallDpiShouldBeEqualToMinDpi) {
  Helper<SmallDpiQmlHelper> helper;
  CT_BOOST_TEST(helper.dpi() == Helper<NoScreenQmlHelper>::minDpi);
}

template<class Derived>
using RegularDpiQmlHelper =
    QmlHelperPrivate<Derived, GuiAppWithScreen<RegularDpiScreen>, EmptyWindow, RegularDpiScreen>;

CT_BOOST_AUTO_TEST_CASE(dpiWithScreenAndRegularDpiShouldBeEqualToRegularDpi) {
  Helper<RegularDpiQmlHelper> helper;
  CT_BOOST_TEST(helper.dpi() == regularDpi);
}

using namespace boost::unit_test;

const auto dpDataSet = data::make({0.0, static_cast<double>(Helper<NoScreenQmlHelper>::baseDpi),
                                   static_cast<double>(regularDpi), 1000.0}) ^
                       data::make({0.0, static_cast<double>(regularDpi), 250.0, 1250.0});

CT_BOOST_DATA_TEST_CASE(dpShouldBeCalculatedCorrectly, dpDataSet, input, expected) {
  Helper<RegularDpiQmlHelper> helper;
  CT_BOOST_TEST(helper.dp(input, helper.dpi()) == expected);
}

const auto& spDataSet = dpDataSet; // same until font scale is used in sp(...) function

CT_BOOST_DATA_TEST_CASE(spShouldBeCalculatedCorrectly, spDataSet, input, expected) {
  Helper<RegularDpiQmlHelper> helper;
  CT_BOOST_TEST(helper.sp(input, helper.dpi()) == expected);
}

template<class Window>
class GuiApp {
public:
  [[nodiscard]] static auto allWindows() -> std::vector<Window*> {
    static Window window{};
    std::vector<Window*> list;
    list.push_back(&window);
    return list;
  }
  [[nodiscard]] static auto primaryScreen() -> const TestScreen* { return &m_screen; }

private:
  static inline const RegularDpiScreen m_screen;
};

class Window : public QObject {
  Q_OBJECT // NOLINT
public:
  [[nodiscard]] static auto screen() -> const TestScreen* { return GuiApp<Window>::primaryScreen(); }
signals:
  void screenChanged(const RegularDpiScreen* _t1);

private:
};

template<class Derived>
using QmlHelper = QmlHelperPrivate<Derived, GuiApp<Window>, Window, TestScreen>;

template<template<typename> class Parent>
class UpdateableHelper : public Parent<UpdateableHelper<Parent>> {
  friend Parent<UpdateableHelper<Parent>>;

private:
  void updateDpi() {}
};

CT_BOOST_AUTO_TEST_CASE(windowCreatedOnPrimaryScreen) {
  UpdateableHelper<QmlHelper> helper;
  const auto dpiBeforeWindow = helper.dpi();
  helper.windowCreated();
  CT_BOOST_TEST(dpiBeforeWindow == helper.dpi());
}

namespace {
constexpr uint64_t differentDpi = 260;

class DifferentDpiScreen final : public TestScreen {
  Q_OBJECT // NOLINT
public:
  [[nodiscard]] auto logicalDotsPerInch() const -> qreal final { return static_cast<qreal>(differentDpi); }
signals:
  void logicalDotsPerInchChanged(qreal _t1);
};

class WindowWithScreen : public QObject {
  Q_OBJECT // NOLINT
public:
  [[nodiscard]] static auto screen() -> const TestScreen* {
    static DifferentDpiScreen screen;
    return &screen;
  }
signals:
  void screenChanged(const DifferentDpiScreen* _t1);

private:
};

template<class Derived>
using QmlHelperWithScreen = QmlHelperPrivate<Derived, GuiApp<WindowWithScreen>, WindowWithScreen, TestScreen>;
}

CT_BOOST_AUTO_TEST_CASE(windowCreatedOnNonPrimaryScreen) {
  UpdateableHelper<QmlHelperWithScreen> helper;
  const auto dpiBeforeWindow = helper.dpi();
  helper.windowCreated();
  CT_BOOST_TEST(dpiBeforeWindow != helper.dpi());
  CT_BOOST_TEST(differentDpi == helper.dpi());
}

namespace {
class WindowWithChangeableScreen : public QObject {
  Q_OBJECT // NOLINT
public:
  [[nodiscard]] auto screen() const -> const TestScreen* {
    return m_useDifferentScreen ? static_cast<const TestScreen*>(&m_differentScreen)
                                : static_cast<const TestScreen*>(&m_regularScreen);
  }

  void changeScreen() {
    m_useDifferentScreen = true;
    emit screenChanged(&m_differentScreen);
  }
signals:
  void screenChanged(const TestScreen* _t1);

private:
  RegularDpiScreen m_regularScreen;
  DifferentDpiScreen m_differentScreen;

  bool m_useDifferentScreen{false};
};

template<class Derived>
using QmlHelperWithChangeableScreen =
    QmlHelperPrivate<Derived, GuiApp<WindowWithChangeableScreen>, WindowWithChangeableScreen, TestScreen>;
}

CT_BOOST_AUTO_TEST_CASE(screenChangeShouldUpdateDpi) {
  UpdateableHelper<QmlHelperWithChangeableScreen> helper;
  const auto dpiBeforeWindow = helper.dpi();
  helper.windowCreated();
  CT_BOOST_TEST(dpiBeforeWindow == helper.dpi());
  GuiApp<WindowWithChangeableScreen>::allWindows()[0]->changeScreen();
  CT_BOOST_TEST(dpiBeforeWindow != helper.dpi());
  CT_BOOST_TEST(differentDpi == helper.dpi());
}
CT_BOOST_AUTO_TEST_SUITE_END()

#include "QmlHelperTests.moc"