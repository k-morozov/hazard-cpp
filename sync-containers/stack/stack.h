//
// Created by konstantin on 21.03.24.
//

#pragma once

#include <simple/hazard_ptr.h>

#include <atomic>

namespace NSync {

template <typename T>
class Stack {
public:
    Stack() = default;
    ~Stack() { Clear(); }

    void Clear() {
        for (;;) {
            T value;
            if (bool ok = Pop(&value); !ok) {
                return;
            }
        }
    }

    void Push(const T& value) {
        auto* node = new Node{.value = value, .next = head_.load()};
        while (!head_.compare_exchange_weak(node->next, node)) {
        }
    }
    bool Pop(T* value) {
        for (;;) {
            auto* old_head = NHazard::Acquire(&head_);
            if (!old_head) {
                return false;
            }

            if (head_.compare_exchange_weak(old_head, old_head->next)) {
                *value = old_head->value;
                NHazard::Release();
                NHazard::Retire(old_head);
                return true;
            }
            NHazard::Release();
        }
    }

private:
    struct Node {
        T value;
        Node* next;
    };

    std::atomic<Node*> head_{};
};

}  // namespace NSync