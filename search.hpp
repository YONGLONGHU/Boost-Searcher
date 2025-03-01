#pragma once 
#include"index.hpp"
#include<boost/algorithm/string.hpp>
#include<algorithm>
#include"util.hpp"
#include<jsoncpp/json/json.h>
namespace ns_searcher
{
  struct InvertedElemPrint
  {
    uint64_t doc_id;
    int weight;
    std::vector<std::string>words;
    InvertedElemPrint():doc_id(0),weight(0){}
  };
  class Searcher
  {
    private:
      ns_index::Index* index;
    public:
      Searcher(){}
      ~Searcher(){}
    public:
      void InitSearcher(const std::string& input)
      {
        //1.创建index对象
        index = ns_index::Index::GetInstance();
        //std::cout<<"获取index单例成功......"<<std::endl;
        LOG(NORMAL,"获取Index单例成功......");
        //2.创建索引
        index->BuildIndex(input);
        std::cout<<"建立正排和倒排索引成功......"<<std::endl;
      }
      void Search(const std::string& query,std::string* json_string)
      {
        //1.对query进行分词处理
        std::vector<std::string> words;
        ns_util::JiebaUtil::CutString(query,&words);
        //2.根据各个词，进行index查找
        std::unordered_map<uint64_t,InvertedElemPrint> tokens_map;
       // ns_index::InvertedList inverted_list_all;
        std::vector<InvertedElemPrint> inverted_list_all;
        for(std::string word:words)
        {
          boost::to_lower(word);
          ns_index::InvertedList* inverted_list = index->GetInvertedList(word);
          if(nullptr == inverted_list)
            continue;
          for(const auto& elem:*inverted_list)
          {
            auto& item = tokens_map[elem.doc_id];
            item.doc_id = elem.doc_id;
            item.weight += elem.weight;
            item.words.push_back(elem.word);
          }
         // inverted_list_all.insert(inverted_list_all.end(),inverted_list->begin(),inverted_list->end());
        }
        for(const auto& item:tokens_map)
        {
          inverted_list_all.push_back(std::move(item.second));
        }
        //3.汇总结果，根据相关性降序排序
       // std::sort(inverted_list_all.begin(),inverted_list_all.end(),[](const ns_index::InvertedElem &e1,const ns_index::InvertedElem& e2){
          // return e1.weight>e2.weight;
           // });
         std::sort(inverted_list_all.begin(),inverted_list_all.end(),\
             [](const InvertedElemPrint& e1,const InvertedElemPrint& e2){
             return e1.weight>e2.weight;
             });
        //4.根据结果构建json串
      
        Json::Value root;
        for(auto& item :inverted_list_all)
        {
          ns_index::DocInfo* doc = index->GetForwardIndex(item.doc_id);
          if(nullptr == doc)
            continue;

        Json::Value elem;
        elem["title"] = doc->title;
        elem["desc"] = GetDesc(doc->content,item.words[0]);
        elem["url"] = doc->url;

        root.append(elem);
      }

        //Json::StyledWriter writer;
        Json::FastWriter writer;
      *json_string = writer.write(root);
  }
      std::string GetDesc(const std::string& html_content,const std::string& word)
      {
        //找到word在html_content中的首次出现，往前找50字节，没有从begin开始，往后找100字节
        //1.找到首次出现
        const int  prev_step = 50;
        const int  next_step = 100;
        auto iter = std::search(html_content.begin(),html_content.end(),word.begin(),word.end(),[](int x,int y){
            return (std::tolower(x) == std::tolower(y));
            });
        if(iter == html_content.end())
        {
          return "None";
        }
        int pos = std::distance(html_content.begin(),iter);
        //2.获取start和end的位置
        int start = 0;
        int end = html_content.size()-1;
        if(pos > start + prev_step)
          start = pos - prev_step;
        if(pos < end - next_step)
          end = pos + next_step;
        //3.截取字串
        if(start > end) return "None";
        std::string desc =  html_content.substr(start,end-start);
        desc += "...";
        return desc;
      }
};
}
