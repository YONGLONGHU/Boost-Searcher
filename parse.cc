#include<iostream>
#include<string>
#include<vector>
#include<boost/filesystem.hpp>
#include"util.hpp"
//路径，存放原始数据及处理后的数据
const std::string src_path="data/input";
const std::string raw="data/raw_html/raw.txt";
typedef struct DocInfo
{
  std::string title;
  std::string content;
  std::string url;
}DocInfo_t;
//const&:输入
//*:输出
//&：既是输入，也是输出
bool EnumFile(const std::string& src_path,std::vector<std::string>* file_list);
bool ParseHtml(const std::vector<std::string>& file_list,std::vector<DocInfo_t>*results);
bool SaveHtml(const std::vector<DocInfo_t>& results,const std::string& raw);

int main()
{
  std::vector<std::string> file_list;
  //第一步：递归式的把每个文件名带路径，保存到files_list中，方便后期读取
  if(!EnumFile(src_path,&file_list))
  {
    std::cerr<<"enum file name error!"<<std::endl;
    return 1;
  }
  //第二步：按照file_list读取每个文件的内容并进行解析
  std::vector<DocInfo_t> results;
  if(!ParseHtml(file_list,&results))
  {
    std::cerr<<"parse html error"<<std::endl;
    return 2;
  }
  //第三步：把解析完毕的各个文件内容写入到raw当中,按照\3作为每个文档的分隔符
  if(!SaveHtml(results,raw))
  {
    std::cerr<<"sava html error"<<std::endl;
    return 3;
  }
  return 0;
}

bool EnumFile(const std::string& src_path,std::vector<std::string>* file_list)
{
  namespace fs=boost::filesystem;
  fs::path root_path(src_path);
  //判断路径是否存在
  if(!fs::exists(root_path))
  {
    std::cerr<<src_path<<"not exists"<<std::endl;
    return false;
  }
  //定义一个空的迭代器，用于判断递归结束
  fs::recursive_directory_iterator end;
  for(fs::recursive_directory_iterator iter(root_path);iter!=end;iter++)
  {
    //判断文件是否都是.html普通文件
    if(!fs::is_regular_file(*iter))
      continue;
    if(iter->path().extension()!=".html")//判断文件路径名是否符合要求
      continue;
    //std::cout<<"debug: "<<iter->path().string()<<std::endl;
    file_list->push_back(iter->path().string());//将所有带路径的html保存在file_list方便后续文本分析
  }
  return true;
}

static bool ParseTitle(const std::string& file,std::string* title)
{
  std::size_t begin=file.find("<title>");
  if(begin==std::string::npos)
  {
    return false;
  }
  std::size_t end=file.find("</title>");
  if(end==std::string::npos)
  {
    return false;
  }
  begin+=std::string("<title>").size();
  if(begin>end)
  {
    return false;
  }
  *title=file.substr(begin,end-begin);
  return true;
}

static bool ParseContent(const std::string& file,std::string* content)
{
  //去标签，基于一个简单的状态机
  enum status
  {
    LABEL,
    CONTENT
  };
  enum status s=LABEL;
  for(char c:file)
  {
    switch(s)
    {
      case LABEL:
        if(c=='>')
          s=CONTENT;
        break;
      case CONTENT:
        if(c=='<')s=LABEL;
        else 
        {
          if(c=='\n') c=' ';
          content->push_back(c);
        }
        break;
      default:
        break;
    }
  }
  return true;
}

static bool ParseUrl(const std::string& file_path,std::string* url)
{
  std::string url_head="https://www.boost.org/doc/libs/1_87_0/doc/html";
  std::string url_tail=file_path.substr(src_path.size());
  *url=url_head+url_tail;
  return true;
}
void ShowDoc(const DocInfo_t& doc)
{
  std::cout<<"title: "<<doc.title<<std::endl;
  std::cout<<"content: "<<doc.content<<std::endl;
  std::cout<<"url: "<<doc.url<<std::endl;
}
bool ParseHtml(const std::vector<std::string>& file_list,std::vector<DocInfo_t>*results)
{
  for(const std::string&file:file_list)
  {
    //1.读取文件
    std::string result;
    if(!ns_util::FileUtil::ReadFile(file,&result))
    {
      continue;
    }
    //2.解析指定的文件，提取title
    DocInfo_t doc;
    if(!ParseTitle(result,&doc.title))
    {
      continue;
    }
    //3.解析指定的文件，提取content
    if(!ParseContent(result,&doc.content))
    {
      continue;
    }
    //4.解析文件路径，构建url
    if(!ParseUrl(file,&doc.url))
    {
      continue;
    }
    results->push_back(std::move(doc));
    //   ShowDoc(doc));
  }
  return true;
}

bool SaveHtml(const std::vector<DocInfo_t>& results,const std::string& raw)
{
#define SEP '\3';
  //按照二进制方式写入
  std::ofstream out(raw,std::ios::out|std::ios::binary);
  if(!out.is_open())
  {
    std::cerr<<"open "<<raw<<"failed "<<std::endl;
    return false;
  }
  for(auto& item:results)
  {
    std::string out_string;
    out_string=item.title;
    out_string+=SEP;
    out_string+=item.content;
    out_string+=SEP;
    out_string+=item.url;
    out_string+='\n';

    out.write(out_string.c_str(),out_string.size());
  }
  return true;
}
