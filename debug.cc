#include"search.hpp"
#include<cstdio>
#include<iostream>
#include<string>
#include<cstring>
const std::string input = "data/raw_html/raw.txt";
int main()
{
  ns_searcher::Searcher* search = new ns_searcher::Searcher();
  search->InitSearcher(input);
  char buffer[1024];
  std::string query;
  std::string json_string;
  while(true)
  {
    std::cout<<"please enter your search query:";
    fgets(buffer,sizeof(buffer)-1,stdin);
    buffer[strlen(buffer)-1] = 0;
    query = buffer;
    search->Search(query,&json_string);
    std::cout<<json_string<<std::endl;
  }
  return 0;
}
