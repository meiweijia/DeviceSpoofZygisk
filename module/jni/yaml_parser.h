#ifndef YAML_PARSER_H
#define YAML_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <utility>

struct Entry
{
    std::string package_name;
    std::string template_name;
    std::map<std::string, std::string> fields;
};

class YamlParser
{
public:
    // 加载模板文件
    bool load_templates(const std::string &filename);

    // 加载目标文件
    bool load_targets(const std::string &filename);

    // 查找目标条目（带模板合并）
    Entry find_by_package(const std::string &package_name) const;

    // 检查目标是否存在
    bool contains_package(const std::string &package_name) const;

    // 格式验证方法
    bool is_valid_format() const;

private:
    std::vector<Entry> templates_;
    std::vector<Entry> targets_;
    std::vector<std::string> lines_;

    // 解析相关方法
    void parse_yaml(std::vector<Entry> &output);
    int get_indent(const std::string &line) const;
    std::string trim(const std::string &s) const;
    std::pair<std::string, std::string> parse_key_value(const std::string &line) const;
    void parse_fields(int start, int &end, std::map<std::string, std::string> &fields);
    bool is_valid_line(const std::string &line, int expected_indent) const;
};

#endif // YAML_PARSER_H