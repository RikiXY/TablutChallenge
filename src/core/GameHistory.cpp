#include "core/GameHistory.hpp"
#include <algorithm>

bool GameHistory::push(const Position &after, bool captured) {
    if (captured) // If a capture occurred, the previous positions are no longer possible to repeat and therefore useless to keep track of
        sinceLastCapture.clear();

    const auto seen = after.outcome == Outcome::Ongoing
                          ? std::count(sinceLastCapture.begin(), sinceLastCapture.end(), after.hash)
                          : 0;

    sinceLastCapture.push_back(after.hash);
    return seen > allowedRepetitions;
}
