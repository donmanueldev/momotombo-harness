#pragma once

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

namespace redisforge::shared::di
{
    class ServiceContainer
    {
    public:
        template <typename T>
        void registerInstance(std::shared_ptr<T> instance)
        {
            services_[std::type_index(typeid(T))] = std::move(instance);
        }

        template <typename T>
        std::shared_ptr<T> resolve() const
        {
            const auto iterator = services_.find(std::type_index(typeid(T)));
            if (iterator == services_.end())
            {
                throw std::runtime_error("Requested service is not registered.");
            }

            return std::static_pointer_cast<T>(iterator->second);
        }

    private:
        std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
    };
} // namespace redisforge::shared::di
