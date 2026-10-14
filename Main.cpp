#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <utility>
#include <algorithm>

struct Rule {
    std::string name;
    std::vector<std::string> conds;
    std::string conclusion;
};

std::vector<Rule> get_rules() {
    return {
        {"R1",  {"жанр=стратегия"}, "нагрузка=cpu_зависимая"},
        {"R2",  {"жанр=симулятор"}, "нагрузка=смешанная"},
        {"R3",  {"жанр=шутер", "производительность=максимальная"}, "нагрузка=gpu_зависимая"},
        {"R4",  {"жанр=rpg"}, "нагрузка=gpu_зависимая"},
        {"R5",  {"нагрузка=cpu_зависимая"}, "приоритет=cpu"},
        {"R6",  {"нагрузка=gpu_зависимая"}, "приоритет=gpu"},
        {"R7",  {"нагрузка=смешанная"}, "приоритет=баланс"},
        {"R8",  {"бюджет=низкий", "разрешение=1080p"}, "gpu=начальный"},
        {"R9",  {"бюджет=средний", "разрешение=1080p"}, "gpu=средний"},
        {"R10", {"бюджет=средний", "разрешение=1440p"}, "gpu=топовый"},
        {"R11", {"бюджет=высокий", "разрешение=1440p"}, "gpu=флагманский"},
        {"R12", {"бюджет=высокий", "разрешение=4k"}, "gpu=флагманский"},
        {"R13", {"производительность=максимальная", "приоритет=gpu"}, "gpu=флагманский"},
        {"R14", {"приоритет=cpu", "бюджет=средний"}, "cpu=средний"},
        {"R15", {"приоритет=cpu", "бюджет=высокий"}, "cpu=топовый"},
        {"R16", {"производительность=максимальная", "бюджет=высокий"}, "cpu=топовый"},
        {"R17", {"бюджет=низкий"}, "cpu=базовый"},
        {"R18", {"разрешение=4k"}, "vram=16gb"},
        {"R19", {"разрешение=1440p"}, "vram=12gb"},
        {"R20", {"жанр=симулятор", "производительность=высокая"}, "ram=32gb"},
        {"R21", {"cpu=топовый"}, "охлаждение=жидкостное"},
        {"R22", {"cpu=топовый", "gpu=флагманский"}, "psu=850вт"},
        {"R23", {"gpu=средний"}, "psu=650вт"},
        {"R24", {"gpu=начальный"}, "psu=500вт"},
        {"R25", {"cpu=топовый"}, "матплата=топовая"},
        {"R26", {"матплата=топовая"}, "ssd=nvme"}
    };
}

using Facts = std::set<std::string>;
using Fired = std::vector<std::pair<std::string, std::string>>;

std::pair<Facts, Fired> forward_chain(const Facts& initial_facts,
    const std::vector<Rule>& rules) {
    Facts facts = initial_facts;
    Fired fired;
    bool changed = true;

    while (changed) {
        changed = false;
        for (const auto& rule : rules) {
            if (facts.find(rule.conclusion) == facts.end() &&
                std::all_of(rule.conds.begin(), rule.conds.end(),
                    [&](const std::string& c) {
                        return facts.find(c) != facts.end();
                    })) {
                facts.insert(rule.conclusion);
                fired.emplace_back(rule.name, rule.conclusion);
                changed = true;
            }
        }
    }

    return { facts, fired };
}

std::map<std::string, std::string> recommend(const std::string& budget,
    const std::string& resolution,
    const std::string& genre,
    const std::string& perf) {
    Facts init = {
        "бюджет=" + budget,
        "разрешение=" + resolution,
        "жанр=" + genre,
        "производительность=" + perf
    };

    auto rules = get_rules();
    auto result = forward_chain(init, rules);
    const Facts& facts = result.first;
    const Fired& fired = result.second;

    std::cout << "\nВходные данные: бюджет=" << budget
        << ", " << resolution << ", " << genre << ", " << perf << "\n";

    for (const auto& p : fired) {
        std::cout << "  " << p.first << " -> " << p.second << "\n";
    }

    std::set<std::string> keys = {
        "cpu", "gpu", "ram", "psu",
        "охлаждение", "матплата", "ssd", "vram"
    };

    std::map<std::string, std::string> out;
    for (const auto& f : facts) {
        auto pos = f.find('=');
        if (pos != std::string::npos) {
            std::string key = f.substr(0, pos);
            if (keys.count(key)) {
                out[key] = f.substr(pos + 1);
            }
        }
    }

    return out;
}

int main() {
    setlocale(LC_ALL, "Russian");
    auto print_result = [](const std::map<std::string, std::string>& res) {
        std::cout << "{";
        bool first = true;
        for (const auto& p : res) {
            if (!first) std::cout << ", ";
            std::cout << "'" << p.first << "': '" << p.second << "'";
            first = false;
        }
        std::cout << "}\n";
        };

    print_result(recommend("низкий", "1080p", "шутер", "базовая"));
    print_result(recommend("средний", "1440p", "стратегия", "высокая"));
    print_result(recommend("высокий", "4k", "симулятор", "максимальная"));

    return 0;
}