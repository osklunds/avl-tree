
#ifndef HEADER_MAIN_HPP
#define HEADER_MAIN_HPP

#include <memory>
#include <optional>

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
};


template <typename Key, typename Value>
node<Key, Value>::node(Key k, Value v) {
    key = k;
    value= v;
    height = 0;
}

template <typename Key, typename Value>
class avl_map {
private:
    std::shared_ptr<node<Key, Value>> root;

public:
    std::optional<Value> find(Key key) const;
    void insert(Key key, Value value);
};

template <typename Key, typename Value>
std::optional<Value> avl_map<Key, Value>::find(Key key) const {
    auto current = root;

    while (current) {
        auto res = current->key <=> key;

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
    if (!root) {
        root = std::make_shared<node<Key, Value>>(key, value);
        return;
    }
    auto current = root;

    while (current) {
        auto res = current->key <=> key;

        if (res == std::weak_ordering::less) {
            if (current->left) {
                current = current->left;
            } else {
                current->left = std::make_shared<node<Key, Value>>(key, value);
                current->left->parent = current;
                return;
            }
        } else if (res == std::weak_ordering::greater) {
            if (current->right) {
                current = current->right;
            } else {
                current->right = std::make_shared<node<Key, Value>>(key, value);
                current->right->parent = current;
                return;
            }
        } else {
            current->value = value;
            return;
        }
    }
}

#endif
