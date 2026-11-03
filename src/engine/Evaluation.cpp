#include "engine/Evaluation.hpp"

namespace {

constexpr int whitePawnValue = 100;
constexpr int blackPawnValue = 100;

} // namespace

int evaluate(const Position &position) {
    int whiteScore = 0;
    for (const auto &row : position.board) {
        for (Piece piece : row) {
            if (piece == Piece::White)
                whiteScore += whitePawnValue;
            else if (piece == Piece::Black)
                whiteScore -= blackPawnValue;
        }
    }
    return position.sideToMove == Side::White ? whiteScore : -whiteScore;
}
