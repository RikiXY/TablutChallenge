#include "core/Position.hpp"
#include <array>
#include <cstdlib>

namespace { // Functions declared here are only accessible within this file

struct ZobristKeys {
    std::array<std::array<uint64_t, 81>, 4> piece{};
    uint64_t whiteToMove = 0;
};

/**
 * @brief Generates the Zobrist keys at compile time with SplitMix64. Taken from https://prng.di.unimi.it/splitmix.c
 *
 * @return The keys
 */
constexpr ZobristKeys makeKeys() {
    uint64_t state = 0x9E3779B97F4A7C15ull;
    auto next = [&state] {
        uint64_t z = (state += 0x9E3779B97F4A7C15ull);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31);
    };
    ZobristKeys keys;
    for (Piece p : {Piece::White, Piece::Black, Piece::King})
        for (uint64_t &key : keys.piece[static_cast<int>(p)])
            key = next();
    keys.whiteToMove = next();
    return keys;
}

constexpr ZobristKeys zobrist = makeKeys();

/**
 * @brief Gets the Zobrist key of a piece on a square
 *
 * @param piece The piece
 * @param square The square
 * @return The key (always 0 for an empty square)
 */
uint64_t key(Piece piece, int square) {
    return zobrist.piece[static_cast<int>(piece)][square];
}

/**
 * @brief Generates all legal moves for a given piece
 *
 * @param moves Where to add the moves
 * @param from The square of the piece
 * @param blockers The cells the piece can't stop on or pass through
 */
void addMoves(std::vector<Move> &moves, int from, Bitboard blockers) {
    for (int direction = 0; direction < 4; direction++) {
        const int length = freeLength(from, direction, blockers);
        for (int i = 0; i < length; i++)
            moves.push_back(makeMove(from, tables.ray[from][direction][i]));
    }
}

/**
 * @brief Checks whether the king is captured by the black pawn that just moved next to it
 *
 * @param black The black pawns after the move
 * @param king The square of the king
 * @param beyond The square on the other side of the king, seen from the pawn that moved, -1 if off the board
 * @return true if the king is captured
 */
bool isKingCaptured(Bitboard black, int king, int beyond) {
    const bool onOrNextToThrone = std::abs(king / 9 - 4) + std::abs(king % 9 - 4) <= 1; // Manhattan distance between the king and the throne

    if (onOrNextToThrone)
        return (tables.neighbours[king] & ~bit(THRONE) & ~black) == 0;

    return beyond != -1 && ((black | tables.camps) & bit(beyond));
}

} // namespace

Piece Position::at(int square) const {
    const Bitboard mask = bit(square);
    if (white & mask)
        return Piece::White;
    if (black & mask)
        return Piece::Black;
    if (king & mask)
        return Piece::King;
    return Piece::Empty;
}

void Position::put(int square, Piece piece) {
    const Bitboard mask = bit(square);
    white &= ~mask;
    black &= ~mask;
    king &= ~mask;
    if (piece == Piece::White)
        white |= mask;
    else if (piece == Piece::Black)
        black |= mask;
    else if (piece == Piece::King)
        king |= mask;
}

int Position::kingSquare() const {
    return king ? lowestSquare(king) : -1;
}

Position Position::start() {
    Position position;
    position.black = tables.camps; // The black pawns in the camps
    // The white pawns in the cross around the king
    for (int i : {2, 3, 5, 6})
        position.white |= bit(i * 9 + 4) | bit(4 * 9 + i);
    position.king = bit(THRONE); // The king in the throne
    position.computeHash();
    return position;
}

void Position::calculateLegalMoves(std::vector<Move> &moves) const {
    moves.clear();
    if (outcome != Outcome::Ongoing)
        return;

    const Bitboard occupied = white | black | king;
    for (Bitboard own = sideToMove == Side::White ? white | king : black; own; own &= own - 1) {
        const int from = lowestSquare(own);
        addMoves(moves, from, occupied | forbiddenFrom(from));
    }
}

bool Position::isLegal(const Move &move) const {
    if (outcome != Outcome::Ongoing || move.from >= 81 || move.to >= 81)
        return false;
    const Bitboard own = sideToMove == Side::White ? white | king : black;
    if (!(own & bit(move.from)) || move.from == move.to)
        return false;
    int direction;
    if (move.from / 9 == move.to / 9)
        direction = move.to < move.from ? 0 : 1;
    else if (move.from % 9 == move.to % 9)
        direction = move.to < move.from ? 2 : 3;
    else
        return false;
    const int distance = std::abs(move.to - move.from) / (direction < 2 ? 1 : 9);
    return distance <= freeLength(move.from, direction, white | black | king | forbiddenFrom(move.from));
}

bool Position::hasLegalMove() const {
    if (outcome != Outcome::Ongoing)
        return false;

    const Bitboard occupied = white | black | king;
    for (Bitboard own = sideToMove == Side::White ? white | king : black; own; own &= own - 1) {
        const int from = lowestSquare(own);
        if (tables.neighbours[from] & ~(occupied | forbiddenFrom(from)))
            return true;
    }
    return false;
}

void Position::calculateTacticalMoves(std::vector<TacticalMove> &moves) const {
    moves.clear();
    if (outcome != Outcome::Ongoing)
        return;
    const Bitboard own = sideToMove == Side::White ? white | king : black;
    const Bitboard occupied = white | black | king;

    // Finds all the empty cells that are next to an opponent's piece (the only viable ones for a capture)
    Bitboard nearOpponent = 0;
    for (int direction = 0; direction < 4; direction++)
        nearOpponent |= shift(occupied & ~own, direction);
    nearOpponent &= ~occupied;

    // Keeps a candidate move only if it captures or wins
    const auto add = [&](int from, int to) {
        const Move move = makeMove(from, to);
        const MoveEffects moveEffects = effects(move);
        if (moveEffects.captured > 0 || moveEffects.outcome != Outcome::Ongoing)
            moves.push_back({move, moveEffects});
    };

    // The king also wins on the edge cells next to an opponent
    const Bitboard edgesNearOpponent = sideToMove == Side::White ? nearOpponent & tables.edges : 0;
    for (Bitboard targets = captureSquares() | edgesNearOpponent; targets; targets &= targets - 1) {
        const int to = lowestSquare(targets);
        for (int direction = 0; direction < 4; direction++) {
            // Find the first piece in this direction and the cells crossed to reach the target
            const int length = freeLength(to, direction, occupied);
            if (length == tables.rayLength[to][direction])
                continue;
            const int from = tables.ray[to][direction][length];
            const Bitboard path = bit(to) | (tables.rayMask[to][direction] & ~tables.rayMask[from][direction] & ~bit(from));
            // The piece must belong to the side to move and be able to cross every cell up to the target
            if ((own & bit(from)) && !(path & forbiddenFrom(from)))
                add(from, to);
        }
    }

    // White wins by moving the king to the edge (does not duplicate the edge cells next to an opponent)
    if (sideToMove == Side::White && king) {
        const int from = kingSquare();
        const Bitboard blockers = occupied | forbiddenFrom(from);
        for (int direction = 0; direction < 4; direction++) {
            const int length = freeLength(from, direction, blockers);
            for (int i = 0; i < length; i++) {
                const int to = tables.ray[from][direction][i];
                if ((tables.edges & bit(to)) && !(nearOpponent & bit(to)))
                    add(from, to);
            }
        }
    }
}

bool Position::hasWinningMove() const {
    if (outcome != Outcome::Ongoing || !king)
        return false;
    const int square = kingSquare();
    const Bitboard occupied = white | black | king;

    if (sideToMove == Side::White) {
        const Bitboard blockers = white | black | tables.camps | bit(THRONE);
        for (int direction = 0; direction < 4; direction++)
            if (tables.rayLength[square][direction] > 0 && !(tables.rayMask[square][direction] & blockers))
                return true;
        return false;
    }

    Bitboard targets = 0;
    if (square == THRONE || (tables.neighbours[THRONE] & king)) {
        const Bitboard missing = tables.neighbours[square] & ~(black | bit(THRONE));
        if (popcount(missing) == 1)
            targets = missing;
    } else {
        for (int direction = 0; direction < 4; direction++)
            if (shift(king, direction ^ 1) & (black | tables.camps))
                targets |= shift(king, direction);
    }

    for (targets &= ~occupied; targets; targets &= targets - 1) {
        const int to = lowestSquare(targets);
        for (int direction = 0; direction < 4; direction++) {
            const int length = freeLength(to, direction, occupied);
            if (length == tables.rayLength[to][direction])
                continue;
            const int from = tables.ray[to][direction][length];
            const Bitboard path = bit(to) | (tables.rayMask[to][direction] & ~tables.rayMask[from][direction] & ~bit(from));
            if ((black & bit(from)) && !(path & forbiddenFrom(from)))
                return true;
        }
    }
    return false;
}

Bitboard Position::captureSquares() const {
    const bool whiteToMove = sideToMove == Side::White;
    const Bitboard hostile = whiteToMove ? white | king | bit(THRONE) | (tables.camps & ~tables.campCenters) : black | bit(THRONE) | tables.camps;
    const Bitboard enemy = whiteToMove ? black : white;
    Bitboard squares = 0;
    for (int direction = 0; direction < 4; direction++)
        squares |= shift(enemy & shift(hostile, direction), direction);
    if (!whiteToMove)
        for (int direction = 0; direction < 4; direction++)
            squares |= shift(king, direction);
    return squares & ~(white | black | king);
}

MoveEffects Position::effects(const Move &move) const {
    // Simulate the server's capture rules locally on the pieces after the move
    const Bitboard moved = bit(move.from) | bit(move.to);
    Bitboard whiteAfter = white;
    Bitboard blackAfter = black;
    Bitboard kingAfter = king;
    if (king & bit(move.from))
        kingAfter ^= moved;
    else if (white & bit(move.from))
        whiteAfter ^= moved;
    else
        blackAfter ^= moved;

    const Bitboard hostileToBlack = whiteAfter | kingAfter | bit(THRONE) | (tables.camps & ~tables.campCenters);
    const Bitboard hostileToWhite = blackAfter | bit(THRONE) | tables.camps;

    MoveEffects result;
    for (int direction = 0; direction < 4; direction++) {
        const int target = tables.neighbour[move.to][direction]; // The neighbour that may be captured
        if (target == -1)
            continue;
        const int beyond = tables.neighbour[target][direction]; // The cell after the neighbour
        const bool capturesBlack = sideToMove == Side::White && (blackAfter & bit(target)) && beyond != -1 && (hostileToBlack & bit(beyond));
        const bool capturesWhite = sideToMove == Side::Black && (whiteAfter & bit(target)) && beyond != -1 && (hostileToWhite & bit(beyond));
        if (capturesBlack || capturesWhite) {
            result.capturedDirections = static_cast<uint8_t>(result.capturedDirections | 1 << direction);
            result.captured++;
        } else if (sideToMove == Side::Black && (kingAfter & bit(target)) && isKingCaptured(blackAfter, target, beyond)) {
            result.outcome = Outcome::BlackWins;
        }
    }

    if ((kingAfter & bit(move.to)) && (tables.edges & bit(move.to)))
        result.outcome = Outcome::WhiteWins;
    return result;
}

int Position::apply(const Move &move) {
    return apply(move, effects(move));
}

uint64_t Position::hashAfterQuietMove(const Move &move) const {
    const Piece piece = at(move.from);
    return hash ^ key(piece, move.from) ^ key(piece, move.to) ^ zobrist.whiteToMove;
}

int Position::apply(const Move &move, const MoveEffects &effects) {
    // Apply the move to the board
    const Piece pieceToMove = at(move.from);
    Bitboard &pieces = pieceToMove == Piece::King ? king : pieceToMove == Piece::White ? white
                                                                                       : black;
    pieces ^= bit(move.from) | bit(move.to);
    hash ^= key(pieceToMove, move.from) ^ key(pieceToMove, move.to);

    // Remove the captured pawns
    Bitboard &opponent = sideToMove == Side::White ? black : white;
    const Piece captured = sideToMove == Side::White ? Piece::Black : Piece::White;
    for (int direction = 0; direction < 4; direction++) {
        if (!(effects.capturedDirections & (1 << direction)))
            continue;
        const int square = tables.neighbour[move.to][direction];
        opponent &= ~bit(square);
        hash ^= key(captured, square);
    }

    if (effects.outcome != Outcome::Ongoing)
        outcome = effects.outcome;
    sideToMove = sideToMove == Side::White ? Side::Black : Side::White;
    hash ^= zobrist.whiteToMove;
    return effects.captured;
}

void Position::computeHash() {
    hash = sideToMove == Side::White ? zobrist.whiteToMove : 0;
    for (int square = 0; square < 81; square++)
        hash ^= key(at(square), square);
}

uint64_t perft(const Position &position, int depth) {
    std::vector<Move> moves;
    position.calculateLegalMoves(moves);
    if (depth == 1)
        return moves.size();

    uint64_t count = 0;
    for (const Move &move : moves) {
        Position next = position;
        next.apply(move);
        if (next.outcome == Outcome::Ongoing)
            count += perft(next, depth - 1);
    }
    return count;
}
