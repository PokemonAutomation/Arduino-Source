/*  PkmnLib Matchup
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "PokemonSwSh_PkmnLib_Matchup.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{


double calc_average_damage(
    const std::vector<const Pokemon*>& attackers,
    const std::vector<const Pokemon*>& defenders,
    const Field& field, bool multipleTargets
){
    // we need to iterate through the attackers and defenders to calculate the damage done on the list

    if (attackers.empty() || defenders.empty()){
        return 0.0;
    }

    double totalDamage = 0.0;
    size_t count = 0;
    double subTotalDamage = 0.0;

    for (const Pokemon* attacker : attackers){
        for (const Pokemon* defender : defenders){
            // we're now iterating through attackers and defenders!
            subTotalDamage = 0.0;
            // so we need to go through the moves we have and calculate the damage they can do
            size_t numMoves = attacker->num_moves();

#if 0
            //  Assume the other players pick random moves.
            for (size_t ii = 0; ii < numMoves; ii++){
                subTotalDamage += damage_score(*attacker, *defender, ii, field, multipleTargets);
            }
            subTotalDamage /= numMoves;
#else
            //  Assume the other players pick the most damaging move.
            for (size_t ii = 0; ii < numMoves; ii++){
                subTotalDamage = std::max(subTotalDamage, damage_score(*attacker, *defender, ii, field, multipleTargets));
            }
#endif

            totalDamage += subTotalDamage;
            count++;
        }
    }

    // then return the average damage
    return totalDamage / count;
}

double calc_move_score(
    const Pokemon& attacker, Pokemon defender,
    const std::vector<const Pokemon*>& teammates,
    size_t moveIdx, const Field& field
){
    // this function is different than the one above
    // it will give a score for a single move based on several different factors

    // get the move object based on the move index

    // first start by calculating damage based on the attacker
    // no on multiple targets since we're only hitting the boss
    // TODO: set defender to dynamax?
    double damageScore = damage_score(attacker, defender, moveIdx, field, false) / 2.0;

    // TODO: make sure the defender and attacker aren't in the teammates list

    // fudge factor is available based on AI decisions
    double fudgeFactor = 1.5;
    std::vector<const Pokemon*> tempDefenderList{&defender};
    // calculate the damage the teammates do against the boss
    damageScore += (1.5 * fudgeFactor) * calc_average_damage(teammates, tempDefenderList, field, false);

    // TODO: implement status moves contributions, since all NonVolatile ones are pretty good

    // estimate potential received damage
    double receivedDamage = 0.0;
    double receivedRegularDamage = 0.0;
    double receivedMaxMoveDamage = 0.0;
    double maxMoveProbability = 0.3; // TODO: we need hard data for this guy eventually
    size_t defenderNumMoves = defender.num_moves();

    // get the attacker move
    const Move& attackerMove = attacker.move(moveIdx);

    bool originalDefenderDynamax = defender.is_dynamax();
    // first calculate damage from regular moves
    defender.set_is_dynamax(false);
    // replace it
    tempDefenderList[0] = &defender;

    //  Disable the dmax HP bonus for this calculation. This actively hurts
    //  multiplayer mode where other players can dmax.
//    double dmax_hp_ratio = attacker.is_dynamax() ? 2.0 : 1.0;
    double dmax_hp_ratio = 1.0;

    // iterate through defender moves for non-dynamax
    for (size_t ii = 0; ii < defenderNumMoves; ii++){
        const Move& defenderMove = defender.move(ii);
        // NOTE: original function in python also checked to make sure we aren't dynamax, we already did that
        if (defenderMove.is_spread()){
            if (attackerMove != "wide-guard" || attacker.is_dynamax()){
                receivedRegularDamage += damage_score(defender, attacker, ii, field, true) / defenderNumMoves;
                receivedRegularDamage += 3 * calc_average_damage(tempDefenderList, teammates, field, true) / defenderNumMoves;
            }
        }else{
            receivedRegularDamage += 0.25 * damage_score(defender, attacker, ii, field, false) / dmax_hp_ratio / defenderNumMoves;
            receivedRegularDamage += 0.75 * calc_average_damage(tempDefenderList, teammates, field, false) / defenderNumMoves;
        }
    }
//    cout << "receivedRegularDamage = " << receivedRegularDamage << endl;

    // then set up for max moves
    defender.set_is_dynamax(true);
    tempDefenderList[0] = &defender;
    for (size_t ii = 0; ii < defenderNumMoves; ii++){
        receivedMaxMoveDamage += 0.25 * damage_score(defender, attacker, ii, field, false) / dmax_hp_ratio / defenderNumMoves;
        receivedMaxMoveDamage += 0.75 * calc_average_damage(tempDefenderList, teammates, field, false) / defenderNumMoves;
    }
//    cout << "receivedMaxMoveDamage = " << receivedMaxMoveDamage << endl;

    // at the end restore defender dynamax state
    defender.set_is_dynamax(originalDefenderDynamax);

    receivedDamage = receivedRegularDamage * (1 - maxMoveProbability) + receivedMaxMoveDamage * maxMoveProbability;

    // failsafe in case received damage is very small (or zero!), don't want to blow it up to infinity
    if (receivedDamage < 0.0001){
        return 1.0;
    }

    double score = damageScore / receivedDamage;
//    cout << attackerMove.name() << ":\t" << score << " - " << damageScore << " / " << receivedDamage << endl;
    return score;
}

void select_best_move(
    const Pokemon& attacker, const Pokemon& defender, const Field& field,
    const std::vector<const Pokemon*>& teammates,
    size_t& bestIndex, std::string& bestMoveName, double& bestMoveScore
){
    // by default best score should be small, so we can only grow
    bestMoveScore = 0;
    bestIndex = 0;
    bestMoveName = "";

    double score = 0.0;

    // now iterate through the moves
    for (size_t ii = 0; ii < attacker.num_moves(); ii++){
        if (attacker.pp(ii) > 0){
            score = calc_move_score(attacker, defender, teammates, ii, field);
            if (score > bestMoveScore){
                bestIndex = ii;
                bestMoveScore = score;
                bestMoveName = attacker.move(ii).name();
            }
        }
    }
}


double evaluate_matchup(
    Pokemon attacker, const Pokemon& boss,
    const std::vector<const Pokemon*>& teammates,
    uint8_t numLives
){
    // TODO: assert that the lives should be between 1 and 4

    // start by creating a new field object that's empty
    Field baseField;
    baseField.set_default_field(boss.name());

    if (attacker.name() == "ditto"){
        attacker = boss;
    }

    // calculate scores for base and DA versions of the attacker
    // start by yoinking out the DMax variable
    bool originalDMaxState = attacker.is_dynamax();
    // update attacker to normal
    attacker.set_is_dynamax(false);

    // then send it through our base version score
    std::string bestMoveName;
    double bestMoveScore;
    size_t bestIndex;
    // get the best moves and score, we're going to only use move score
    select_best_move(attacker, boss, baseField, teammates, bestIndex, bestMoveName, bestMoveScore);

    // then do the dynamax version
    attacker.set_is_dynamax(true);
    double bestDMaxMoveScore;
    select_best_move(attacker, boss, baseField, teammates, bestIndex, bestMoveName, bestDMaxMoveScore);

    // return the attacker back to original dmax state
    attacker.set_is_dynamax(originalDMaxState);

    // now for the score between the two!
    double score = std::max(bestMoveScore, (bestMoveScore + bestDMaxMoveScore) / 2.0);

    // calculate an hp correction based on number of lives
    double hpCorrection = (double)((5 - numLives) * attacker.current_hp() + numLives - 1) / 4.0;

    // then return the score multiplied by the correction
    return score * hpCorrection;
}


double evaluate_average_matchup(
    const Pokemon& attacker, const std::vector<const Pokemon*>& bosses,
    const std::vector<const Pokemon*>& teammates, uint8_t numLives
){
    // if there are no bosses, we return 1
    if (bosses.empty()){
        return 1.0;
    }

    double totalScore = 0.0;

    // iterate through the bosses and get the score
    for (const Pokemon* boss : bosses){
        totalScore += evaluate_matchup(attacker, *boss, teammates, numLives);
    }

    // return average
    return totalScore / bosses.size();
}


double get_weighted_score(double rentalScore, double rentalWeight, double bossScore, double bossWeight){
    return std::pow(
        std::pow(rentalScore, rentalWeight) * std::pow(bossScore, bossWeight),
        (1 / (rentalWeight + bossWeight))
    );
}





}
}
}
}
