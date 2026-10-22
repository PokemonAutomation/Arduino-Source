/*  Text Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_TextMatcher_H
#define PokemonAutomation_CommonTools_OCR_TextMatcher_H

#include <string>
#include <set>
#include <map>
#include "OCR_StringMatchResult.h"

namespace PokemonAutomation{
namespace OCR{


//  Calculate Levenshtein (edit) distance between two strings.
//  This is the minimum number of single-character edits (insertions,
//  deletions, substitutions)
//  needed to transform string x into string y.
template <typename StringType>
size_t levenshtein_distance(const StringType& x, const StringType& y);

//  Calculate minimum edit distance to find 'substring' anywhere within
//  'fullstring'.
//  This is a variant of Levenshtein distance that allows the match to
//  start/end anywhere
//  in fullstring without penalty.
//
//  Example:
//    substring = "cat"
//    fullstring = "the cat ran"
//    Result: 0 (exact substring match found)
//
//    substring = "cet"
//    fullstring = "the cat ran"
//    Result: 1 (one substitution: e->a)
template <typename StringType>
size_t levenshtein_distance_substring(const StringType& substring, const StringType& fullstring);

//  Calculate weighted Levenshtein (edit) distance between match_target and
//  input, with a per-pair substitution cost and a per-character cost for the
//  part of input that lies outside the matched region (pre/postfix).
//
//  match_target is the string being looked for (e.g. a dictionary token);
//  input is the raw OCR'ed string, typically longer.
//
//  Edit costs:
//    - Substitute input[i] with match_target[j]:
//      substitution_cost(input[i], match_target[j]) (identical characters
//      cost 0; the function is not called for them).
//    - Delete a character of input / insert a character of match_target
//      inside the matched region: 1.0 each.
//    - Skip a character of input before/after the matched region
//      (pre/postfix): prepostfix_cost each (0.0 = free, i.e. substring
//      match; 1.0 = full string). Values above 1.0 behave like 1.0, since a
//      skipped character can always be charged as a 1.0 deletion instead.
//
//  The whole of match_target must be explained; only input may have skipped
//  context. All costs must be non-negative. The result is the minimum total
//  cost, in the same units as the costs (so fractional, when the costs are
//  fractional).
//
//  Example:
//    input = "xxcatxx", match_target = "cat", prepostfix_cost = 0.25,
//    unit substitution
//    Result: 1.0 (the four context characters, 0.25 each)
//
//    input = "kitten", match_target = "sitten", prepostfix_cost = 1.0,
//    substitution_cost('k', 's') = 0.25, 1.0 otherwise
//    Result: 0.25

double levenshtein_distance(
    double prepostfix_cost,
    double (*substitution_cost)(char32_t ch0, char32_t ch1),
    const std::u32string& match_target, const std::u32string& input
);

//  Calculate probability that a match of 'matched' characters out of 'total'
//  occurred by random chance.
//  This uses the binomial cumulative distribution function (CDF) to determine
//  statistical significance.
//  Mathematically equivalent to:
//      BinomialCDF[k=matched, n=total, p=random_match_chance]
//
//  This is the probability of getting 'matched' characters correct by pure
//  luck.
//
//  Parameters:
//    total: Total length of the dictionary token being compared
//    matched: Number of characters that matched (total - levenshtein_distance)
//    random_match_chance: Probability a random character matches
//    (language-dependent)
//
//  Returns: Probability value in range [0, 1]. Lower values indicate
//  better/more significant matches.
//
//  Example:
//    total=6, matched=6, random_match_chance=0.05 => very low probability
//    (strong match)
//    total=6, matched=3, random_match_chance=0.05 => higher probability (weak
//    match)
//
//  This probability is later converted to log10(p) for numerical range and
//  used as a score to measure
//  match closeness.
double random_match_probability(size_t total, size_t matched, double random_match_chance);


//  Core proximity matching algorithm for OCR results against a dictionary.
//  This function matches OCR'd text against a database of known strings,
//  handling OCR errors
//  using Levenshtein edit distance and statistical probability scoring.
//
//  Parameters:
//    database: Map of normalized UTF-32 strings to their token identifiers
//    (slugs)
//    random_match_chance: Probability that a character matches by pure chance
//    (language-dependent)
//    text: Raw OCR text to match
//    log10p_spread: Maximum log10(probability) difference to keep multiple
//    candidates
//
//  Returns: StringMatchResult with candidates sorted by match probability
//
//  MATCHING ALGORITHM:
//  1. Normalize the OCR text (remove spaces, convert to lowercase, standardize
//  characters)
//  2. Try exact match first - if found, return immediately with high confidence
//  3. For each dictionary entry:
//     a. Calculate Levenshtein distance (min edit operations:
//     insert/delete/substitute)
//     b. Compute "matched" characters = token_length - distance
//     c. Calculate probability this match occurred by random chance using
//     binomial distribution
//     d. Convert to log10(probability) for numerical stability (lower = better
//     match)
//     e. Add all slugs associated with this dictionary entry at this score
//
//  Limitation for East Asian characters:
//  The Levenshtein distance treats all character substitutions equally (cost =
//  1) regardless
//  of different character complexity (e.g. 霹 is very close to 霸 but different
//  from 火, but
//  get the same cost).
StringMatchResult match_substring(
    const std::map<std::u32string, std::set<std::string>>& database, double random_match_chance,
    const std::string& text, double log10p_spread
);




}
}
#endif
