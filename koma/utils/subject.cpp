#include "subject.hpp"

void koma::Subject::add_observer(koma::Observer *observer) {
    this->observers_.insert(observer);
}

void koma::Subject::remove_observer(koma::Observer *observer) {
    this->observers_.erase(observer);
}

void koma::Subject::notify_observers(std::string event) {
    for (koma::Observer* observer : this->observers_) {
        observer->receive_event(event);
    }
}
