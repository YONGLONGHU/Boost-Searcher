#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<unordered_map>
#include<mutex>
#include"util.hpp"
#include"log.hpp"
namespace ns_index
{
  struct DocInfo
  {
    std::string title;
    std::string content;
    std::string url;
    uint64_t doc_id;
  };
  struct InvertedElem 
  {
    uint64_t doc_id;
    std::string word;
    int weight;
  };
  //倒排拉链
  typedef std::vector<InvertedElem> InvertedList;
  class Index 
  {
    private:
      std::vector<DocInfo> forward_index;//正排索引，数据结构用数组，下标是天然文档ID
      //倒排索引，一个关键字和一组InvertedElem对应
      std::unordered_map<std::string,InvertedList > inverted_index;
    private:
      Index(){}
      Index(const Index&) = delete;
      Index& operator=(const Index&) = delete;
      static Index* instance;
      static std::mutex mtx;
    public:
      ~Index(){}
    public:
     static Index* GetInstance()
      {
        if(nullptr == instance)
        {
        mtx.lock();
        if(nullptr == instance)
        {
          instance = new Index();
        }
        mtx.unlock();
        }
        return instance;
      }
      //根据文件id找到文档内容
      DocInfo* GetForwardIndex(uint64_t doc_id)
      {
        if(doc_id>=forward_index.size())
        {
          std::cerr<<"doc_id out range,error!"<<std::endl;
          return nullptr;
       }
        return &forward_index[doc_id];
      }
      //根据关键字获得倒排拉链
      InvertedList* GetInvertedList(const std::string& word)
      {
        auto iter = inverted_index.find(word);
        if(iter == inverted_index.end())
        {
          std::cerr<<word <<"have no InvertedList"<<std::endl;
          return nullptr; 
        }
        return &(iter->second);
      }
      //去标签格式化后的文档，构建正排和倒排索引
      bool BuildIndex(const std::string& input )//parse处理好的数据交给我
      {
        std::ifstream in(input,std::ios::in|std::ios::binary);
        if(!in.is_open())
        {
          std::cerr<<"sorry, "<<input<<"open error"<<std::endl;
          return false; 
        }
        std::string line;
        int count = 0;
        while(std::getline(in,line))
        {
          //建立正排索引
          DocInfo* doc = BuildForwardIndex(line);
          if(nullptr == doc)
          {
            std::cerr<<"build "<<line<<" error"<<std::endl;
            continue;
          }
          //建立倒排索引
         BuildInvertedIndex(*doc);
         count++;
         if(count%50==0)
           //std::cout<<"当前已经建立的索引文档： "<<count<<std::endl;
           LOG(NORMAL,"当前的已经建立的索引文档"+std::to_string(count));
        }
        return true;
      }
    private:
      DocInfo* BuildForwardIndex(const std::string& line)
      {
        //1.解析line,字符串切分
        std::vector<std::string>results;
        const std::string sep = "\3";
        ns_util::StringUtil::CutString(line,&results,sep);
        if(results.size()!=3)
        {
          return nullptr;
        }
        //2.字符串填充到DocInfo
        DocInfo doc;
        doc.title = results[0];
        doc.content = results[1];
        doc.url = results[2];
        doc.doc_id = forward_index.size(); 
        //3.插入到正排索引vector
        forward_index.push_back(std::move(doc));
        return &forward_index.back();
      }
      bool BuildInvertedIndex(const DocInfo &doc)
      {
        //统计关键字出现次数
        struct word_cnt
        {
          int title_cnt;
          int content_cnt;
          word_cnt():title_cnt(0),content_cnt(0){}
        };
        //构建关键字与出现次数之间的映射
        std::unordered_map<std::string,word_cnt> word_map;
        std::vector<std::string> title_words;
        ns_util::JiebaUtil::CutString(doc.title,&title_words);
        for(std::string s:title_words)
        {
          boost::to_lower(s);
          word_map[s].title_cnt++;
        }
        //构建内容关键字与出现次数之间的映射
        std::vector<std::string> content_words;
        ns_util::JiebaUtil::CutString(doc.content,&content_words);
        for(std::string s:content_words)
        {
          boost::to_lower(s);
          word_map[s].content_cnt++;
        }
#define X 10
#define Y 1
        for(auto& word_pair:word_map)
        {
          InvertedElem item;
          item.doc_id = doc.doc_id;
          item.word = word_pair.first;
          item.weight = X*word_pair.second.title_cnt+Y*word_pair.second.content_cnt;
          InvertedList& inverted_list = inverted_index[word_pair.first];
          inverted_list.push_back(std::move(item));
        }
        return true;
      }
  };
   Index* Index::instance = nullptr;
   std::mutex Index::mtx;
}
