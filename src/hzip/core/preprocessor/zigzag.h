#ifndef HYBRIDZIP_ZIGZAG_H
#define HYBRIDZIP_ZIGZAG_H

#include <cstdint>
#include <iterator>

namespace hztrans {
    class ZigZagTransformer {
    private:
        uint32_t width{};
        uint32_t height{};
    public:
        struct zigzag_pair {
            uint32_t x{};
            uint32_t y{};

            inline void next(uint32_t _width, uint32_t _height) {
                uint32_t ymax = _height - 1;
                uint32_t xmax = _width - 1;

                if (x >= xmax && y >= ymax) {
                    x = _width;
                    y = _height;
                    return;
                }

                if ((x + y) & 1) {
                    if (y == ymax) {
                        x++;
                        return;
                    }

                    if (x == 0) {
                        y++;
                        return;
                    }

                    y++;
                    x--;
                } else {
                    if (x == xmax) {
                        y++;
                        return;
                    }

                    if (y == 0) {
                        x++;
                        return;
                    }

                    x++;
                    y--;
                }
            }

            bool operator==(const zigzag_pair &rhs) const {
                return x == rhs.x && y == rhs.y;
            }

            bool operator!=(const zigzag_pair &rhs) const {
                return x != rhs.x || y != rhs.y;
            }
        };

        ZigZagTransformer(uint32_t width, uint32_t height) {
            this->width = width;
            this->height = height;
        }

        class const_iterator {
        public:
            typedef const_iterator self_type;
            typedef zigzag_pair value_type;
            typedef int difference_type;
            typedef std::forward_iterator_tag iterator_category;

            uint32_t width{};
            uint32_t height{};
            uint32_t x{};
            uint32_t y{};

            const_iterator(value_type val, uint32_t width, uint32_t height) : _val(val) {
                this->width = width;
                this->height = height;
            }

            self_type operator++() {
                self_type i = *this;
                _val.next(width, height);
                this->x = _val.x;
                this->y = _val.y;
                return i;
            }

            self_type operator++(int) {
                _val.next(width, height);
                this->x = _val.x;
                this->y = _val.y;
                return *this;
            }

            bool operator==(const self_type &rhs) { return _val == rhs._val; }

            bool operator!=(const self_type &rhs) { return _val != rhs._val; }

            self_type operator*() {
                return *this;
            }

        private:
            value_type _val;
        };

        [[nodiscard]] const_iterator begin() const {
            return const_iterator(zigzag_pair(), width, height);
        }

        [[nodiscard]] const_iterator end() const {
            return const_iterator(zigzag_pair{.x=width, .y=height}, width, height);
        }
    };
}

#endif
