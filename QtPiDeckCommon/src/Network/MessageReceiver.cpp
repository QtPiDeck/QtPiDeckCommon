#include "Network/MessageReceiver.hpp"

#include <iostream>

#include <QJsonDocument>
#include <cassert>
#include <qendian.h>
#include <qjsondocument.h>
#include <unordered_map>

#include "Bus/Message.hpp"
#include "Network/DeckDataStream.hpp"
#include "Network/Messages.hpp"
#include "Services/ISocketHolder.hpp"
#include "Utilities/ISerializable.hpp"
#include "Utilities/OnExit.hpp"
#include "Utilities/Traits.hpp"

namespace QtPiDeck::Network {
MessageReceiver::MessageReceiver(dependency_list dependencies) {
  Utilities::initLogger(m_slg, "MessageReceiver");
  setServices(std::move(dependencies));
  connect(service<Services::ISocketHolder>()->socket(), &QTcpSocket::readyRead, this, &MessageReceiver::readData);
}

namespace {
auto readHeader(QIODevice& socket) -> std::optional<Network::MessageHeader> {
  auto inStream = Network::DeckDataStream{&socket};
  auto object   = Network::MessageHeader{};

  inStream.startTransaction();
  inStream >> object;
  if (inStream.commitTransaction()) {
    return object;
  }

  return std::nullopt;
}
}

void MessageReceiver::readData() {
  auto* socket       = service<Services::ISocketHolder>()->socket();
  const auto& header = m_savedHeader.has_value() ? m_savedHeader : readHeader(*socket);
  if (!header.has_value()) {
    return;
  }

  auto payload          = QJsonDocument{};
  const auto hasPayload = header->dataSize > 0;
  if (hasPayload) {
    auto json     = QString{};
    auto inStream = Network::DeckDataStream{socket};

    inStream.startTransaction();
    inStream >> json;
    if (!inStream.commitTransaction()) {
      if (!m_savedHeader.has_value()) {
        m_savedHeader = header;
      }

      return;
    }

    payload = QJsonDocument::fromJson(json.toUtf8());

    m_savedHeader.reset();
  }

  const auto& messageTypeMapper = service<Services::IDeckMessageToBusMessageMapper>();
  const auto busMessageType     = messageTypeMapper->getBusMessageType(header->messageType);
  if (!busMessageType.has_value()) {
    return;
  }

  service<Services::IMessageBus>()->sendMessage({*busMessageType, payload});
}
}
