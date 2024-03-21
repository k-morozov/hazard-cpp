//
// Created by konstantin on 21.03.24.
//

#pragma once

#include <atomic>

#include <simple/hazard_ptr.h>

namespace NSync {

template <typename T>

class Stack {
public:
    Stack() = default;

    void Clear() {}

    void Push(const T& value) {

    }
    bool Pop(T* value) {}
private:
    struct Node {
        T value;
        Node* next;
    };

    std::atomic<Node*> head_{};
};

}  // namespace NSync