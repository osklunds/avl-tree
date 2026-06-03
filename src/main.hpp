
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
    Key key;
    Value value;

    std::shared_ptr<node> left;
    std::shared_ptr<node> right;
    std::weak_ptr<node> parent;
    int height;

    node(Key k, Value v);

    static int height_maybe(std::shared_ptr<const node> node);
    static int balance(std::shared_ptr<const node> node);
    void update_height();

    // Only for invariant checking
    static void check_invariants(std::shared_ptr<const node> node);
    int calculate_height() const;
};

template <typename Key, typename Value>
node<Key, Value>::node(Key k, Value v) {
    key = k;
    value= v;
    height = 1;
}

template <typename Key, typename Value>
int node<Key, Value>::height_maybe(std::shared_ptr<const node> node) {
    if (node) {
        return node->height;
    } else {
        return 0;
    }
}

template <typename Key, typename Value>
int node<Key, Value>::balance(std::shared_ptr<const node> node) {
    if (node) {
        return height_maybe(node->left) - height_maybe(node->right);
    } else {
        return 0;
    }
}

template <typename Key, typename Value>
void node<Key, Value>::update_height() {
    height = 1 + std::max(height_maybe(left), height_maybe(right));
}

template <typename Key, typename Value>
void node<Key, Value>::check_invariants(std::shared_ptr<const node> node) {
    if (node->left) {
        assert(node->left->parent.lock() == node);
        assert(node->left->key < node->key);
        check_invariants(node->left);
    }
    if (node->right) {
        assert(node->right->parent.lock() == node);
        assert(node->right->key > node->key);
        check_invariants(node->right);
    }

    // Check that heights are stored correctly
    assert(node->height == node->calculate_height());

    assert(balance(node) == -1 ||
           balance(node) == 0 ||
           balance(node) == 1
           );
}

template <typename Key, typename Value>
int node<Key, Value>::calculate_height() const {
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
    remove_recursive(std::shared_ptr<node<Key, Value>> current,
                     Key key
                     );

    std::shared_ptr<node<Key, Value>>
    left_rotate(std::shared_ptr<node<Key, Value>> current);
    std::shared_ptr<node<Key, Value>>
    right_rotate(std::shared_ptr<node<Key, Value>> current);

public:
    std::optional<Value> find(Key key) const;
    void insert(Key key, Value value);
    void remove(Key key);
    void check_invariants() const;

    // todo:
    // compare
    // iterator
    // min/max
};

// todo: consider to merge with insert/delete
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

        if (node<Key, Value>::balance(current) > 1) {
            if (key < current->left->key) {
                return right_rotate(current);
            } else {
                current->left = left_rotate(current->left);
                return right_rotate(current);
            }
        } else {
            return current;
        }
    } else if (res == std::weak_ordering::greater) {
        auto new_right = insert_recursive(current->right, key, value);
        current->right = new_right;
        current->right->parent = current;
        current->update_height();
        new_right->update_height();

        if (node<Key, Value>::balance(current) < -1) {
            if (key > current->right->key) {
                return left_rotate(current);
            } else {
                current->right = right_rotate(current->right);
                return left_rotate(current);
            }
        }
    } else {
        current->value = value;
    }
    current->update_height();
    return current;
}

//            old_top
//   a              new_top
//                temp      b 
//----------------------------------
//           new_top
//     old_top           b
//  a       temp
template <typename Key, typename Value>
std::shared_ptr<node<Key, Value>>
avl_map<Key, Value>::left_rotate(std::shared_ptr<node<Key, Value>> old_top) {
    auto new_top = old_top->right;
    assert(new_top);
    auto temp = new_top->left;
    new_top->left = old_top;
    new_top->left->parent = new_top;

    old_top->right = temp;
    if (old_top->right) {
        old_top->right->parent = old_top;
    }

    old_top->update_height();
    new_top->update_height();

    return new_top;
}

//            old_top
//   new_top          b
// a       temp
//----------------------------------
//           new_top
//        a          old_top
//                temp      b
template <typename Key, typename Value>
std::shared_ptr<node<Key, Value>>
avl_map<Key, Value>::right_rotate(std::shared_ptr<node<Key, Value>> old_top) {
    auto new_top = old_top->left;
    assert(new_top);
    auto temp = new_top->right;
    new_top->right = old_top;
    new_top->right->parent = new_top;

    old_top->left = temp;
    if (old_top->left) {
        old_top->left->parent = old_top;
    }

    old_top->update_height();
    new_top->update_height();

    return new_top;
}

template <typename Key, typename Value>
void avl_map<Key, Value>::insert(Key key, Value value) {
    root = insert_recursive(root, key, value);
    check_invariants();
}

template <typename Key, typename Value>
void avl_map<Key, Value>::remove(Key key) {
    root = remove_recursive(root, key);
    check_invariants();
}

template <typename Key, typename Value>
std::shared_ptr<node<Key, Value>>
avl_map<Key, Value>::remove_recursive(std::shared_ptr<node<Key, Value>> current,
                                      Key key) {
    if (current == nullptr) {
        return nullptr;
    }

    auto res = key <=> current->key;

    if (res == std::weak_ordering::less) {
        auto new_left = remove_recursive(current->left, key);
        current->left = new_left;
        if (current->left) {
            current->left->parent = current;
        }
    } else if (res == std::weak_ordering::greater) {
        auto new_right = remove_recursive(current->right, key);
        current->right = new_right;
        if (current->right) {
            current->right->parent = current;
        }
    } else {
        if (current->left && current->right) {
            // change current value to the smallest value in the right subtree,
            // then delete that value from the right subtree.
            auto smallest_right = current->right;
            while (true) {
                if (smallest_right->left) {
                    smallest_right = smallest_right->left;
                } else {
                    break;
                }
            }

            auto new_key = smallest_right->key;
            auto new_value = smallest_right->value;
            current->right = remove_recursive(current->right, new_key);
            if (current->right) {
                current->right->parent = current;
            }
            current->key = new_key;
            current->value = new_value;
        } else if (current->left && !current->right) {
            current->left->parent = current->parent;
            return current->left;
        } else if (!current->left && current->right) {
            current->right->parent = current->parent;
            return current->right;
        } else {
            return nullptr;
        }
    }

    current->update_height();

    if (node<Key, Value>::balance(current) > 1) {
        if (node<Key, Value>::balance(current->left) >= 0) {
            return right_rotate(current);
        } else {
            current->left = left_rotate(current->left);
            return right_rotate(current);
        }
    }

    if (node<Key, Value>::balance(current) < -1) {
        if (node<Key, Value>::balance(current->right) <= 0) {
            return left_rotate(current);
        } else {
            current->right = right_rotate(current->right);
            return left_rotate(current);
        }
    }



    return current;
}

template <typename Key, typename Value>
void avl_map<Key, Value>::check_invariants() const {
    if (root) {
        node<Key, Value>::check_invariants(root);
    }
}

#endif
