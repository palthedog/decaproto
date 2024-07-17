#ifndef DECAPROTO_FIELD_H
#define DECAPROTO_FIELD_H

#include <memory>

namespace decaproto {

class Message;

// A smart pointer that holds a pointer to a decaproto Message.
// So that we can copy Message instance easily, it provides a copy constructor
// and operator= which copies the instance of the submessage, not the pointer.
template <typename T>
class SubMessagePtr {
    std::shared_ptr<T> ptr_;

public:
    SubMessagePtr() : ptr_(nullptr) {
    }

    SubMessagePtr(T* ptr) : ptr_(ptr) {
    }

    ~SubMessagePtr() {
    }

    // Copy the instance of SubMessage.
    SubMessagePtr(const SubMessagePtr<T>& other) {
        if (other.ptr_) {
            ptr_.reset(new T(*other.ptr_));
        } else {
            ptr_.reset();
        }
    }

    // operator= also copies the instance of SubMessage.
    const SubMessagePtr<T>& operator=(const SubMessagePtr<T>& other) {
        if (other.ptr_) {
            ptr_.reset(new T(*other.ptr_));
        } else {
            ptr_.reset();
        }
        return *this;
    }

    T* get() {
        if (!ptr_) {
            resetDefault();
        }
        return ptr_.get();
    }

    void resetDefault() {
        ptr_.reset(new T());
    }

    void reset(T* ptr = nullptr) {
        ptr_.reset(ptr);
    }

    void clear() {
        ptr_.reset();
    }

    T* operator->() {
        if (!ptr_) {
            resetDefault();
        }

        return ptr_.get();
    }

    const T* operator->() const {
        if (!ptr_) {
            resetDefault();
        }

        return ptr_.get();
    }

    T& operator*() {
        if (!ptr_) {
            resetDefault();
        }

        return *ptr_;
    }

    const T& operator*() const {
        if (!ptr_) {
            resetDefault();
        }

        return *ptr_;
    }

    operator bool() const {
        return ptr_ != nullptr;
    }

    bool operator!() const {
        return !static_cast<bool>(*this);
    }
};

}  // namespace decaproto

#endif
