#include "BoostUnitTest.hpp"

#include <array>
#include <type_traits>

#include "Services/Ioc.hpp"

struct IocFixture {
  std::unique_ptr<QtPiDeck::Services::Ioc> ioc{std::make_unique<QtPiDeck::Services::Ioc>()};
};

CT_BOOST_FIXTURE_TEST_SUITE(IocTests, IocFixture)

using namespace QtPiDeck::Services;

template<uint32_t id>
struct Interface : public ServiceInterface {
  ~Interface() override = 0;

protected:
  Interface() noexcept            = default;
  Interface(const Interface&)     = default;
  Interface(Interface&&) noexcept = default;

  auto operator=(const Interface&) -> Interface& = default;
  auto operator=(Interface&&) noexcept -> Interface& = default;
};

using Interface0 = Interface<0>;
template<>
Interface0::~Interface() = default;

template<uint32_t id, uint32_t intId = id, class... TDeps>
struct Implementation final : Interface<intId>, UseServices<TDeps...> {
  Implementation() noexcept                 = default;
  Implementation(const Implementation&)     = default;
  Implementation(Implementation&&) noexcept = default;

  ~Implementation() final = default;
  auto operator=(const Implementation&) -> Implementation& = default;
  auto operator=(Implementation&&) noexcept -> Implementation& = default;

  template<class TInt>
  auto ResolvedService() -> std::shared_ptr<TInt>& {
    return QtPiDeck::Services::ServiceUser<TInt>::service();
  }
};

using Implementation0 = Implementation<0>;

// NOLINTNEXTLINE
CT_BOOST_AUTO_TEST_CASE(no_service) {
  auto service0 = ioc->resolveService<Interface0>();
  // NOLINTNEXTLINE
  CT_BOOST_TEST(service0 == nullptr);
}

// NOLINTNEXTLINE
CT_BOOST_AUTO_TEST_CASE(register_service) {
  ioc->registerService<Interface0, Implementation0>();
  auto service0 = ioc->resolveService<Interface0>();
  // NOLINTNEXTLINE
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service0.get()) != nullptr);
}

using Interface1 = Interface<1>;
template<>
Interface1::~Interface() = default;

using Implementation1 = Implementation<1, 1>;

CT_BOOST_AUTO_TEST_CASE(register_two_services) {
  ioc->registerService<Interface0, Implementation0>();
  auto service0 = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service0.get()) != nullptr);
  ioc->registerService<Interface1, Implementation1>();
  auto service1 = ioc->resolveService<Interface1>();
  CT_BOOST_TEST(dynamic_cast<Implementation1*>(service1.get()) != nullptr);
}

using Implementation1_0 = Implementation<1, 0>;

CT_BOOST_AUTO_TEST_CASE(replace_service) {
  ioc->registerService<Interface0, Implementation0>();
  auto service0 = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service0.get()) != nullptr);
  ioc->registerService<Interface0, Implementation1_0>();
  auto service1_0 = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation1_0*>(service1_0.get()) != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service1_0.get()) == nullptr);
}

CT_BOOST_AUTO_TEST_CASE(resolve_by_impl) {
  ioc->registerService<Interface0, Implementation0>();
  auto service = ioc->resolveService<Implementation0>();
  CT_BOOST_TEST(service != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(register_impl) {
  ioc->registerService<Implementation0, Implementation0>();
  auto service = ioc->resolveService<Implementation0>();
  CT_BOOST_TEST(service != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(register_specialization) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Implementation0, Implementation0>();
  auto service = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service.get()) != nullptr);
  auto service2 = ioc->resolveService<Implementation0>();
  CT_BOOST_TEST(service2 != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(register_specialization_different) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Implementation1_0, Implementation1_0>();
  auto service = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service.get()) != nullptr);
  auto service2 = ioc->resolveService<Implementation1_0>();
  CT_BOOST_TEST(service2 != nullptr);
  auto service3 = ioc->resolveService<Implementation0>();
  CT_BOOST_TEST(service3 != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(resolve_not_same) {
  ioc->registerService<Interface0, Implementation0>();
  auto service = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service.get()) != nullptr);
  auto service2 = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service2.get()) != nullptr);
  CT_BOOST_TEST(service != service2);
}

CT_BOOST_AUTO_TEST_CASE(register_singleton) {
  ioc->registerSingleton<Interface0>(std::make_shared<Implementation0>());
  auto service = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(service.use_count() == 2);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service.get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(replace_singleton) {
  ioc->registerSingleton<Interface0>(std::make_shared<Implementation0>());
  auto service = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service.get()) != nullptr);
  ioc->registerSingleton<Interface0>(std::make_shared<Implementation1_0>());
  CT_BOOST_TEST(service.use_count() == 1);
  auto service2 = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service2.get()) == nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation1_0*>(service2.get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(register_two_singletons) {
  ioc->registerSingleton<Interface0>(std::make_shared<Implementation0>());
  auto service = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service.get()) != nullptr);
  ioc->registerSingleton<Interface1>(std::make_shared<Implementation1>());
  auto service2 = ioc->resolveService<Interface1>();
  CT_BOOST_TEST(dynamic_cast<Implementation1*>(service2.get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(resolve_same_singleton) {
  ioc->registerSingleton<Interface0>(std::make_shared<Implementation0>());
  auto service = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service.get()) != nullptr);
  auto service2 = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(service2.get()) != nullptr);
  CT_BOOST_TEST(service == service2);
}

using Implementation0WithDeps = Implementation<0, 0, Interface1>;

CT_BOOST_AUTO_TEST_CASE(resolve_with_missing_dependency) {
  ioc->registerService<Interface0, Implementation0WithDeps>();
  auto service = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation0WithDeps*>(service.get()) != nullptr);
  CT_BOOST_TEST(std::dynamic_pointer_cast<Implementation0WithDeps>(service)->ResolvedService<Interface1>() == nullptr);
}

CT_BOOST_AUTO_TEST_CASE(resolve_with_dependency) {
  ioc->registerService<Interface0, Implementation0WithDeps>();
  ioc->registerService<Interface1, Implementation1>();
  auto service = ioc->resolveService<Interface0>();
  CT_BOOST_TEST(dynamic_cast<Implementation0WithDeps*>(service.get()) != nullptr);
  auto resolverService = std::dynamic_pointer_cast<Implementation0WithDeps>(service)->ResolvedService<Interface1>();
  CT_BOOST_TEST(resolverService != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation1*>(resolverService.get()) != nullptr);
}

using Interface2 = Interface<2>;
template<>
Interface2::~Interface() = default;

using Implementation2WithTwoDeps = Implementation<2, 2, Interface0, Interface1>;

CT_BOOST_AUTO_TEST_CASE(resolve_with_two_dependencies) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Interface1, Implementation1>();
  ioc->registerService<Interface2, Implementation2WithTwoDeps>();
  auto service = ioc->resolveService<Interface2>();
  CT_BOOST_TEST(dynamic_cast<Implementation2WithTwoDeps*>(service.get()) != nullptr);
  auto resolvedService1 = std::dynamic_pointer_cast<Implementation2WithTwoDeps>(service)->ResolvedService<Interface0>();
  CT_BOOST_TEST(resolvedService1 != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(resolvedService1.get()) != nullptr);
  auto resolvedService2 = std::dynamic_pointer_cast<Implementation2WithTwoDeps>(service)->ResolvedService<Interface1>();
  CT_BOOST_TEST(resolvedService2 != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation1*>(resolvedService2.get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(resolve_with_multilevel_dependencies) {
  ioc->registerService<Interface0, Implementation0WithDeps>();
  ioc->registerService<Interface1, Implementation1>();
  ioc->registerService<Interface2, Implementation2WithTwoDeps>();
  auto service = ioc->resolveService<Interface2>();
  CT_BOOST_TEST(dynamic_cast<Implementation2WithTwoDeps*>(service.get()) != nullptr);
  auto resolverService1 = std::dynamic_pointer_cast<Implementation2WithTwoDeps>(service)->ResolvedService<Interface0>();
  CT_BOOST_TEST(resolverService1 != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0WithDeps*>(resolverService1.get()) != nullptr);
  auto resolverService2 =
      std::dynamic_pointer_cast<Implementation0WithDeps>(resolverService1)->ResolvedService<Interface1>();
  CT_BOOST_TEST(resolverService2 != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation1*>(resolverService2.get()) != nullptr);
}

class ClassWithoutDependencies {};

auto operator<<(std::ostream& ostr, const std::unique_ptr<ClassWithoutDependencies>& right) -> std::ostream& {
  ostr << "ClassWithoutDependencies {" << right.get() << "}";
  return ostr;
}

CT_BOOST_AUTO_TEST_CASE(make_unique_pointer_without_dependencies) {
  auto result = ioc->make<ClassWithoutDependencies, CreationType::UniquePointer>();
  static_assert(std::is_same_v<std::unique_ptr<ClassWithoutDependencies>, decltype(result)>);
  CT_BOOST_TEST(result != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_shared_pointer_without_dependencies) {
  auto result = ioc->make<ClassWithoutDependencies, CreationType::SharedPointer>();
  static_assert(std::is_same_v<std::shared_ptr<ClassWithoutDependencies>, decltype(result)>);
  CT_BOOST_TEST(result != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_copy_without_dependencies) {
  [[maybe_unused]] auto result = ioc->make<ClassWithoutDependencies, CreationType::Copy>();
  static_assert(std::is_same_v<ClassWithoutDependencies, decltype(result)>);
  CT_BOOST_TEST(true);
}

CT_BOOST_AUTO_TEST_CASE(make_raw_in_memory_without_dependencies) {
  alignas(ClassWithoutDependencies) std::array<std::byte, sizeof(ClassWithoutDependencies)> buffer{std::byte{}};
  auto* result = ioc->make<ClassWithoutDependencies, CreationType::RawInMemory>(buffer.data());
  static_assert(std::is_same_v<ClassWithoutDependencies*, decltype(result)>);
  CT_BOOST_TEST(result != nullptr);
  result->~ClassWithoutDependencies();
}

CT_BOOST_AUTO_TEST_CASE(make_raw_without_dependencies) {
  auto* result = ioc->make<ClassWithoutDependencies, CreationType::Raw>();
  static_assert(std::is_same_v<ClassWithoutDependencies*, decltype(result)>);
  auto wrapper = std::unique_ptr<ClassWithoutDependencies>(result);
  CT_BOOST_TEST(result != nullptr);
}

class ClassWithOneDependencyConstructor : public UseServices<Interface0> {
public:
  ClassWithOneDependencyConstructor(std::shared_ptr<Interface0> interface0) { setService(std::move(interface0)); }

  auto getInterface() -> std::shared_ptr<Interface0> { return service<Interface0>(); }
};

auto operator<<(std::ostream& ostr, const std::unique_ptr<ClassWithOneDependencyConstructor>& right) -> std::ostream& {
  ostr << "ClassWithOneDependencyConstructor {" << right.get() << "}";
  return ostr;
}

CT_BOOST_AUTO_TEST_CASE(make_unique_pointer_with_one_dependency_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  auto result = ioc->make<ClassWithOneDependencyConstructor, CreationType::UniquePointer>();
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_shared_pointer_with_one_dependency_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  auto result = ioc->make<ClassWithOneDependencyConstructor, CreationType::SharedPointer>();
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_copy_with_one_dependency_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  auto result = ioc->make<ClassWithOneDependencyConstructor, CreationType::Copy>();
  CT_BOOST_TEST(result.getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result.getInterface().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_raw_in_memory_with_one_dependency_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  using classType = ClassWithOneDependencyConstructor;
  alignas(classType) std::array<std::byte, sizeof(classType)> buffer{std::byte{}};
  auto* result = ioc->make<classType, CreationType::RawInMemory>(buffer.data());
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface().get()) != nullptr);
  result->~ClassWithOneDependencyConstructor();
}

CT_BOOST_AUTO_TEST_CASE(make_raw_with_one_dependency_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  using classType = ClassWithOneDependencyConstructor;
  auto result     = std::unique_ptr<classType>(ioc->make<classType, CreationType::Raw>());
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface().get()) != nullptr);
}

class ClassWithOneDependencyConstructorQobject : public UseServices<Interface0> {
public:
  ClassWithOneDependencyConstructorQobject(QObject* /*parent*/, std::shared_ptr<Interface0> interface0) {
    setService(std::move(interface0));
  }

  auto getInterface() -> std::shared_ptr<Interface0> { return service<Interface0>(); }
};

auto operator<<(std::ostream& ostr, const std::unique_ptr<ClassWithOneDependencyConstructorQobject>& right)
    -> std::ostream& {
  ostr << "ClassWithOneDependencyConstructorQobject {" << right.get() << "}";
  return ostr;
}

CT_BOOST_AUTO_TEST_CASE(make_unique_pointer_with_one_dependency_constructor_qobject) {
  ioc->registerService<Interface0, Implementation0>();
  auto result = ioc->make<ClassWithOneDependencyConstructorQobject, CreationType::UniquePointer>();
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_shared_pointer_with_one_dependency_constructor_qobject) {
  ioc->registerService<Interface0, Implementation0>();
  auto result = ioc->make<ClassWithOneDependencyConstructorQobject, CreationType::SharedPointer>();
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_copy_with_one_dependency_constructor_qobject) {
  ioc->registerService<Interface0, Implementation0>();
  auto result = ioc->make<ClassWithOneDependencyConstructorQobject, CreationType::Copy>();
  CT_BOOST_TEST(result.getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result.getInterface().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_raw_in_memory_with_one_dependency_constructor_qobject) {
  ioc->registerService<Interface0, Implementation0>();
  using classType = ClassWithOneDependencyConstructorQobject;
  alignas(classType) std::array<std::byte, sizeof(classType)> buffer{std::byte{}};
  auto* result = ioc->make<ClassWithOneDependencyConstructorQobject, CreationType::RawInMemory>(buffer.data());
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface().get()) != nullptr);
  result->~ClassWithOneDependencyConstructorQobject();
}

CT_BOOST_AUTO_TEST_CASE(make_raw_with_one_dependency_constructor_qobject) {
  ioc->registerService<Interface0, Implementation0>();
  using classType = ClassWithOneDependencyConstructorQobject;
  auto result     = std::unique_ptr<classType>(ioc->make<classType, CreationType::Raw>());
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface().get()) != nullptr);
}

class ClassWithOneDependencyNoConstructor : public UseServices<Interface0> {
public:
  auto getInterface() -> std::shared_ptr<Interface0> { return service<Interface0>(); }
};

auto operator<<(std::ostream& ostr, const std::unique_ptr<ClassWithOneDependencyNoConstructor>& right)
    -> std::ostream& {
  ostr << "ClassWithOneDependencyNoConstructor {" << right.get() << "}";
  return ostr;
}

CT_BOOST_AUTO_TEST_CASE(make_unique_pointer_with_one_dependency_no_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  auto result = ioc->make<ClassWithOneDependencyNoConstructor, CreationType::UniquePointer>();
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_shared_pointer_with_one_dependency_no_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  auto result = ioc->make<ClassWithOneDependencyNoConstructor, CreationType::SharedPointer>();
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_copy_with_one_dependency_no_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  auto result = ioc->make<ClassWithOneDependencyNoConstructor, CreationType::Copy>();
  CT_BOOST_TEST(result.getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result.getInterface().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_raw_in_memory_with_one_dependency_no_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  using classType = ClassWithOneDependencyNoConstructor;
  alignas(classType) std::array<std::byte, sizeof(classType)> buffer{std::byte{}};
  auto* result = ioc->make<classType, CreationType::RawInMemory>(buffer.data());
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface().get()) != nullptr);
  result->~ClassWithOneDependencyNoConstructor();
}

CT_BOOST_AUTO_TEST_CASE(make_raw_with_one_dependency_no_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  using classType = ClassWithOneDependencyNoConstructor;
  auto result     = std::unique_ptr<classType>(ioc->make<classType, CreationType::Raw>());
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface().get()) != nullptr);
}

class ClassWithTwoDependenciesConstructor : public UseServices<Interface0, Interface1> {
public:
  ClassWithTwoDependenciesConstructor(std::shared_ptr<Interface0> interface0, std::shared_ptr<Interface1> interface1) {
    setService(std::move(interface0));
    setService(std::move(interface1));
  }

  template<class TInterface>
  auto getInterface() -> std::shared_ptr<TInterface> {
    return service<TInterface>();
  }
};

auto operator<<(std::ostream& ostr, const std::unique_ptr<ClassWithTwoDependenciesConstructor>& right)
    -> std::ostream& {
  ostr << "ClassWithTwoDependenciesConstructor {" << right.get() << "}";
  return ostr;
}

CT_BOOST_AUTO_TEST_CASE(make_unique_pointer_with_two_dependencies_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Interface1, Implementation1>();
  auto result = ioc->make<ClassWithTwoDependenciesConstructor, CreationType::UniquePointer>();
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface0>() != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface1>() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface<Interface0>().get()) != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation1*>(result->getInterface<Interface1>().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_shared_pointer_with_two_dependencies_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Interface1, Implementation1>();
  auto result = ioc->make<ClassWithTwoDependenciesConstructor, CreationType::SharedPointer>();
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface0>() != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface1>() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface<Interface0>().get()) != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation1*>(result->getInterface<Interface1>().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_copy_with_two_dependencies_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Interface1, Implementation1>();
  auto result = ioc->make<ClassWithTwoDependenciesConstructor, CreationType::Copy>();
  CT_BOOST_TEST(result.getInterface<Interface0>() != nullptr);
  CT_BOOST_TEST(result.getInterface<Interface1>() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result.getInterface<Interface0>().get()) != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation1*>(result.getInterface<Interface1>().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_raw_in_memory_with_two_dependencies_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Interface1, Implementation1>();
  using classType = ClassWithTwoDependenciesConstructor;
  alignas(classType) std::array<std::byte, sizeof(classType)> buffer{std::byte{}};
  auto* result = ioc->make<classType, CreationType::RawInMemory>(buffer.data());
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface0>() != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface1>() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface<Interface0>().get()) != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation1*>(result->getInterface<Interface1>().get()) != nullptr);
  result->~ClassWithTwoDependenciesConstructor();
}

CT_BOOST_AUTO_TEST_CASE(make_raw_with_two_dependencies_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Interface1, Implementation1>();
  using classType = ClassWithTwoDependenciesConstructor;
  auto result     = std::unique_ptr<classType>(ioc->make<classType, CreationType::Raw>());
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface0>() != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface1>() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface<Interface0>().get()) != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation1*>(result->getInterface<Interface1>().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_unique_pointer_with_nested_dependencies_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Interface1, Implementation<1, 1, Interface0>>();
  auto result = ioc->make<ClassWithTwoDependenciesConstructor, CreationType::UniquePointer>();
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface0>() != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface1>() != nullptr);
  auto castedInterface =
      std::dynamic_pointer_cast<Implementation<1, 1, Interface0>>(result->getInterface<Interface1>());
  CT_BOOST_TEST(castedInterface != nullptr);
  CT_BOOST_TEST(castedInterface->ResolvedService<Interface0>() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(castedInterface->ResolvedService<Interface0>().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_shared_pointer_with_nested_dependencies_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Interface1, Implementation<1, 1, Interface0>>();
  auto result = ioc->make<ClassWithTwoDependenciesConstructor, CreationType::SharedPointer>();
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface0>() != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface1>() != nullptr);
  auto castedInterface =
      std::dynamic_pointer_cast<Implementation<1, 1, Interface0>>(result->getInterface<Interface1>());
  CT_BOOST_TEST(castedInterface != nullptr);
  CT_BOOST_TEST(castedInterface->ResolvedService<Interface0>() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(castedInterface->ResolvedService<Interface0>().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_copy_with_nested_dependencies_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Interface1, Implementation<1, 1, Interface0>>();
  auto result = ioc->make<ClassWithTwoDependenciesConstructor, CreationType::Copy>();
  CT_BOOST_TEST(result.getInterface<Interface0>() != nullptr);
  CT_BOOST_TEST(result.getInterface<Interface1>() != nullptr);
  auto castedInterface = std::dynamic_pointer_cast<Implementation<1, 1, Interface0>>(result.getInterface<Interface1>());
  CT_BOOST_TEST(castedInterface != nullptr);
  CT_BOOST_TEST(castedInterface->ResolvedService<Interface0>() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(castedInterface->ResolvedService<Interface0>().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_raw_in_memory_with_nested_dependencies_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Interface1, Implementation<1, 1, Interface0>>();
  using classType = ClassWithTwoDependenciesConstructor;
  alignas(classType) std::array<std::byte, sizeof(classType)> buffer{std::byte{}};
  auto* result = ioc->make<classType, CreationType::RawInMemory>(buffer.data());
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface0>() != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface1>() != nullptr);
  auto castedInterface =
      std::dynamic_pointer_cast<Implementation<1, 1, Interface0>>(result->getInterface<Interface1>());
  CT_BOOST_TEST(castedInterface != nullptr);
  CT_BOOST_TEST(castedInterface->ResolvedService<Interface0>() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(castedInterface->ResolvedService<Interface0>().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_CASE(make_raw_with_nested_dependencies_constructor) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Interface1, Implementation<1, 1, Interface0>>();
  using classType = ClassWithTwoDependenciesConstructor;
  auto result     = std::unique_ptr<classType>(ioc->make<classType, CreationType::Raw>());
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface0>() != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface1>() != nullptr);
  auto castedInterface =
      std::dynamic_pointer_cast<Implementation<1, 1, Interface0>>(result->getInterface<Interface1>());
  CT_BOOST_TEST(castedInterface != nullptr);
  CT_BOOST_TEST(castedInterface->ResolvedService<Interface0>() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(castedInterface->ResolvedService<Interface0>().get()) != nullptr);
}

class ClassWithTwoDependenciesConstructorTuple : public UseServices<Interface0, Interface1> {
public:
  ClassWithTwoDependenciesConstructorTuple(dependency_list list) {
    setServices(std::move(list));
  }

  template<class TInterface>
  auto getInterface() -> std::shared_ptr<TInterface> {
    return service<TInterface>();
  }
};

auto operator<<(std::ostream& ostr, const std::unique_ptr<ClassWithTwoDependenciesConstructorTuple>& right)
    -> std::ostream& {
  ostr << "ClassWithTwoDependenciesConstructorTuple {" << right.get() << "}";
  return ostr;
}

CT_BOOST_AUTO_TEST_CASE(make_unique_pointer_with_two_dependencies_constructor_tuple) {
  ioc->registerService<Interface0, Implementation0>();
  ioc->registerService<Interface1, Implementation1>();
  auto result = ioc->make<ClassWithTwoDependenciesConstructorTuple, CreationType::UniquePointer>();
  CT_BOOST_TEST(result != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface0>() != nullptr);
  CT_BOOST_TEST(result->getInterface<Interface1>() != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation0*>(result->getInterface<Interface0>().get()) != nullptr);
  CT_BOOST_TEST(dynamic_cast<Implementation1*>(result->getInterface<Interface1>().get()) != nullptr);
}

CT_BOOST_AUTO_TEST_SUITE_END()

//#include "IocTests.moc"
