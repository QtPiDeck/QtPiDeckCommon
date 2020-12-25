#pragma once

#include <memory>

namespace QtPiDeck::Services {
template <class TService>
class ServiceUser;

class Ioc;

namespace detail {
template<class T>
void SetService(QtPiDeck::Services::ServiceUser<T> & service, const Ioc & ioc) noexcept;
}

template <class TService>
class ServiceUser {
    template<class T>
    friend void detail::SetService(ServiceUser<T> & service, const Ioc & ioc) noexcept; // NOLINT(readability-redundant-declaration)
protected:
    void SetService(std::shared_ptr<TService> service) {
        m_service = std::move(service);
    }
    auto Service() -> std::shared_ptr<TService>&  {
        return m_service;
    }
private:
    std::shared_ptr<TService> m_service;
};

namespace detail {
class HasDependecies {};
}

template <class... TServices>
class UseServices : public detail::HasDependecies, public ServiceUser<TServices>... {
protected:
    template<class TService>
    auto Service() -> std::shared_ptr<TService> & {
        return ServiceUser<TService>::Service();
    }
};
}
