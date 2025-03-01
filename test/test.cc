#include<iostream>
#include<vector>
#include<string>
#include<jsoncpp/json/json.h>
int main()
{
  Json::Value root;
  Json::Value iteml;
  iteml["key1"] = "value1";
  iteml["key2"] = "value2";

  Json::Value item2;
  item2["key1"] = "value1";
  item2["key2"] = "value2";
  root.append(iteml);
  root.append(item2);

  Json::StyledWriter writer;
  std::string s = writer.write(root);
  std::cout<< s <<std::endl;
  return 0;
}
