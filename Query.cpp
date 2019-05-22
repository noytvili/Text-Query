/* Doriya Spielman 313466625 */
/* Noy Tvili 308426790 */

#include "Query.h"
#include "TextQuery.h"
#include <memory>
#include <set>
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <regex>
using namespace std;
////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<QueryBase> QueryBase::factory(const string& s) {
  regex regex_WORD("^\\s*([\\w']+)\\s*$");
  regex regex_AND("^\\s*([\\w']+)\\s+AND\\s+([\\w']+)\\s*$");
  regex regex_OR("^\\s*([\\w']+)\\s+OR\\s+([\\w']+)\\s*$");
  regex regex_NOT("^\\s*NOT\\s+([\\w']+)\\s*$");
  regex regex_N("^\\s*([\\w']+)\\s+(\\d+)\\s+([\\w']+)\\s*$");
  
  if (regex_match(s, regex_WORD)){   //word
    return std::shared_ptr<QueryBase>(new WordQuery(s));
  }
  
  else if (regex_match(s, regex_AND)){   // word1 AND word2
    istringstream iss(s);
    vector <string> s_Words;
    string w;
    while(iss >> w){
      s_Words.push_back(w);
    }
    return std::shared_ptr<QueryBase> (new AndQuery(s_Words[0], s_Words[2]));
  }
  
  else if (regex_match(s, regex_OR)){   // word1 OR word2
    istringstream iss(s);
    vector <string> s_Words;
    string w;
    while(iss >> w){
      s_Words.push_back(w);
    }
    return std::shared_ptr<QueryBase> (new OrQuery(s_Words[0], s_Words[2]));
  }
  
 else if (regex_match(s, regex_N)){   // word1 n word2
    istringstream iss(s);
    vector <string> s_Words;
    string w;
    while(iss >> w){
      s_Words.push_back(w);
    }
    int num = stoi(s_Words[1]);
    return std::shared_ptr<QueryBase> (new NQuery(s_Words[0], s_Words[2], num));
  }
  
 else if (regex_match(s, regex_NOT)){   // word1 NOT word2
    istringstream iss(s);
    vector <string> s_Words;
    string w;
    while(iss >> w){
      s_Words.push_back(w);
    }
    return std::shared_ptr<QueryBase> (new NotQuery(s_Words[1]));
  }
  
  else 
    throw std::invalid_argument( "Unrecognized search" );
}
////////////////////////////////////////////////////////////////////////////////
QueryResult NotQuery::eval(const TextQuery &text) const
{
  QueryResult result = text.query(query_word);
  auto ret_lines = std::make_shared<std::set<line_no>>();
  auto beg = result.begin(), end = result.end();
  auto sz = result.get_file()->size();
  
  for (size_t n = 0; n != sz; ++n)
  {
    if (beg==end || *beg != n)
		ret_lines->insert(n);
    else if (beg != end)
		++beg;
  }
  return QueryResult(rep(), ret_lines, result.get_file());
    
}

QueryResult AndQuery::eval (const TextQuery& text) const
{
  QueryResult left_result = text.query(left_query);
  QueryResult right_result = text.query(right_query);
  
  auto ret_lines = std::make_shared<std::set<line_no>>();
  std::set_intersection(left_result.begin(), left_result.end(),
      right_result.begin(), right_result.end(), 
      std::inserter(*ret_lines, ret_lines->begin()));

  return QueryResult(rep(), ret_lines, left_result.get_file());
}

QueryResult OrQuery::eval(const TextQuery &text) const
{
  QueryResult left_result = text.query(left_query);
  QueryResult right_result = text.query(right_query);
  
  auto ret_lines = 
      std::make_shared<std::set<line_no>>(left_result.begin(), left_result.end());

  ret_lines->insert(right_result.begin(), right_result.end());

  return QueryResult(rep(), ret_lines, left_result.get_file());
}
/////////////////////////////////////////////////////////



QueryResult NQuery::eval(const TextQuery &text) const {
  int left_indx;
  regex words_regex("[\\w']+");
  QueryResult result = AndQuery::eval(text);
  auto it= result.begin();
  auto ret_lines = std::make_shared<std::set<line_no>>();
  string word;
  string word1;
  string word2;
  vector <string> words;
  while(it!=result.end()){
    string l = result.get_file()->at(*it);
    istringstream line(l);
    while(line >> word){
      words.push_back(word);
    }
    for (int i = 0; i < word.size(); i++) { 
      auto start_words = sregex_iterator(words[i].begin(), words[i].end(), words_regex);
			smatch match = *start_words;
			word1 = match.str();
			if((word1==left_query) || (word1==right_query) ){
			  left_indx=i;
			  break;
			}
    }

			for (int i = 0; i < words.size(); i++) { //for on the right
        auto start_words1 = sregex_iterator(words[i].begin(), words[i].end(), words_regex);
			  smatch match1 = *start_words1;
			  word2 = match1.str();	
			  if(((word1 == left_query) && (word2 == right_query)) || ((word2 == left_query) && (word1 == right_query))){
          if((i-left_indx-1) <= dist){
            ret_lines->insert(*it);
			    }
			}
    
  }
    ++it;
  }
 
   return QueryResult(rep(), ret_lines, result.get_file());
}
/////////////////////////////////////////////////////////
