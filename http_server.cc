#include"cpp-httplib/httplib.h"
#include"search.hpp"
const std::string root_path = "./wwwroot";
const std::string input = "data/raw_html/raw.txt";
int main()
{
  ns_searcher::Searcher search;
  search.InitSearcher(input);

  httplib::Server svr;
  svr.set_base_dir(root_path.c_str());
  svr.Get("/s",[&search](const httplib::Request& req, httplib::Response& rsp){
  //    rsp.set_content("hello world.","text/plain;charset=utf-8");
        if(!req.has_param("word"))
        {
        rsp.set_content("必须要有关键字才能搜索！","text/plain;charset=utf-8");
        return ;
        }
        std::string word = req.get_param_value("word");
        std::cout<<"用户正在搜索： "<<word<<std::endl;
        std::string json_string;
        search.Search(word,&json_string);
        rsp.set_content(json_string,"application/json");
      });
  svr.listen("0.0.0.0",8083);
  return 0;
}
