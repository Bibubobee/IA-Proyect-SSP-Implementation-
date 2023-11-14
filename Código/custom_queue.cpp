// Instancia 24
// 15:19 INICIO
// 15:35 ( ES MUY MUY LENTO EL TABU SEARCH, CADA ITERACION)
#include <queue>
#include <deque>
#include <iostream>

template <typename T, int MaxLen, typename Container=std::deque<T>>
class FixedQueue : public std::queue<T, Container> {
public:
    typedef typename Container::iterator iterator;
    typedef typename Container::const_iterator const_iterator;

    iterator begin() { return this->c.begin(); }
    iterator end() { return this->c.end(); }
    const_iterator begin() const { return this->c.begin(); }
    const_iterator end() const { return this->c.end(); }

    void push(const T& value) {
        if (this->size() == MaxLen) {
           this->c.pop_front();
        }
        std::queue<T, Container>::push(value);
    }
};