
#ifndef NOVA_XML_HPP
#define NOVA_XML_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>

namespace nova_sim::xml {

class XmlNode;

class XmlAttribute {
public:
    XmlAttribute() = default;
    XmlAttribute(std::string name, std::string value) : name_(std::move(name)), value_(std::move(value)) {}

    const std::string& name() const { return name_; }
    const std::string& value() const { return value_; }

    bool empty() const { return name_.empty(); }

    const char* as_string() const { return value_.c_str(); }
    double as_double() const { try { return std::stod(value_); } catch (...) { return 0.0; } }
    int as_int() const { try { return std::stoi(value_); } catch (...) { return 0; } }
    unsigned int as_uint() const { try { return static_cast<unsigned int>(std::stoul(value_)); } catch (...) { return 0; } }
    bool as_bool() const {
        std::string v = value_;
        std::transform(v.begin(), v.end(), v.begin(), ::tolower);
        return v == "true" || v == "1";
    }

    operator bool() const { return !name_.empty(); }

private:
    std::string name_;
    std::string value_;
};

class XmlNode {
public:
    XmlNode() = default;
    
    const std::string& name() const { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }

    const std::string& text() const { return text_; }
    void set_text(std::string text) { text_ = std::move(text); }

    XmlNode child(const char* name) const {
        for (const auto& child : children_) {
            if (child.name_ == name) return child;
        }
        return XmlNode();
    }

    XmlAttribute attribute(const char* name) const {
        auto it = attributes_.find(name);
        if (it != attributes_.end()) return XmlAttribute(it->first, it->second);
        return XmlAttribute();
    }

    const std::vector<XmlNode>& children() const { return children_; }
    
    std::vector<XmlNode> children(const char* name) const {
        std::vector<XmlNode> result;
        for (const auto& child : children_) {
            if (child.name_ == name) result.push_back(child);
        }
        return result;
    }

    bool empty() const { return name_.empty() && children_.empty(); }

    void add_child(XmlNode node) { children_.push_back(std::move(node)); }
    void add_attribute(std::string name, std::string value) { attributes_[std::move(name)] = std::move(value); }

    // Iteration support
    auto begin() const { return children_.begin(); }
    auto end() const { return children_.end(); }

    operator bool() const { return !name_.empty() || !children_.empty(); }

protected:
    std::vector<XmlNode> children_;
    std::map<std::string, std::string> attributes_;

private:
    std::string name_;
    std::string text_;
};

class XmlDocument : public XmlNode {
public:
    bool load_file(const char* path) {
        std::ifstream ifs(path);
        if (!ifs) return false;
        std::stringstream ss;
        ss << ifs.rdbuf();
        return load_string(ss.str());
    }

    bool load_string(const std::string& xml) {
        size_t pos = 0;
        children_.clear();
        attributes_.clear();
        set_name(""); 
        
        while (true) {
            skip_junk(xml, pos);
            if (pos >= xml.size()) break;

            if (xml[pos] == '<') {
                if (xml.compare(pos, 2, "</") == 0) return false; 
                
                XmlNode root;
                if (parse_node(xml, pos, root)) {
                    this->add_child(std::move(root));
                } else {
                    return false;
                }
            } else {
                pos++;
            }
        }
        return !children().empty();
    }

private:
    void skip_whitespace(const std::string& xml, size_t& pos) {
        while (pos < xml.size() && std::isspace(static_cast<unsigned char>(xml[pos]))) pos++;
    }

    void skip_junk(const std::string& xml, size_t& pos) {
        while (true) {
            skip_whitespace(xml, pos);
            if (pos >= xml.size()) break;
            if (xml.compare(pos, 4, "<!--") == 0) {
                pos = xml.find("-->", pos);
                if (pos == std::string::npos) { pos = xml.size(); break; }
                pos += 3;
            } else if (xml.compare(pos, 2, "<?") == 0) {
                pos = xml.find("?>", pos);
                if (pos == std::string::npos) { pos = xml.size(); break; }
                pos += 2;
            } else {
                break;
            }
        }
    }

    bool parse_node(const std::string& xml, size_t& pos, XmlNode& node) {
        skip_junk(xml, pos);
        if (pos >= xml.size() || xml[pos] != '<') return false;
        pos++;

        // Tag name
        size_t end_name = xml.find_first_of(" />\t\r\n", pos);
        if (end_name == std::string::npos) return false;
        node.set_name(xml.substr(pos, end_name - pos));
        pos = end_name;

        // Attributes
        while (true) {
            skip_whitespace(xml, pos);
            if (pos >= xml.size()) return false;
            if (xml[pos] == '>' || xml.compare(pos, 2, "/>") == 0) break;

            size_t end_attr = xml.find('=', pos);
            if (end_attr == std::string::npos) break;
            std::string attr_name = xml.substr(pos, end_attr - pos);
            pos = end_attr + 1;

            skip_whitespace(xml, pos);
            if (pos >= xml.size()) return false;
            char quote = xml[pos++];
            size_t end_val = xml.find(quote, pos);
            if (end_val == std::string::npos) return false;
            node.add_attribute(attr_name, xml.substr(pos, end_val - pos));
            pos = end_val + 1;
        }

        if (xml[pos] == '/') {
            pos += 2; // />
            return true;
        }
        pos++; // >

        // Content / Children
        while (true) {
            skip_junk(xml, pos);
            if (pos >= xml.size()) return false;

            if (xml.compare(pos, 2, "</") == 0) {
                size_t end_close = xml.find('>', pos);
                if (end_close == std::string::npos) return false;
                pos = end_close + 1;
                return true;
            }

            if (xml[pos] == '<') {
                XmlNode child;
                if (parse_node(xml, pos, child)) {
                    node.add_child(std::move(child));
                } else {
                    return false;
                }
            } else {
                size_t end_text = xml.find('<', pos);
                if (end_text == std::string::npos) return false;
                node.set_text(xml.substr(pos, end_text - pos));
                pos = end_text;
            }
        }
    }
};

} // namespace nova_sim::xml

#endif // NOVA_XML_HPP
