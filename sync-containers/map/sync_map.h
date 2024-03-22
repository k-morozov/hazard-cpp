//
// Created by konstantin on 22.03.24.
//

#pragma once

#include <simple/hazard_ptr.h>

#include <atomic>
#include <memory>
#include <unordered_map>

namespace NSync {

template <class K, class V>
class SyncMap {
    using Ptr = std::shared_ptr<const std::unordered_map<K, V>>;

public:
    SyncMap() {
        mutable_map_ = std::make_shared<std::unordered_map<K, V>>();
        snapshot_.store(new Snapshot{mutable_map_, false});
    }

    ~SyncMap() {
        auto* old_snapshot = NHazard::Acquire(&snapshot_);
        NHazard::Release();
        NHazard::Retire(old_snapshot);
    }

    bool Lookup(const K& key, V* value) {
        auto* old_snapshot = NHazard::Acquire(&snapshot_);

        if (old_snapshot->dirty) {
            std::lock_guard g(mutex_);
            operation_count_++;

            const auto& data = *mutable_map_;
            bool found = false;
            if (data.contains(key)) {
                found = true;
                *value = data.at(key);
            }

            NHazard::Release();

            if (operation_count_ > 2) {
                using Tp = std::unordered_map<K, V>;
                using Tp1 = const Tp;

                auto v = old_snapshot->read_only;
                auto updated = std::const_pointer_cast<Tp, Tp1>(v);

                updated->merge(*mutable_map_);

                snapshot_.store(new Snapshot{updated, false});
                NHazard::Retire(old_snapshot);
            }

            return found;
        }

        const auto& data = *old_snapshot->read_only;
        bool found = false;
        if (data.contains(key)) {
            found = true;
            *value = data.at(key);
        }

        NHazard::Release();
        return found;
    }

    bool Insert(const K& key, const V& value) {
        std::lock_guard g(mutex_);

        if (snapshot_.load()->read_only->contains(key)) {
            return false;
        }

        auto& data = *mutable_map_;
        if (data.contains(key)) {
            return false;
        }

        data[key] = value;
        operation_count_ = 0;

        return true;
    }

private:
    struct Snapshot {
        const std::shared_ptr<const std::unordered_map<K, V>> read_only;

        // Indicates that read_only snapshot may be incomplete and lookup should
        // take lock.
        const bool dirty;
    };

    std::atomic<Snapshot*> snapshot_;

    std::shared_ptr<std::unordered_map<K, V>> mutable_map_;
    size_t operation_count_ = 0;
    std::mutex mutex_;
};

}  // namespace NSync