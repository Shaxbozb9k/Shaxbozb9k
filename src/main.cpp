#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct Record {
  int id = 0;
  std::string name;
  std::string category;
  std::string note;
};

std::string trim(const std::string &value) {
  size_t start = 0;
  while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
    ++start;
  }
  size_t end = value.size();
  while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
    --end;
  }
  return value.substr(start, end - start);
}

std::string to_lower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return value;
}

std::string sanitize(const std::string &value) {
  std::string result = value;
  std::replace(result.begin(), result.end(), '|', '/');
  return result;
}

std::vector<std::string> split(const std::string &line, char delimiter) {
  std::vector<std::string> parts;
  std::stringstream ss(line);
  std::string item;
  while (std::getline(ss, item, delimiter)) {
    parts.push_back(item);
  }
  return parts;
}

class Database {
 public:
  explicit Database(const std::string &path) : path_(path) {}

  bool load() {
    records_.clear();
    std::ifstream file(path_);
    if (!file.is_open()) {
      return false;
    }
    std::string line;
    while (std::getline(file, line)) {
      if (line.empty()) {
        continue;
      }
      auto parts = split(line, '|');
      if (parts.size() != 4) {
        continue;
      }
      Record record;
      record.id = std::stoi(parts[0]);
      record.name = parts[1];
      record.category = parts[2];
      record.note = parts[3];
      records_.push_back(record);
    }
    return true;
  }

  bool save() const {
    std::ofstream file(path_, std::ios::trunc);
    if (!file.is_open()) {
      return false;
    }
    for (const auto &record : records_) {
      file << record.id << '|' << sanitize(record.name) << '|' << sanitize(record.category)
           << '|' << sanitize(record.note) << '\n';
    }
    return true;
  }

  int next_id() const {
    int max_id = 0;
    for (const auto &record : records_) {
      max_id = std::max(max_id, record.id);
    }
    return max_id + 1;
  }

  void add_record(const Record &record) { records_.push_back(record); }

  const std::vector<Record> &records() const { return records_; }

 private:
  std::string path_;
  std::vector<Record> records_;
};

void print_menu() {
  std::cout << "\n=== Универсальный справочник ===\n";
  std::cout << "1. Добавить запись\n";
  std::cout << "2. Показать все записи\n";
  std::cout << "3. Поиск\n";
  std::cout << "4. Выход\n";
  std::cout << "Выберите пункт: ";
}

void show_record(const Record &record) {
  std::cout << "ID: " << record.id << "\n";
  std::cout << "Название: " << record.name << "\n";
  std::cout << "Категория: " << record.category << "\n";
  std::cout << "Описание: " << record.note << "\n";
  std::cout << "------------------------\n";
}

Record read_record(int id) {
  Record record;
  record.id = id;
  std::string input;
  std::cout << "Название: ";
  std::getline(std::cin, input);
  record.name = trim(input);

  std::cout << "Категория: ";
  std::getline(std::cin, input);
  record.category = trim(input);

  std::cout << "Краткое описание: ";
  std::getline(std::cin, input);
  record.note = trim(input);

  return record;
}

void search_records(const std::vector<Record> &records) {
  std::string query;
  std::cout << "Введите ключевое слово: ";
  std::getline(std::cin, query);
  query = to_lower(trim(query));

  if (query.empty()) {
    std::cout << "Пустой запрос.\n";
    return;
  }

  bool found = false;
  for (const auto &record : records) {
    const std::string haystack = to_lower(record.name + " " + record.category + " " + record.note);
    if (haystack.find(query) != std::string::npos) {
      show_record(record);
      found = true;
    }
  }

  if (!found) {
    std::cout << "Ничего не найдено.\n";
  }
}

int main() {
  const std::string db_path = "data/database.txt";
  Database db(db_path);
  db.load();

  while (true) {
    print_menu();
    std::string choice;
    std::getline(std::cin, choice);
    if (choice == "1") {
      Record record = read_record(db.next_id());
      if (record.name.empty()) {
        std::cout << "Название обязательно.\n";
        continue;
      }
      db.add_record(record);
      if (db.save()) {
        std::cout << "Запись сохранена.\n";
      } else {
        std::cout << "Ошибка сохранения базы данных.\n";
      }
    } else if (choice == "2") {
      const auto &records = db.records();
      if (records.empty()) {
        std::cout << "Пока нет записей.\n";
      }
      for (const auto &record : records) {
        show_record(record);
      }
    } else if (choice == "3") {
      search_records(db.records());
    } else if (choice == "4") {
      std::cout << "До свидания!\n";
      break;
    } else {
      std::cout << "Неизвестная команда.\n";
    }
  }

  return 0;
}
