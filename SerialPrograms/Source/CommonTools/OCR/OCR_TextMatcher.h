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
#include <QString>
#include "OCR_RawOCR.h"
#include "OCR_StringMatchResult.h"

namespace PokemonAutomation{
namespace OCR{


// Calculate Levenshtein (edit) distance between two strings.
// This is the minimum number of single-character edits (insertions, deletions, substitutions)
// needed to transform string x into string y.
size_t levenshtein_distance(const QString& x, const QString& y);

// Calculate minimum edit distance to find 'substring' anywhere within 'fullstring'.
// This is a variant of Levenshtein distance that allows the match to start/end anywhere
// in fullstring without penalty.
//
// Example:
//   substring = "cat"
//   fullstring = "the cat ran"
//   Result: 0 (exact substring match found)
//
//   substring = "cet"
//   fullstring = "the cat ran"
//   Result: 1 (one substitution: e->a)
size_t levenshtein_distance_substring(const QString& substring, const QString& fullstring);

// Calculate Levenshtein (edit) distance between two strings.
// This is the minimum number of single-character edits (insertions, deletions, substitutions)
// needed to transform string x into string y.
template <typename StringType>
size_t levenshtein_distance(const StringType& x, const StringType& y);

// Calculate minimum edit distance to find 'substring' anywhere within 'fullstring'.
// This is a variant of Levenshtein distance that allows the match to start/end anywhere
// in fullstring without penalty.
//
// Example:
//   substring = "cat"
//   fullstring = "the cat ran"
//   Result: 0 (exact substring match found)
//
//   substring = "cet"
//   fullstring = "the cat ran"
//   Result: 1 (one substitution: e->a)
template <typename StringType>
size_t levenshtein_distance_substring(const StringType& substring, const StringType& fullstring);

// Calculate probability that a match of 'matched' characters out of 'total' occurred by random chance.
// This uses the binomial cumulative distribution function (CDF) to determine statistical significance.
// Mathematically equivalent to:
//     BinomialCDF[k=matched, n=total, p=random_match_chance]
//
// This is the probability of getting 'matched' characters correct by pure luck.
//
// Parameters:
//   total: Total length of the dictionary token being compared
//   matched: Number of characters that matched (total - levenshtein_distance)
//   random_match_chance: Probability a random character matches (language-dependent)
//
// Returns: Probability value in range [0, 1]. Lower values indicate better/more significant matches.
//
// Example:
//   total=6, matched=6, random_match_chance=0.05 => very low probability (strong match)
//   total=6, matched=3, random_match_chance=0.05 => higher probability (weak match)
//
// This probability is later converted to log10(p) for numerical range and used as a score to measure
// match closeness.
double random_match_probability(size_t total, size_t matched, double random_match_chance);


// Core proximity matching algorithm for OCR results against a dictionary.
// This function matches OCR'd text against a database of known strings, handling OCR errors
// using Levenshtein edit distance and statistical probability scoring.
//
// Parameters:
//   database: Map of normalized UTF-32 strings to their token identifiers (slugs)
//   random_match_chance: Probability that a character matches by pure chance (language-dependent)
//   text: Raw OCR text to match
//   log10p_spread: Maximum log10(probability) difference to keep multiple candidates
//
// Returns: StringMatchResult with candidates sorted by match probability
//
// MATCHING ALGORITHM:
// 1. Normalize the OCR text (remove spaces, convert to lowercase, standardize characters)
// 2. Try exact match first - if found, return immediately with high confidence
// 3. For each dictionary entry:
//    a. Calculate Levenshtein distance (min edit operations: insert/delete/substitute)
//    b. Compute "matched" characters = token_length - distance
//    c. Calculate probability this match occurred by random chance using binomial distribution
//    d. Convert to log10(probability) for numerical stability (lower = better match)
//    e. Add all slugs associated with this dictionary entry at this score
//
// Limitation for East Asian characters:
// The Levenshtein distance treats all character substitutions equally (cost = 1) regardless
// of different character complexity (e.g. 霹 is very close to 霸 but different from 火, but
// get the same cost).
StringMatchResult match_substring(
    const std::map<std::u32string, std::set<std::string>>& database, double random_match_chance,
    const std::string& text, double log10p_spread
);




}
}
#endif
