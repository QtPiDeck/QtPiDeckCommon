#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QtQml>

#include <array>

#include "QtPiDeckCommon.hpp"
#include "QmlTreeExplorer.hpp"
#include "TestSetup.hpp"

#include <Windows.h>
#include <iostream>
#include <cstring>

namespace QtPiDeck::Tests {
auto makeImports() noexcept {
    return std::array<std::string, 1> {"qrc:/qml/components"};
}

void registerTypes() noexcept {
    qmlRegisterType<QtPiDeck::Tests::QmlTreeExplorer>("QtPiDeck.Tests", 1, 0, "QmlTreeExplorer");
}

class Setup : public QObject
{
    // NOLINTNEXTLINE
    Q_OBJECT

public:
    Setup() { hack.registerTypes(); }

public slots: // NOLINT(readability-redundant-access-specifiers)
    void qmlEngineAvailable(MAYBE_UNUSED QQmlEngine *engine) // NOLINT(readability-convert-member-functions-to-static)
    {
        addImports15(engine, makeImports);
        callRegisterTypes15(registerTypes);
    }

private:
    QtPiDeckCommon hack;
};
}

#if QT_VERSION == QTPI4_VERSION
QTPIDECK_QUICK_TEST_MAIN_WITH_SETUP(QtPiDeck::Tests::CommandTests, QtPiDeck::Tests::Setup, QtPiDeck::Tests::makeImports, QtPiDeck::Tests::registerTypes) // NOLINT
#else
//QUICK_TEST_MAIN_WITH_SETUP(QtPiDeck::Tests::CommandTests, QtPiDeck::Tests::Setup) // NOLINT
int main(int argc, char** argv) {
  char buffer[32767] = {0};
  char b2[32767]     = R"(C:\Qt\5.15.2\msvc2019_64;)";
  GetEnvironmentVariable("PATH", &buffer[0], 32767);
  strcat(b2, buffer);
  //SetEnvironmentVariable("QTDIR", "");
  SetEnvironmentVariable("QT_PLUGIN_PATH ", R"(C:\Qt\5.15.2\msvc2019_64\plugins\)");
  std::cout << "VAR: " << buffer << "\n";
  QTEST_SET_MAIN_SOURCE_PATH
  QtPiDeck::Tests::Setup setup;
  return quick_test_main_with_setup(argc, argv, "QtPiDeck::Tests::CommandTests", QUICK_TEST_SOURCE_DIR, &setup);
}
#endif

#include "main.moc"
