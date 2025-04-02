#include "yaml_parser.h"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iostream>

using namespace std;

bool YamlParser::load_templates(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
        return false;

    lines_.clear();
    templates_.clear();

    string line;
    while (getline(file, line))
    {
        lines_.push_back(line);
    }

    parse_yaml(templates_);
    return true;
}

bool YamlParser::load_targets(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
        return false;

    lines_.clear();
    targets_.clear();

    string line;
    while (getline(file, line))
    {
        lines_.push_back(line);
    }

    parse_yaml(targets_);
    return true;
}

Entry YamlParser::find_by_package(const string &package_name) const
{
    for (const auto &target : targets_)
    {
        if (target.package_name == package_name)
        {
            Entry result = target;

            // 合并模板字段
            if (!result.template_name.empty())
            {
                for (const auto &tpl : templates_)
                {
                    if (tpl.template_name == result.template_name)
                    {
                        for (const auto &[key, value] : tpl.fields)
                        {
                            if (result.fields.find(key) == result.fields.end())
                            {
                                result.fields[key] = value;
                            }
                        }
                        break;
                    }
                }
            }
            return result;
        }
    }
    return {};
}

bool YamlParser::contains_package(const string &package_name) const
{
    return any_of(targets_.begin(), targets_.end(),
                  [&](const Entry &e)
                  { return e.package_name == package_name; });
}

bool YamlParser::is_valid_format() const
{
    if (lines_.empty())
        return false;

    vector<int> indent_stack = {0};

    for (size_t i = 0; i < lines_.size(); ++i)
    {
        const string &line = lines_[i];
        if (trim(line).empty())
            continue;

        int current_indent = get_indent(line);
        int expected_indent = indent_stack.back();

        if (current_indent < expected_indent)
        {
            while (!indent_stack.empty() && current_indent < indent_stack.back())
            {
                indent_stack.pop_back();
            }
            if (indent_stack.empty() || current_indent != indent_stack.back())
            {
                return false;
            }
            expected_indent = current_indent;
        }
        else if (current_indent > expected_indent)
        {
            if (current_indent != expected_indent + 2)
            {
                return false;
            }
            indent_stack.push_back(current_indent);
        }

        if (!is_valid_line(line, expected_indent))
        {
            return false;
        }

        if (expected_indent == 0 && line.find("- ") == 0)
        {
            string content = trim(line.substr(2));
            if (content.find(':') == string::npos)
            {
                return false;
            }
        }
    }
    return true;
}

// 以下是私有方法实现
void YamlParser::parse_yaml(vector<Entry> &output)
{
    for (size_t i = 0; i < lines_.size(); ++i)
    {
        string line = trim(lines_[i]);
        if (line.empty())
            continue;

        if (lines_[i].find("- ") == 0 && get_indent(lines_[i]) == 0)
        {
            Entry entry;
            string content = lines_[i].substr(2);
            auto [key, value] = parse_key_value(content);

            if (key == "template")
                entry.template_name = value;
            else if (key == "package_name")
                entry.package_name = value;

            i++;
            while (i < lines_.size())
            {
                int indent = get_indent(lines_[i]);
                if (indent < 2)
                {
                    i--;
                    break;
                }

                if (indent == 2)
                {
                    auto [k, v] = parse_key_value(lines_[i].substr(2));
                    if (k == "fields")
                    {
                        int fields_end;
                        parse_fields(i + 1, fields_end, entry.fields);
                        i = fields_end;
                    }
                    else if (k == "template")
                    {
                        entry.template_name = v;
                    }
                    else if (k == "package_name")
                    {
                        entry.package_name = v;
                    }
                }
                i++;
            }
            output.push_back(entry);
        }
    }
}

int YamlParser::get_indent(const string &line) const
{
    int indent = 0;
    while (indent < line.size() && line[indent] == ' ')
        indent++;
    return indent;
}

string YamlParser::trim(const string &s) const
{
    size_t start = s.find_first_not_of(" \t");
    if (start == string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

pair<string, string> YamlParser::parse_key_value(const string &line) const
{
    string trimmed = trim(line);
    size_t colon_pos = trimmed.find(':');
    if (colon_pos == string::npos)
        return {"", ""};

    string key = trimmed.substr(0, colon_pos);
    string value = trimmed.substr(colon_pos + 1);
    return {key, trim(value)};
}

void YamlParser::parse_fields(int start, int &end, map<string, string> &fields)
{
    end = start;
    while (end < lines_.size())
    {
        string line = lines_[end];
        int indent = get_indent(line);

        if (indent != 4)
            break;

        line = line.substr(4);
        auto [key, value] = parse_key_value(line);
        if (!key.empty())
            fields[key] = value;
        end++;
    }
    end--;
}

bool YamlParser::is_valid_line(const string &line, int expected_indent) const
{
    string content = trim(line.substr(expected_indent));
    if (content.empty())
        return false;

    if (expected_indent == 0 && content[0] == '-')
    {
        if (content.size() < 2 || content[1] != ' ')
        {
            return false;
        }
        content = trim(content.substr(2));
    }

    size_t colon_pos = content.find(':');
    if (colon_pos == string::npos)
    {
        return false;
    }

    string key = trim(content.substr(0, colon_pos));
    if (key.empty())
    {
        return false;
    }

    if (content.size() > colon_pos + 1 && content[colon_pos + 1] != ' ')
    {
        return false;
    }

    return true;
}