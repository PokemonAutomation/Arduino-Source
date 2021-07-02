/*  Text Inference Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <vector>
#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/Exception.h"
#include "StringNormalization.h"
#include "TextMatcher.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace OCR{

void print(const std::vector<size_t>& v){
    std::string str = "{";
    bool first = true;
    for (size_t x : v){
        if (!first){
            str += ", ";
        }
        first = false;
        str += std::to_string(x);
    }
    str += "}";
    cout << str << endl;
}


size_t levenshtein_distance(const QString& x, const QString& y){
    size_t xlen = x.size();
    size_t ylen = y.size();

    std::vector<size_t> v0(ylen + 1);
    std::vector<size_t> v1(ylen + 1);

    for (size_t i = 0; i <= ylen; i++){
        v0[i] = i;
    }

    for (size_t i = 0; i < xlen; i++){
        v1[0] = i + 1;

        for (size_t j = 0; j < ylen; j++){
            size_t delete_cost = v0[j + 1] + 1;
            size_t insert_cost = v1[j] + 1;
            size_t sub_cost = v0[j];
            if (x[(int)i] != y[(int)j]){
                sub_cost += 1;
            }
            v1[j + 1] = std::min({delete_cost, insert_cost, sub_cost});
        }

        std::swap(v0, v1);
    }

    return v0[ylen];
}
size_t levenshtein_distance_substring(const QString& substring, const QString& fullstring){
    size_t xlen = fullstring.size();
    size_t ylen = substring.size();

    std::vector<size_t> v0(ylen + 1);
    std::vector<size_t> v1(ylen + 1);

    for (size_t i = 0; i <= ylen; i++){
        v0[i] = i;
    }
//    print(v0);

    size_t min = ylen;

    for (size_t i = 0; i < xlen; i++){
        v1[0] = 0;

        for (size_t j = 0; j < ylen; j++){
            size_t delete_cost = v0[j + 1] + 1;
            size_t insert_cost = v1[j] + 1;
            size_t sub_cost = v0[j];
            if (fullstring[(int)i] != substring[(int)j]){
                sub_cost += 1;
            }
            v1[j + 1] = std::min({delete_cost, insert_cost, sub_cost});
        }

        std::swap(v0, v1);

        min = std::min(min, v0[ylen]);
//        print(v0);
    }

    return min;
}


std::map<size_t, std::vector<uint64_t>> binomial_table;
SpinLock binomial_lock;
std::vector<uint64_t> binomial_row_u64(size_t degree){
    std::vector<uint64_t> row;

    uint64_t a = (uint64_t)degree;
    uint64_t b = 1;
    {
        row.emplace_back(1);
    }
    {
        row.emplace_back(a);
        a--;
        b++;
    }
    while (a > b){
        row.emplace_back(row.back() * a / b);
        a--;
        b++;
    }

    return row;
}
uint64_t binomial_coefficient_u64(size_t degree, size_t index){

    if (degree > 62){
        PA_THROW_StringException("Cannot go beyond degree 62.");
    }

    SpinLockGuard lg(binomial_lock, "binomial_coefficient_u64()");

    auto iter = binomial_table.find(degree);
    std::vector<uint64_t>& row = iter != binomial_table.end()
        ? iter->second
        : binomial_table[degree] = binomial_row_u64(degree);

    if (index > degree / 2){
        index = degree - index;
    }

    return row[index];
}


double random_match_probability(size_t total, size_t matched, double random_match_chance){
    double c_match = 1 - random_match_chance;

    double misses[62];
    {
        double miss = 1;
        misses[0] = miss;
        for (size_t c = 1; c <= total - matched; c++){
            miss *= c_match;
            misses[c] = miss;
        }
    }

    double hits = 1;

    size_t m = 0;
    for (; m < matched; m++){
        hits *= random_match_chance;
    }

    double probability = 0;
    for (; m < total; m++){
        double binomial = (double)binomial_coefficient_u64(total, m);
        probability += hits * binomial * misses[total - m];
        hits *= random_match_chance;
    }
    probability += hits;
    return probability;
}






void MatchResult::log(Logger* logger, const QString& extra) const{
    if (logger == nullptr){
        return;
    }

    QString str = "OCR Result: ";

    if (!expected_token.empty()){
        str += "Expected (";
        str += expected_token.c_str();
        str += "): ";
    }

    str += "\"";
    for (QChar ch : ocr_text){
        if (ch != '\r' && ch != '\n'){
            str += ch;
        }
    }
    str += "\" -> ";
    str += "\"" + normalized_text + "\" -> ";

    QString candidate_str;
    if (candidates.size() > 5){
        candidate_str += "(" + QString::number(candidates.size()) + " candidates)";
    }else{
        candidate_str += "(";
        for (size_t c = 0; c < candidates.size(); c++){
            candidate_str += "\"" + candidates[c] + "\"";
            if (c + 1 < candidates.size()){
                candidate_str += ", ";
            }
        }
        candidate_str += ")";
    }

    QString token_str;
    if (tokens.size() > 5){
        token_str += "(" + QString::number(candidates.size()) + " matches)";
    }else{
        token_str += "(";
        bool first = true;
        for (const std::string& token : tokens){
            if (!first){
                token_str += ", ";
            }
            first = false;
            token_str += "\"" + QString(token.c_str()) + "\"";
        }
        token_str += ")";
    }

    str += candidate_str;
    str += ": ";
    str += token_str;
//    str += " (error = " + QString::number(exact_match_error) + ")";
    str += " (alpha = " + QString::number(alpha) + ")";

    if (!extra.isEmpty()){
        str += " ===> ";
        str += extra;
    }

    logger->log(str, matched ? Qt::blue : Qt::red);
}




MatchResult match_substring(
    const std::map<QString, std::set<std::string>>& database,
    const QString& text,
    double random_match_chance,
    double min_alpha
){
    MatchResult result;
    result.ocr_text = text;
    result.normalized_text = normalize(text);

    const QString& normalized = result.normalized_text;

    //  Search for exact match of candidate.
    auto iter = database.find(normalized);
    if (iter != database.end()){
        result.matched = true;
//        result.exact_match_error = 0;
        result.alpha = 1. / random_match_probability(normalized.size(), normalized.size(), random_match_chance);
        result.candidates = {normalized};
        result.tokens = iter->second;
        return result;
    }


    bool exact_substring_match = false;
//    double best_error = 1.0;
    double best_alpha = 0;
    std::vector<QString> candidates;
    std::set<std::string> tokens;

    for (auto item : database){
        double token_length = item.first.size();

//        double error = (double)levenshtein_distance(item.first, normalized);
//        error /= token_length;

        size_t distance = levenshtein_distance_substring(item.first, normalized);
        size_t matched = token_length - distance;
        double alpha = 1. / random_match_probability(token_length, matched, random_match_chance);

//        double alpha = (token_length - (double)distance) / std::sqrt(token_length);

        if (distance == 0){
            exact_substring_match = true;
        }

#if 0
        if (error < 1.0 && best_error > error){
            best_error = error;
            candidates.clear();
            candidates.emplace_back(item.first);
            tokens = item.second;
        }else if (best_error < 1.0 && best_error == error){
            candidates.emplace_back(item.first);
            tokens.insert(item.second.begin(), item.second.end());
        }
#else
        if (alpha > 2.0 && best_alpha < alpha){
//            best_error = error;
            best_alpha = alpha;
            candidates.clear();
            candidates.emplace_back(item.first);
            tokens = item.second;
        }else if (alpha > 2.0 && best_alpha == alpha){
            candidates.emplace_back(item.first);
            tokens.insert(item.second.begin(), item.second.end());
        }
#endif
    }
    result.matched = exact_substring_match || best_alpha >= min_alpha;
//    result.exact_match_error = best_error;
    result.alpha = best_alpha;
    result.candidates = candidates;
    result.tokens = tokens;
    return result;
}








}
}

