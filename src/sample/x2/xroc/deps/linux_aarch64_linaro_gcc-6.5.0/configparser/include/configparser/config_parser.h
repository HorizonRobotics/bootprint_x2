/**
 * Copyright 2017 <Copyright hobot>
 * @brief parse config
 * @file format:
     [secction]
      key1 = value1
      key2 = value2
      key3 = value3
      key3 = value4
 * @author dc_neo(yan01.zhang@)
 * @date  12/July/2017
*/
#ifndef CONFIGPARSER_INCLUDE_CONFIGPARSER_CONFIG_PARSER_H_
#define CONFIGPARSER_INCLUDE_CONFIGPARSER_CONFIG_PARSER_H_

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

namespace hobotconfigparser {

const int RET_OK = 0;
const int RET_ERR = -1;
// every item has key = value & comment
struct IniItem {
  std::string key;
  std::string value;
  std::string comment;
};

struct IniSection {
  typedef std::vector<IniItem>::iterator iterator;
  iterator begin() { return items.begin(); }
  iterator end() { return items.end(); }
  std::string name;
  std::string comment;
  std::vector<IniItem> items;
};

// IniFile class
class IniFile {
 public:
  IniFile();
  ~IniFile() { release(); }

 public:
  typedef std::unordered_map<std::string, IniSection *>::iterator iterator;

  iterator begin() { return _sections.begin(); }
  iterator end() { return _sections.end(); }

 public:
  // open file  named fname;
  int load(const std::string &fname);

  /*获取section段第一个键为key的值,并返回其std::string型的值*/
  std::string getStringValue(const std::string &section, const std::string &key,
                             int &ret);
  /*获取section段第一个键为key的值,并返回其int型的值*/
  int getIntValue(const std::string &section, const std::string &key, int &ret);
  /*获取section段第一个键为key的值,并返回其double型的值*/
  double getDoubleValue(const std::string &section, const std::string &key,
                        int &ret);

  /*获取section段第一个键为key的值,并将值赋到value中*/
  int getValue(const std::string &section, const std::string &key,
               std::string &value);
  /*获取section段第一个键为key的值,并将值赋到value中,将注释赋到comment中*/
  int getValue(const std::string &section, const std::string &key,
               std::string &value, std::string &comment);

  /*获取section段所有键为key的值,并将值赋到values的vector中*/
  int getValues(const std::string &section, const std::string &key,
                std::vector<std::string> &values);
  /*获取section段所有键为key的值,并将值赋到values的vector中*/
  /*将注释赋到comments的vector中*/
  int getValues(const std::string &section, const std::string &key,
                std::vector<std::string> &value,
                std::vector<std::string> &comments);

  // 是否有section
  bool hasSection(const std::string &section);
  bool hasKey(const std::string &section, const std::string &key);
  // get all section
  void getSections(std::vector<std::string> &sectionname);
  // get all section excpet s1
  void getSections(std::vector<std::string> &sectionname,
                   const std::string &s1);
  // for dubug
  void print();

 private:
  IniSection *getSection(const std::string &section = "");
  /*去掉str后面的c字符*/
  void trimleft(std::string &str, char c);
  /*去掉str前面的c字符*/
  void trimright(std::string &str, char c);
  /*去掉str前面和后面的空格符,Tab符等空白符*/
  void trim(std::string &str);

  void release();
  int getline(std::string &str, FILE *fp);
  bool isComment(const std::string &str);
  bool parse(const std::string &content, std::string &key, std::string &value);

 private:
  std::unordered_map<std::string, IniSection *> _sections;
  std::string _fname;
  std::vector<char> _flags;
};
}  // namespace hobotconfigparser

#endif  // CONFIGPARSER_INCLUDE_CONFIGPARSER_CONFIG_PARSER_H_
