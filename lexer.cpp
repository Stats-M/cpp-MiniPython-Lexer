#include "lexer.h"

#include <algorithm>
#include <charconv>
//#include <unordered_map>

using namespace std;

namespace parse
{

bool operator==(const Token& lhs, const Token& rhs)
{
    using namespace token_type;

    if (lhs.index() != rhs.index())
    {
        return false;
    }
    if (lhs.Is<Char>())
    {
        return lhs.As<Char>().value == rhs.As<Char>().value;
    }
    if (lhs.Is<Number>())
    {
        return lhs.As<Number>().value == rhs.As<Number>().value;
    }
    if (lhs.Is<String>())
    {
        return lhs.As<String>().value == rhs.As<String>().value;
    }
    if (lhs.Is<Id>())
    {
        return lhs.As<Id>().value == rhs.As<Id>().value;
    }
    return true;
}

bool operator!=(const Token& lhs, const Token& rhs)
{
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const Token& rhs)
{
    using namespace token_type;

#define VALUED_OUTPUT(type) \
    if (auto p = rhs.TryAs<type>()) return os << #type << '{' << p->value << '}';

    VALUED_OUTPUT(Number);
    VALUED_OUTPUT(Id);
    VALUED_OUTPUT(String);
    VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
    if (rhs.Is<type>()) return os << #type;

    UNVALUED_OUTPUT(Class);
    UNVALUED_OUTPUT(Return);
    UNVALUED_OUTPUT(If);
    UNVALUED_OUTPUT(Else);
    UNVALUED_OUTPUT(Def);
    UNVALUED_OUTPUT(Newline);
    UNVALUED_OUTPUT(Print);
    UNVALUED_OUTPUT(Indent);
    UNVALUED_OUTPUT(Dedent);
    UNVALUED_OUTPUT(And);
    UNVALUED_OUTPUT(Or);
    UNVALUED_OUTPUT(Not);
    UNVALUED_OUTPUT(Eq);
    UNVALUED_OUTPUT(NotEq);
    UNVALUED_OUTPUT(LessOrEq);
    UNVALUED_OUTPUT(GreaterOrEq);
    UNVALUED_OUTPUT(None);
    UNVALUED_OUTPUT(True);
    UNVALUED_OUTPUT(False);
    UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

    return os << "Unknown token :("sv;
}

Lexer::Lexer(std::istream& input) : in_stream_(input)
{
    current_token_it_ = tokens_.begin();
    // Для автоматического разбора входящего потока символов
    // при создании объекта лексера, раскомментируйте строку ниже
    //ParseInputStream(in_stream_);
}

const Token& Lexer::CurrentToken() const
{
    return *current_token_it_;
}


Token Lexer::NextToken()
{
    //TODO Заглушка. Реализуйте метод самостоятельно
    throw std::logic_error("Not implemented"s);
}


void Lexer::ParseInputStream(std::istream& istr)
{
    // Логика работы парсера:
    // Пока в потоке есть символы, последовательно выполняем проверки
    //  1. Проверить на отступ
    //  2. Проверить на ключевые слова и идентификаторы
    //  3. Проверить на длинные операторы (например, <=)
    //  4. Проверить на короткие операторы (например, = или :)
    //  5. Проверить на значения для переменных (строки или цифры)
    //  6. Проверить на комментарии
    //  7. Проверить на пробелы в конце строки
    //  8. Проверить на конец строки

    // Инициализирум переменные
    spaces_counter_ = 0;
    tokens_.clear();
    current_token_it_ = tokens_.begin();

    // Основной цикл обработки входящего потока.
    while (istr.peek())
    {
        ParseNewLine(istr);
    }


    // Разбор потока завершен
    tokens_.emplace_back(token_type::Eof{});
    // Обновляем итератор, указывающий на текущий токен
    current_token_it_ = tokens_.begin();
}


void Lexer::ParseIndent(std::istream& istr)
{
}


void Lexer::ParseNewLine(std::istream& istr)
{
}

}  // namespace parse