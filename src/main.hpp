
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

    int balance();
    int left_height();
    int right_height();
    void update_height();

    void check_invariants();
    int calculate_height();
};

template <typename Key, typename Value>
node<Key, Value>::node(Key k, Value v) {
    key = k;
    value= v;
    height = 1;
}

template <typename Key, typename Value>
int node<Key, Value>::balance() {
    return left_height() - right_height();
}

template <typename Key, typename Value>
int node<Key, Value>::left_height() {
    if (left) {
        return left->height;
    } else {
        return 0;
    }
}

template <typename Key, typename Value>
int node<Key, Value>::right_height() {
    if (right) {
        return right->height;
    } else {
        return 0;
    }
}

template <typename Key, typename Value>
void node<Key, Value>::update_height() {
    height = 1 + std::max(left_height(), right_height());
}

template <typename Key, typename Value>
void node<Key, Value>::check_invariants() {
    // std::cout << "key: " << key << std::endl;
    // std::cout << "oskar: " << height << std::endl;
    // std::cout << "oskar: " << calculate_height() << std::endl;


    if (left) {
        assert(left->parent.lock().get() == this);
        left->check_invariants();
    }
    if (right) {
        assert(right->parent.lock().get() == this);
        right->check_invariants();
    }

    // Check that heights are stored correctly
    // Wasteful to calculate for each node for each recursive call
    // to check_invariants(), but it seems to be fast enough.
    assert(height == calculate_height());

    // assert(balance() == -1 ||
    //        balance() == 0 ||
    //        balance() == 1
    //        );

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
int node<Key, Value>::calculate_height() {
    int h = 0;
    if (left) {
        h = left->calculate_height();
    }
    if (right) {
        h = std::max(h, right->calculate_height());
    }

    return h+1;
}




template <typename Key, typename Value>
class avl_map {
private:
    std::shared_ptr<node<Key, Value>> root;
    std::shared_ptr<node<Key, Value>>
    insert_recursive(std::shared_ptr<node<Key, Value>> current,
                     Key key,
                     Value value
                     );

    std::shared_ptr<node<Key, Value>>
    left_rotate(std::shared_ptr<node<Key, Value>> current);
    std::shared_ptr<node<Key, Value>>
    right_rotate(std::shared_ptr<node<Key, Value>> current);

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
std::shared_ptr<node<Key, Value>>
avl_map<Key, Value>::insert_recursive(
                                      std::shared_ptr<node<Key, Value>> current,
                                      Key key,
                                      Value value
                                      ) {
    if (current == nullptr) {
        return std::make_shared<node<Key, Value>>(key, value);
    }

    auto res = key <=> current->key;

    if (res == std::weak_ordering::less) {
        auto new_left = insert_recursive(current->left, key, value);
        current->left = new_left;
        current->left->parent = current;
        current->update_height();
        new_left->update_height();

        if (current->balance() > 1) {
            return right_rotate(current);
        } else {
            return current;
        }
    } else if (res == std::weak_ordering::greater) {
        auto new_right = insert_recursive(current->right, key, value);
        current->right = new_right;
        current->right->parent = current;
        current->update_height();
        new_right->update_height();

        if (current->balance() < -1) {
            return left_rotate(current);
        }
    } else {
        current->value = value;
    }
    current->update_height();
    // std::cout << "key: " << current->key << std::endl;
    // std::cout << "oskar: " << current->left_height() << std::endl;
    // std::cout << "oskar: " << current->right_height() << std::endl;
    return current;
}

//            current
//   a              new_right
//                temp      b 
//----------------------------------
//           new_right
//     current           b
//  a       temp
template <typename Key, typename Value>
std::shared_ptr<node<Key, Value>>
avl_map<Key, Value>::left_rotate(std::shared_ptr<node<Key, Value>> current) {
    auto new_right = current->right;
    auto temp = new_right->left;
    new_right->left = current;
    new_right->left->parent = new_right;

    current->right = temp;
    if (current->right) {
        current->right->parent = current;
    }

    current->update_height();
    new_right->update_height();

    return new_right;
}

//            current
//   new_left          b
// a       temp
//----------------------------------
//           new_left
//        a          current
//                temp      b
template <typename Key, typename Value>
std::shared_ptr<node<Key, Value>>
avl_map<Key, Value>::right_rotate(std::shared_ptr<node<Key, Value>> current) {
    auto new_left = current->left;
    auto temp = new_left->right;
    new_left->right = current;
    new_left->right->parent = new_left;

    current->left = temp;
    if (current->left) {
        current->left->parent = current;
    }

    current->update_height();
    new_left->update_height();

    return new_left;
}

template <typename Key, typename Value>
void avl_map<Key, Value>::insert(Key key, Value value) {
    root = insert_recursive(root, key, value);
    check_invariants();
}

template <typename Key, typename Value>
void avl_map<Key, Value>::check_invariants() {
    if (root) {
        root->check_invariants();
    }
}

#endif
