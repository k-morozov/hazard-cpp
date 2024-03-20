//
// Created by konstantin on 20.03.24.
//

#pragma once

#include <atomic>
#include <memory>

namespace NHazard {

void RegisterThread() {}

void UnregisterThread() {}

template <class T>
T *Acquire(std::atomic<T *> *ptr) {
    int a = 1;
}

void Release() {}

template <class T, class Deleter = std::default_delete<T>>
void Retire(T *value, Deleter deleter = {}) {}

}  // namespace NHazard