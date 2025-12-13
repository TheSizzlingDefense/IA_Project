#ifndef SPACEDREPETITIONCALCULATOR_H
#define SPACEDREPETITIONCALCULATOR_H

#include <ctime>

class SpacedRepetitionCalculator
{
public:
    SpacedRepetitionCalculator();

    void calculateNextReview(int quality);

    bool isDue() const;

    double getEasinessFactor() const {return easinessFactor; };

    int getRepetitions() const { return repetitions; };

    int getInterval() const { return interval; };

    time_t getNextReview() const { return nextReview; };

    void setEasinessFactor(double ef) { easinessFactor = ef; };

    void setRepetitions(int rep) { repetitions = rep; };

    void setInterval(int intv) { interval = intv; };

    void setNextReview(time_t next) { nextReview = next; };

private:
    double easinessFactor;
    int repetitions;
    int interval;
    time_t nextReview;
};

#endif // SPACEDREPETITIONCALCULATOR_H
