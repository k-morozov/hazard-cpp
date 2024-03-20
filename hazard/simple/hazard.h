//
// Created by konstantin on 20.03.24.
//

#pragma once

namespace NHazard {

void RegisterThread();

void UnregisterThread();

template <class T>
T *Acquire(std::atomic<T *> *ptr);

void Release();

template <class T, class Deleter = std::default_delete<T>>
void Retire(T *value, Deleter deleter = {});

}  // namespace NHazard