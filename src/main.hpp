
#ifndef HEADER_MAIN_HPP
#define HEADER_MAIN_HPP

#include <memory>
#include <optional>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <utility>

template <typename Key, typename Value>
class node {
public:
    Key key;
    Value value;

    std::shared_ptr<node> left;
    std::shared_ptr<node> right;

    // Even if I'm careful with pointers, let's have these as weak_ptr to be
    // on the safe side. It avoids silent memory leaks.
    std::weak_ptr<node> parent;
    std::weak_ptr<node> prev;
    std::weak_ptr<node> next;
    int height;

    node();
    node(Key k, Value v);

    static int height_maybe(std::shared_ptr<const node> node);
    static int balance(std::shared_ptr<const node> node);
    void update_height();

    // Only for invariant checking
    static void check_invariants(std::shared_ptr<const node> node);
    int calculate_height() const;
};

template <typename Key, typename Value>
node<Key, Value>::node() {
    height = 1;
}

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
    assert(node->next.lock());
    assert(node->next.lock()->prev.lock() == node);

    assert(node->prev.lock());
    assert(node->prev.lock()->next.lock() == node);
    // note that key order is checked in avl_map::check_invariants

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
    std::shared_ptr<node<Key, Value>> min;
    std::shared_ptr<node<Key, Value>> max;
    int size_;

    std::shared_ptr<node<Key, Value>>
    insert_recursive(std::shared_ptr<node<Key, Value>> current,
                     std::weak_ptr<node<Key, Value>> prev,
                     std::weak_ptr<node<Key, Value>> next,
                     Key key,
                     Value value
                     );
    std::shared_ptr<node<Key, Value>>
    remove_recursive(std::shared_ptr<node<Key, Value>> current,
                     std::shared_ptr<node<Key, Value>> prev,
                     std::shared_ptr<node<Key, Value>> next,
                     Key key
                     );

    std::shared_ptr<node<Key, Value>>
    left_rotate(std::shared_ptr<node<Key, Value>> current);
    std::shared_ptr<node<Key, Value>>
    right_rotate(std::shared_ptr<node<Key, Value>> current);

    void make_nodes_neighbors(std::shared_ptr<node<Key, Value>> prev,
                              std::shared_ptr<node<Key, Value>> next
                              );

    void check_invariants() const;

public:
    std::optional<Value> find(Key key) const;
    void insert(Key key, Value value);
    void remove(Key key);

    std::optional<std::tuple<Key, Value>> get_min() const;
    std::optional<std::tuple<Key, Value>> get_max() const;

    std::optional<std::tuple<Key, Value>> take_min();
    std::optional<std::tuple<Key, Value>> take_max();

    int size() const;

    avl_map();

    // todo:
    // compare
    // iterator
    // print map
    // print node

    class iterator {
    private:
        std::weak_ptr<node<Key, Value>> current;

    public:
        iterator(std::weak_ptr<node<Key, Value>>);

        std::tuple<Key, Value> operator*() const;
        iterator& operator++();
        bool operator!=(const iterator& other) const;
    };

    iterator begin() const;
    iterator end() const;
};

template <typename Key, typename Value>
avl_map<Key, Value>::avl_map() {
    min = std::make_shared<node<Key, Value>>();
    max = std::make_shared<node<Key, Value>>();
    make_nodes_neighbors(min, max);
    size_ = 0;

    check_invariants();
}

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
                                      std::weak_ptr<node<Key, Value>> prev,
                                      std::weak_ptr<node<Key, Value>> next,
                                      Key key,
                                      Value value
                                      ) {
    if (current == nullptr) {
        auto new_node = std::make_shared<node<Key, Value>>(key, value);
        new_node->prev = prev;
        prev.lock()->next = new_node;

        new_node->next = next;
        next.lock()->prev = new_node;

        size_++;

        return new_node;
    }

    auto res = key <=> current->key;

    if (res == std::weak_ordering::less) {
        auto new_left = insert_recursive(current->left, prev, current, key, value);
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
        auto new_right = insert_recursive(current->right, current, next, key, value);
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
    root = insert_recursive(root,
                            min,
                            max,
                            key,
                            value);
    check_invariants();
}

template <typename Key, typename Value>
void avl_map<Key, Value>::remove(Key key) {
    root = remove_recursive(root, min, max, key);
    check_invariants();
}

template <typename Key, typename Value>
std::shared_ptr<node<Key, Value>>
avl_map<Key, Value>::remove_recursive(std::shared_ptr<node<Key, Value>> current,
                                      std::shared_ptr<node<Key, Value>> prev,
                                      std::shared_ptr<node<Key, Value>> next,
                                      Key key) {
    // The case when the object to be deleted isn't part of the tree.
    if (current == nullptr) {
        make_nodes_neighbors(prev, next);
        return nullptr;
    }

    auto res = key <=> current->key;

    if (res == std::weak_ordering::less) {
        auto new_left = remove_recursive(current->left, prev, current, key);
        current->left = new_left;
        if (current->left) {
            current->left->parent = current;
        }
    } else if (res == std::weak_ordering::greater) {
        auto new_right = remove_recursive(current->right, current, next, key);
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
            current->right = remove_recursive(current->right, current, next, new_key);
            if (current->right) {
                current->right->parent = current;
            }
            current->key = new_key;
            current->value = new_value;
        } else if (current->left && !current->right) {
            current->left->parent = current->parent;
            make_nodes_neighbors(current->left, next);
            size_--;
            return current->left;
        } else if (!current->left && current->right) {
            current->right->parent = current->parent;
            make_nodes_neighbors(prev, current->right);
            size_--;
            return current->right;
        } else {
            make_nodes_neighbors(prev, next);
            size_--;
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
void avl_map<Key, Value>::make_nodes_neighbors(std::shared_ptr<node<Key, Value>> prev,
                                               std::shared_ptr<node<Key, Value>> next
                                               ) {
    prev->next = next;
    next->prev = prev;
}

template <typename Key, typename Value>
void avl_map<Key, Value>::check_invariants() const {
    if (root) {
        node<Key, Value>::check_invariants(root);
    }

    std::shared_ptr<node<Key, Value>> current = min->next.lock();

    while (current != max && current->next.lock() != max) {
        std::shared_ptr<node<Key, Value>> next = current->next.lock();

        assert(current->key < next->key);
            
        current = next;
    }
}

template <typename Key, typename Value>
std::optional<std::tuple<Key, Value>> avl_map<Key, Value>::get_min() const {
    if (root) {
        return std::make_tuple(min->next.lock()->key, min->next.lock()->value);
    } else {
        return std::nullopt;
    }
}

template <typename Key, typename Value>
std::optional<std::tuple<Key, Value>> avl_map<Key, Value>::get_max() const {
    if (root) {
        return std::make_tuple(max->prev.lock()->key, max->prev.lock()->value);
    } else {
        return std::nullopt;
    }
}

template <typename Key, typename Value>
std::optional<std::tuple<Key, Value>> avl_map<Key, Value>::take_min() {
    // This is O(log n). Maybe O(1) is possible, but for now, I take the easy way
    auto m = get_min();
    if (m) {
        remove(std::get<0>(m.value()));
    }

    return m;
}

template <typename Key, typename Value>
std::optional<std::tuple<Key, Value>> avl_map<Key, Value>::take_max() {
    auto m = get_max();
    if (m) {
        remove(std::get<0>(m.value()));
    }

    return m;
}

template <typename Key, typename Value>
int avl_map<Key, Value>::size() const {
    return size_;
}

template <typename Key, typename Value>
avl_map<Key, Value>::iterator avl_map<Key, Value>::begin() const {
    return iterator(min->next);
}

template <typename Key, typename Value>
avl_map<Key, Value>::iterator avl_map<Key, Value>::end() const {
    return iterator(max);
}

template <typename Key, typename Value>
avl_map<Key, Value>::iterator::iterator(std::weak_ptr<node<Key, Value>> c) : current(c) {

}

template <typename Key, typename Value>
std::tuple<Key, Value>
avl_map<Key, Value>::iterator::operator*() const {
    return std::make_tuple(current.lock()->key,
                           current.lock()->value);
}

template <typename Key, typename Value>
avl_map<Key, Value>::iterator&
avl_map<Key, Value>::iterator::operator++() {
    current = current.lock()->next;
    return *this;
}

// template <typename Key, typename Value>
// bool
// avl_map<Key, Value>::iterator::operator!=(const iterator& other) const {

// }

#endif
