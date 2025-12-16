#include "spacedrepetitioncalculator.h"
#include <algorithm>

SpacedRepetitionCalculator::SpacedRepetitionCalculator()
    : easinessFactor(2.5)
    , repetitions(0)
    , interval(0)
    , nextReview(time(nullptr)){

}

void SpacedRepetitionCalculator::calculateNextReview(int quality) {
    quality = std::max(0, std::min(5, quality));

    easinessFactor = easinessFactor + (0.1 - (5 - quality) * (0.08 + (5 - quality) * 0.02));

    if (easinessFactor < SpacedRepetitionCalculator::MIN_EF) {
        easinessFactor = SpacedRepetitionCalculator::MIN_EF;
    }
    if (easinessFactor > SpacedRepetitionCalculator::MAX_EF) {
        easinessFactor = SpacedRepetitionCalculator::MAX_EF;
    }

    if (quality < 3) {
        repetitions = 0;
        interval = 1;
    } else {
        repetitions++;

        if (repetitions == 1) {
            interval = 1;
        } else if (repetitions == 2) {
            interval = 6;
        } else {
            interval = static_cast<int>(interval * easinessFactor);
        }
    }

    nextReview = time(nullptr) + (interval * 24 * 3600);
}
