#pragma once
// =====================================================================
// FASE G6.1 - Garuda Shell: REPL interaktif untuk GVM (setara erl shell)
// =====================================================================
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <optional>
#include <sstream>
#include <iostream>
#include <cstddef>
#include <cctype>

namespace garuda::beam {

struct ShellCommand {
    std::string name;
    std::string description;
    std::function<std::string(const std::vector<std::string>& args)> handler;
};

struct ShellResult {
    bool success = false;
    std::string output;
    std::string error;
};

class Shell {
private:
    std::map<std::string, ShellCommand> commands_;
    std::vector<std::string> history_;
    mutable std::mutex mu_;
    bool running_ = false;

public:
    Shell() {
        register_builtins();
    }

    void register_command(const std::string& name,
                          const std::string& description,
                          std::function<std::string(const std::vector<std::string>&)> handler) {
        std::lock_guard<std::mutex> lock(mu_);
        ShellCommand cmd;
        cmd.name = name;
        cmd.description = description;
        cmd.handler = std::move(handler);
        commands_[name] = cmd;
    }

    static std::vector<std::string> tokenize(const std::string& line) {
        std::vector<std::string> tokens;
        std::string current;
        bool in_quotes = false;

        for (char c : line) {
            if (c == '"') {
                in_quotes = !in_quotes;
            } else if (std::isspace(static_cast<unsigned char>(c)) && !in_quotes) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current.push_back(c);
            }
        }
        if (!current.empty()) {
            tokens.push_back(current);
        }
        return tokens;
    }

    ShellResult execute(const std::string& line) {
        std::lock_guard<std::mutex> lock(mu_);
        if (!line.empty()) {
            history_.push_back(line);
        }

        auto tokens = tokenize(line);
        ShellResult res;

        if (tokens.empty()) {
            res.success = true;
            res.output = "";
            return res;
        }

        std::string cmd_name = tokens[0];
        auto it = commands_.find(cmd_name);
        if (it == commands_.end()) {
            res.success = false;
            res.error = "Unknown command: " + cmd_name;
            return res;
        }

        std::vector<std::string> args(tokens.begin() + 1, tokens.end());
        try {
            res.output = it->second.handler(args);
            res.success = true;
        } catch (const std::exception& e) {
            res.success = false;
            res.error = std::string("Error: ") + e.what();
        } catch (...) {
            res.success = false;
            res.error = "Unknown execution error";
        }

        return res;
    }

    std::vector<ShellResult> execute_script(const std::string& script) {
        std::vector<ShellResult> results;
        std::istringstream ss(script);
        std::string line;
        while (std::getline(ss, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            results.push_back(execute(line));
        }
        return results;
    }

    std::vector<std::string> history() const {
        std::lock_guard<std::mutex> lock(mu_);
        return history_;
    }

    void clear_history_internal() {
        history_.clear();
    }

    void clear_history() {
        std::lock_guard<std::mutex> lock(mu_);
        clear_history_internal();
    }

    void register_builtins() {
        register_command("help", "Print available commands", [this](const std::vector<std::string>&) {
            std::ostringstream ss;
            ss << "Garuda GVM Shell Commands:\n";
            for (const auto& [name, cmd] : commands_) {
                ss << "  " << name << " - " << cmd.description << "\n";
            }
            return ss.str();
        });

        register_command("echo", "Echo argument text", [](const std::vector<std::string>& args) {
            std::ostringstream ss;
            for (std::size_t i = 0; i < args.size(); ++i) {
                if (i > 0) ss << " ";
                ss << args[i];
            }
            return ss.str();
        });

        register_command("version", "Print Garuda BEAM version", [](const std::vector<std::string>&) {
            return "Garuda BEAM v67.0.0";
        });

        register_command("status", "Print shell running status", [this](const std::vector<std::string>&) {
            return running_ ? "running" : "stopped";
        });

        register_command("clear", "Clear command history", [this](const std::vector<std::string>&) {
            clear_history_internal();
            return "History cleared";
        });
    }

    std::size_t command_count() const {
        std::lock_guard<std::mutex> lock(mu_);
        return commands_.size();
    }

    bool is_running() const {
        std::lock_guard<std::mutex> lock(mu_);
        return running_;
    }

    void start() {
        std::lock_guard<std::mutex> lock(mu_);
        running_ = true;
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mu_);
        running_ = false;
    }
};

} // namespace garuda::beam
