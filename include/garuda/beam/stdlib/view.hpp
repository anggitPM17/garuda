#pragma once
// Garuda Language Web Framework: View Templating & HTML Rendering Engine (`view.hpp`)
// Mengimplementasikan Template Engine dengan Variable Substitution, Conditionals, Loops, & Layout Inheritance.
#include "../core/term.hpp"
#include "json.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace garuda::beam::stdlib {

class ViewEngine {
public:
    ViewEngine() = default;

    static std::string render_template(const std::string& template_str, const Term& data) {
        std::string result = template_str;

        // 1. Variable substitution: {{ var_name }}
        std::size_t pos = 0;
        while ((pos = result.find("{{", pos)) != std::string::npos) {
            std::size_t end_pos = result.find("}}", pos);
            if (end_pos == std::string::npos) break;

            std::string var_name = result.substr(pos + 2, end_pos - (pos + 2));
            var_name = trim(var_name);

            bool is_raw = false;
            if (var_name.rfind("raw_", 0) == 0) {
                is_raw = true;
                var_name = var_name.substr(4);
            }

            std::string val_str = resolve_variable(var_name, data);
            if (!is_raw) {
                val_str = escape_html(val_str);
            }
            result.replace(pos, (end_pos + 2) - pos, val_str);
            pos += val_str.length();
        }

        // 2. Conditionals: {% if var %} ... {% else %} ... {% endif %}
        result = process_conditionals(result, data);

        // 3. Loops: {% for item in items %} ... {% endfor %}
        result = process_loops(result, data);

        return result;
    }

    static std::string render_with_layout(const std::string& layout_str, const std::string& content_str, const Term& data) {
        std::string rendered_content = render_template(content_str, data);
        std::string result = layout_str;

        std::size_t pos = result.find("{{ content }}");
        if (pos != std::string::npos) {
            result.replace(pos, 13, rendered_content);
        } else {
            pos = result.find("{{content}}");
            if (pos != std::string::npos) {
                result.replace(pos, 11, rendered_content);
            } else {
                result += rendered_content;
            }
        }

        return render_template(result, data);
    }

    static std::string escape_html(const std::string& input) {
        std::string output;
        output.reserve(input.size() * 1.2);
        for (char c : input) {
            switch (c) {
                case '<': output += "&lt;"; break;
                case '>': output += "&gt;"; break;
                case '&': output += "&amp;"; break;
                case '"': output += "&quot;"; break;
                case '\'': output += "&#39;"; break;
                default: output += c; break;
            }
        }
        return output;
    }

private:
    static std::string trim(const std::string& str) {
        std::size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        std::size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }

    static std::string resolve_variable(const std::string& var_name, const Term& data) {
        if (auto* str_val = data.get_if<std::string>()) {
            return *str_val;
        }
        if (auto* int_val = data.get_if<std::int64_t>()) {
            return std::to_string(*int_val);
        }
        if (auto* list_val = data.get_if<garuda::beam::List>()) {
            for (const auto& item : seq_items(list_val->items)) {
                if (auto* tup = item.get_if<garuda::beam::Tuple>()) {
                    const auto& pair = seq_items(tup->items);
                    if (pair.size() == 2) {
                        std::string k = pair[0].get_if<std::string>() ? *pair[0].get_if<std::string>() : to_string(pair[0]);
                        if (k == var_name) {
                            if (pair[1].get_if<std::string>()) return *pair[1].get_if<std::string>();
                            return to_string(pair[1]);
                        }
                    }
                }
            }
        }
        return "";
    }

    static std::string process_conditionals(const std::string& tpl, const Term& data) {
        std::string result = tpl;
        std::size_t if_pos = 0;

        while ((if_pos = result.find("{% if ", if_pos)) != std::string::npos) {
            std::size_t tag_end = result.find("%}", if_pos);
            if (tag_end == std::string::npos) break;

            std::string var_name = result.substr(if_pos + 6, tag_end - (if_pos + 6));
            var_name = trim(var_name);

            std::size_t endif_pos = result.find("{% endif %}", tag_end);
            if (endif_pos == std::string::npos) break;

            std::size_t else_pos = result.find("{% else %}", tag_end);
            std::string then_body, else_body;

            if (else_pos != std::string::npos && else_pos < endif_pos) {
                then_body = result.substr(tag_end + 2, else_pos - (tag_end + 2));
                else_body = result.substr(else_pos + 10, endif_pos - (else_pos + 10));
            } else {
                then_body = result.substr(tag_end + 2, endif_pos - (tag_end + 2));
            }

            std::string resolved_val = resolve_variable(var_name, data);
            bool cond_true = !resolved_val.empty() && resolved_val != "false" && resolved_val != "0" && resolved_val != "nil";

            std::string chosen_body = cond_true ? then_body : else_body;
            result.replace(if_pos, (endif_pos + 11) - if_pos, chosen_body);
            if_pos += chosen_body.length();
        }

        return result;
    }

    static std::string process_loops(const std::string& tpl, const Term& data) {
        std::string result = tpl;
        std::size_t for_pos = 0;

        while ((for_pos = result.find("{% for ", for_pos)) != std::string::npos) {
            std::size_t tag_end = result.find("%}", for_pos);
            if (tag_end == std::string::npos) break;

            std::string header = result.substr(for_pos + 7, tag_end - (for_pos + 7));
            header = trim(header); // e.g. "item in items"

            auto in_pos = header.find(" in ");
            if (in_pos == std::string::npos) break;

            std::string item_var = trim(header.substr(0, in_pos));
            std::string list_var = trim(header.substr(in_pos + 4));

            std::size_t endfor_pos = result.find("{% endfor %}", tag_end);
            if (endfor_pos == std::string::npos) break;

            std::string loop_body = result.substr(tag_end + 2, endfor_pos - (tag_end + 2));
            std::ostringstream loop_ss;

            if (auto* list_val = data.get_if<garuda::beam::List>()) {
                for (const auto& item : seq_items(list_val->items)) {
                    if (auto* tup = item.get_if<garuda::beam::Tuple>()) {
                        const auto& pair = seq_items(tup->items);
                        if (pair.size() == 2 && pair[0].get_if<std::string>() && *pair[0].get_if<std::string>() == list_var) {
                            if (auto* items_list = pair[1].get_if<garuda::beam::List>()) {
                                for (const auto& elem : seq_items(items_list->items)) {
                                    std::string rendered_item = loop_body;
                                    std::size_t var_pos = 0;
                                    std::string elem_str = elem.get_if<std::string>() ? *elem.get_if<std::string>() : to_string(elem);
                                    while ((var_pos = rendered_item.find("{{ " + item_var + " }}", var_pos)) != std::string::npos) {
                                        rendered_item.replace(var_pos, item_var.length() + 5, elem_str);
                                        var_pos += elem_str.length();
                                    }
                                    loop_ss << rendered_item;
                                }
                            }
                        }
                    }
                }
            }

            result.replace(for_pos, (endfor_pos + 12) - for_pos, loop_ss.str());
            for_pos += loop_ss.str().length();
        }

        return result;
    }
};

} // namespace garuda::beam::stdlib
