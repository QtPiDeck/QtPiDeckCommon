#include "BoostUnitTest.hpp"

#include <QBuffer>

#include "Network/DeckDataStream.hpp"
#include "Utilities/Logging.hpp"

CT_BOOST_AUTO_TEST_SUITE(MessageHeaderTests)
using namespace QtPiDeck::Network;

CT_BOOST_AUTO_TEST_CASE(shouldSetStreamConfigurationsDefaultConstructor) {
  const DeckDataStream dds;
  CT_BOOST_TEST(dds.byteOrder() == DeckDataStream::BigEndian);
  CT_BOOST_TEST(dds.version() == DeckDataStream::Qt_5_11);
}

CT_BOOST_AUTO_TEST_CASE(shouldSetStreamConfigurationsQIODeviceConstructor) {
  QBuffer qb;
  const DeckDataStream dds{&qb};
  CT_BOOST_TEST(dds.byteOrder() == DeckDataStream::BigEndian);
  CT_BOOST_TEST(dds.version() == DeckDataStream::Qt_5_11);
}

CT_BOOST_AUTO_TEST_CASE(shouldSetStreamConfigurationsQByteArrayWriteConstructor) {
  QByteArray qba;
  const DeckDataStream dds{&qba, DeckDataStream::OpenModeFlag::WriteOnly};
  CT_BOOST_TEST(dds.byteOrder() == DeckDataStream::BigEndian);
  CT_BOOST_TEST(dds.version() == DeckDataStream::Qt_5_11);
}

CT_BOOST_AUTO_TEST_CASE(shouldSetStreamConfigurationsQByteArrayReadConstructor) {
  const QByteArray qba;
  const DeckDataStream dds{qba};
  CT_BOOST_TEST(dds.byteOrder() == DeckDataStream::BigEndian);
  CT_BOOST_TEST(dds.version() == DeckDataStream::Qt_5_11);
}

CT_BOOST_AUTO_TEST_SUITE_END()