#include "include/piece.hpp"

namespace engine {

Color getOppositeColor(Color color) {
    return Color(1 - color);
}

} // namespace engine