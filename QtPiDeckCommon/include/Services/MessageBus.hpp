#pragma once

#include "Bus/Message.hpp"

#include "IMessageBus.hpp"
#include "QtPiDeckCommon_global.hpp"

namespace QtPiDeck::Services {
class QTPIDECKCOMMON_EXPORT MessageBus final : public QObject, public IMessageBus {
    Q_OBJECT
public:
    explicit MessageBus(QObject* parent) : QObject(parent) {}

    auto subscribe(QObject* context, const std::function<void(const Bus::Message&)>& method) -> QMetaObject::Connection final;
    auto subscribe(QObject* context, const std::function<void(const Bus::Message&)>& method, uint64_t messageType) -> QMetaObject::Connection final;
    void unsubscribe(QMetaObject::Connection) final;
    void sendMessage(Bus::Message) final;

signals:
    void newMessage(Bus::Message);
};
}
