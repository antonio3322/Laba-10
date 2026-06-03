#include <iostream>
#include "deque.h"

int main() {
    deque<deque<int>> d;

    for (int i = 0; i < 100; ++i) {
        deque<int> sd2;

        for (int j = 0; j < 100; ++j) {
            sd2.push_front(j + i * 100);
        }

        d.push_front(sd2);
    }

    std::reverse(d.begin(), d.end());

    for (deque<int>& _d : d) {
        std::reverse(_d.begin(), _d.end());

        for (int x : _d) {
            std::cout << x << " ";
        }

        std::cout << "\n";
    }
}
