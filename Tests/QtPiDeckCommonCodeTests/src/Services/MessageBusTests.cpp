#include <QDataStream>

// NOLINTNEXTLINE
#define BOOST_TEST_MODULE MessageBusTests
#include "BoostUnitTest.hpp"

#include "Network/DeckDataStream.hpp"
#include "Services/MessageBus.hpp"

#include "Utilities/Logging.hpp"

auto main(int argc, char* argv[]) -> int {
  QtPiDeck::Utilities::initLogging("MessageBusTests");
  return boost::unit_test::unit_test_main(&init_unit_test, argc, argv);
}

struct MessageBusFixture {
  std::unique_ptr<QtPiDeck::Services::MessageBus> messageBus{std::make_unique<QtPiDeck::Services::MessageBus>(nullptr)};
};

CT_BOOST_FIXTURE_TEST_SUITE(MessageBusTests, MessageBusFixture)
using namespace QtPiDeck::Services;

CT_BOOST_AUTO_TEST_CASE(subscribe) {
  constexpr uint64_t messageType = 1234;
  uint64_t setMessageType        = 0;
  auto subscription              = messageBus->subscribe(
      messageBus.get(), [&setMessageType](const QtPiDeck::Bus::Message& mess) { setMessageType = mess.messageType; });
  messageBus->sendMessage({messageType});
  CT_BOOST_TEST(setMessageType == messageType);
}

CT_BOOST_AUTO_TEST_CASE(unsubscribe) {
  constexpr uint64_t messageType = 1234;
  uint64_t setMessageType        = 0;
  auto subscription              = messageBus->subscribe(
      messageBus.get(), [&setMessageType](const QtPiDeck::Bus::Message& mess) { setMessageType = mess.messageType; });
  messageBus->unsubscribe(subscription);
  messageBus->sendMessage({messageType});
  CT_BOOST_TEST(setMessageType == uint64_t{0});
}

CT_BOOST_AUTO_TEST_CASE(unsubscribeWithRAII) {
  constexpr uint64_t messageType = 1234;
  uint64_t setMessageType        = 0;
  {
    auto subscription = messageBus->subscribe(
        messageBus.get(), [&setMessageType](const QtPiDeck::Bus::Message& mess) { setMessageType = mess.messageType; });
  }
  messageBus->sendMessage({messageType});
  CT_BOOST_TEST(setMessageType == uint64_t{0});
}

CT_BOOST_AUTO_TEST_CASE(subscribeFiltered) {
  constexpr uint64_t messageType  = 1234;
  constexpr uint64_t messageType2 = 1235;
  uint64_t setMessageType         = 0;
  auto subscription               = messageBus->subscribe(
      messageBus.get(), [&setMessageType](const QtPiDeck::Bus::Message& mess) { setMessageType = mess.messageType; },
      messageType);
  messageBus->sendMessage({messageType2});
  CT_BOOST_TEST(setMessageType == uint64_t{0});
  messageBus->sendMessage({messageType});
  CT_BOOST_TEST(setMessageType == messageType);
}

CT_BOOST_AUTO_TEST_CASE(subscribeTwoSubscribers) {
  constexpr uint64_t messageType = 1234;
  uint64_t setMessageType        = 0;
  uint64_t setMessageType2       = 0;

  auto subscription = messageBus->subscribe(
      messageBus.get(), [&setMessageType](const QtPiDeck::Bus::Message& mess) { setMessageType = mess.messageType; });
  auto subscription2 = messageBus->subscribe(
      messageBus.get(), [&setMessageType2](const QtPiDeck::Bus::Message& mess) { setMessageType2 = mess.messageType; });
  messageBus->sendMessage({messageType});
  CT_BOOST_TEST(setMessageType == messageType);
  CT_BOOST_TEST(setMessageType2 == messageType);
}

class Listener : public QObject {
  Q_OBJECT // NOLINT
public:
  Listener(uint64_t expectedMessageType) : m_expectedMessageType(expectedMessageType), setMessageType() {}

  void callMe() noexcept { setMessageType = m_expectedMessageType; };

  void callMe(const QtPiDeck::Bus::Message& message) noexcept { setMessageType = message.messageType; }

  [[nodiscard]] auto getMessageType() const noexcept -> uint64_t { return setMessageType; }

private:
  const uint64_t m_expectedMessageType;
  uint64_t setMessageType;
};

CT_BOOST_AUTO_TEST_CASE(subscribeMember) {
  constexpr uint64_t messageType = 1234;
  Listener listener(messageType);
  auto subscription = QtPiDeck::Services::subscribe(*messageBus, &listener, &Listener::callMe);
  messageBus->sendMessage({messageType});
  auto setMessageType = listener.getMessageType();
  CT_BOOST_TEST(setMessageType == messageType);
}

CT_BOOST_AUTO_TEST_CASE(subscribeMemberFiltered) {
  constexpr uint64_t messageType = 1234;
  Listener listener(messageType);
  auto sub = QtPiDeck::Services::subscribe(*messageBus, &listener, static_cast<void (Listener::*)()>(&Listener::callMe),
                                           messageType);
  messageBus->sendMessage({messageType});
  auto setMessageType = listener.getMessageType();
  CT_BOOST_TEST(setMessageType == messageType);
}

CT_BOOST_AUTO_TEST_CASE(subscribeMemberFilteredWithType) {
  constexpr uint64_t messageType = 1234;
  Listener listener(messageType);
  auto sub = QtPiDeck::Services::subscribe(
      *messageBus, &listener, static_cast<void (Listener::*)(const QtPiDeck::Bus::Message&)>(&Listener::callMe),
      messageType);
  messageBus->sendMessage({messageType});
  auto setMessageType = listener.getMessageType();
  CT_BOOST_TEST(setMessageType == messageType);
}

CT_BOOST_AUTO_TEST_CASE(sendMessageWithPayload) {
  constexpr uint64_t messageType = 1234;
  const QString payloadData      = "Some random data";
  uint64_t setMessageType        = 0;
  QByteArray payload;

  auto subscription =
      messageBus->subscribe(messageBus.get(), [&payload, &setMessageType](const QtPiDeck::Bus::Message& mess) {
        setMessageType = mess.messageType;
        payload        = mess.payload;
      });
  messageBus->sendMessage({messageType, [&payloadData]() {
                             QByteArray qba;
                             QDataStream qds{&qba, QtPiDeck::Network::DeckDataStream::OpenModeFlag::WriteOnly};
                             qds << payloadData;
                             return qba;
                           }()});

  CT_BOOST_TEST(setMessageType == messageType);
  const QString receivedPayloadData = [&payload] {
    QDataStream qds{payload};
    QString data;
    qds >> data;
    return data;
  }();
  CT_BOOST_TEST(receivedPayloadData == payloadData);
}

CT_BOOST_AUTO_TEST_SUITE_END()

#include "MessageBusTests.moc"
