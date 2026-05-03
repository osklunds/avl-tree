
#ifndef HEADER_MAIN_HPP
#define HEADER_MAIN_HPP

#include <memory>
#include <optional>
#include <cassert>
#include <iostream>
#include <algorithm>

template <typename Key, typename Value>
class node {
public:
    std::shared_ptr<node> left;
    std::shared_ptr<node> right;
    std::weak_ptr<node> parent;

    Key key;
    Value value;

    int height;

    node(Key k, Value v);

    void update_heights();

    void check_invariants();
};

template <typename Key, typename Value>
node<Key, Value>::node(Key k, Value v) {
    key = k;
    value= v;
    height = 0;
}

template <typename Key, typename Value>
void node<Key, Value>::update_heights() {
    std::weak_ptr<node<Key, Value>> current = parent;
    while (!current.expired()) {
        std::shared_ptr<node<Key,Value>> current_locked = current.lock();
        current_locked->height = 0;
        if (current_locked->left) {
            current_locked->height = current_locked->left->height + 1;
        }
        if (current_locked->right) {
            current_locked->height =
                std::max(current_locked->height, current_locked->right->height + 1);
        }

        current = current_locked->parent;
    }
}

template <typename Key, typename Value>
void node<Key, Value>::check_invariants() {
    if (left) {
        assert(left->parent.lock().get() == this);
        left->check_invariants();
    }
    if (right) {
        assert(right->parent.lock().get() == this);
        right->check_invariants();
    }

    if (left && right) {
        // std::cout << "key: " << key << std::endl;
        // std::cout << "left: " << left->key << std::endl;
        // std::cout << "right: " << right->key << std::endl;
        // std::cout << "height: " << height << std::endl;
        // std::cout << "left height: " << left->height << std::endl;
        // std::cout << "right height: " << right->height << std::endl;
        // Doesn't pass yet due to no balancing
        // assert((height == left->height + 1) || (height == left->height + 2));
        // assert((height == right->height + 1) || (height == right->height + 2));
    }
}

template <typename Key, typename Value>
class avl_map {
private:
    std::shared_ptr<node<Key, Value>> root;

public:
    std::optional<Value> find(Key key) const;
    void insert(Key key, Value value);
    void check_invariants();
};

template <typename Key, typename Value>
std::optional<Value> avl_map<Key, Value>::find(Key key) const {
    auto current = root;

    while (current) {
        auto res = key <=> current->key;

        if (res == std::weak_ordering::less) {
            current = current->left;
        } else if (res == std::weak_ordering::greater) {
            current = current->right;
        } else {
            return current->value;
        }
    }
    return std::nullopt;
}

template <typename Key, typename Value>
void avl_map<Key, Value>::insert(Key key, Value value) {
    check_invariants();

    if (!root) {
        root = std::make_shared<node<Key, Value>>(key, value);
        return;
    }
    auto current = root;

    while (current) {
        auto res = key <=> current->key;

        if (res == std::weak_ordering::less) {
            if (current->left) {
                current = current->left;
            } else {
                current->left = std::make_shared<node<Key, Value>>(key, value);
                current->left->parent = current;
                current->left->update_heights();

                return;
            }
        } else if (res == std::weak_ordering::greater) {
            if (current->right) {
                current = current->right;
            } else {
                current->right = std::make_shared<node<Key, Value>>(key, value);
                current->right->parent = current;
                current->right->update_heights();

                return;
            }
        } else {
            current->value = value;
            return;
        }
    }
}

template <typename Key, typename Value>
void avl_map<Key, Value>::check_invariants() {
    if (root) {
        root->check_invariants();
    }
}

#endif
