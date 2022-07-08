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
    //  2. Проверить на строки
    //  3. Проверить на длинные (например, <=) и короткие (= или :) символы
    //  4. Проверить на ключевые слова и идентификаторы
    //  5. Проверить на числа
    //  6. Проверить на комментарии
    //  7. Проверить на пробелы в конце строки
    //  8. Проверить на конец строки

    // Инициализирум переменные
    global_indent_counter_ = 0;
    tokens_.clear();
    current_token_it_ = tokens_.begin();

    // Удаляем лидирующие пробелы из потока
    TrimSpaces(istr);

    // Основной цикл обработки входящего потока
    while (istr.peek())
    {
        ParseIndent(istr);
        ParseString(istr);
        ParseKeywords(istr);
        ParseNewLine(istr);
    }


    // Разбор потока завершен
    tokens_.emplace_back(token_type::Eof{});
    // Обновляем итератор, указывающий на текущий токен
    current_token_it_ = tokens_.begin();
}


void Lexer::ParseIndent(std::istream& istr)
{
    char ch;
    int spaces_processed = 0;

    // Пока есть символы в потоке и это пробелы, считываем их
    while (istr.get(ch) && (ch == ' '))
    {
        ++spaces_processed;
    }

    // Вернем последний не пробельный символ, если только это не конец потока
    if (istr.rdstate() != std::ios_base::eofbit)
    {
        istr.putback(ch);
    }

    // Если отступов больше глобального счетчика отступов, это Indent (1 или более)
    if (global_indent_counter_ * SPACES_PER_INDENT < spaces_processed)
    {
        // Нужно корректно обработать случай, если число пробелов не кратно SPACES_PER_INDENT

        // Вычисляем число пробелов, превышающих текущий глобальный отступ
        spaces_processed -= global_indent_counter_ * SPACES_PER_INDENT;
        // Добавляем по 1 отступу за каждое число пробелов (0...SPACES_PER_INDENT]
        while (spaces_processed > 0)
        {
            spaces_processed -= SPACES_PER_INDENT;
            tokens_.emplace_back(token_type::Indent{});
            ++global_indent_counter_;
        }
    }
    // Если отступов меньше глобального счетчика отступов, это Dedent (1 или более) 
    else if (global_indent_counter_ * SPACES_PER_INDENT > spaces_processed)
    {
        // Нужно корректно обработать случай, если число пробелов не кратно SPACES_PER_INDENT

        // Вычисляем число пробелов, не достающих до текущего глобального отступа
        spaces_processed = global_indent_counter_ * SPACES_PER_INDENT - spaces_processed;
        // Откатываем по 1 отступу за каждое число пробелов (0...SPACES_PER_INDENT]
        while (spaces_processed > 0)
        {
            spaces_processed -= SPACES_PER_INDENT;
            tokens_.emplace_back(token_type::Dedent{});
            --global_indent_counter_;
        }
    }

    // Дальнейший код не требуется, приведен в целях отслеживания ошибок
    if (global_indent_counter_ < 0)
    {
        // Ошибка. Счетчик отступов меньше нуля
        using namespace std::literals;
        throw LexerError("ParseIndent() produced negative global indent: "s + std::to_string(global_indent_counter_));
    }
}


void Lexer::ParseString(std::istream& istr)
{
    char open_char = istr.get();

    // Если открывающий символ - любая кавычка...
    if ((open_char == '\'') || (open_char == '\"'))
    {
        // ...это строка

        char ch;
        std::string result;

        // Альтернативный вариант решения через указатели вместо .get()
        //auto begin_it = std::istreambuf_iterator<char>(istr);
        //auto end_it = std::istreambuf_iterator<char>();
        // Пример использования std::istreambuf_iterator
        // https://stackoverflow.com/questions/27406789/confused-about-usage-of-stdistreambuf-iterator

        // Читаем символы из потока пока не встретим закрывающий символ
        while (istr.get(ch))
        {
            if (ch == open_char)
            {
                // Найден закрывающий символ. Выходим из цикла
                break;
            }
            else if (ch == '\\')
            {
                // Слэш. Ищем экранированный слэшем следующий символ
                char esc_ch;
                if (istr.get(esc_ch))
                {
                    // Все допустимые esc-символы добавляем к результату
                    switch (esc_ch)
                    {
                    case 'n':
                        result.push_back('\n');
                        break;
                    case 't':
                        result.push_back('\t');
                        break;
                    case 'r':
                        result.push_back('\r');
                        break;
                    case '"':
                        result.push_back('"');
                        break;
                    case '\'':
                        result.push_back('\'');
                        break;
                    case '\\':
                        result.push_back('\\');
                        break;
                    default:
                        throw std::logic_error("ParseString() has encountered unknown escape sequence \\"s + esc_ch);
                    }
                }
                else
                {
                    // Ошибка. Неожиданный конец потока
                    using namespace std::literals;
                    throw LexerError("ParseString() has encountered unexpected end of stream after a backslash"s);
                }
            }
            else if ((ch == '\n') || (ch == '\r'))
            {
                // Ошибка. Недопустимый символ перевода строки или возврата каретки
                using namespace std::literals;
                throw LexerError("ParseString() has encountered NL or CR symbol within a string"s);
            }
            else
            {
                // ...все остальные символы допустимы. Запоминаем его
                result.push_back(ch);
            }
        }

        // Проверим, был ли найден закрывающий символ или поток закончился раньше
        if (open_char == ch)
        {
            // Закрывающий символ был найден. Запоминаем токен
            tokens_.emplace_back(token_type::String{ result });
        }
        else
        {
            // Ошибка. Из цикла вышли так и не найдя закрывающий символ
            using namespace std::literals;
            throw LexerError("ParseString() has exited without find end-of-string character"s);
        }
    }
    // ... иначе это не строка, возвращаем символ в поток
    else
    {
        istr.putback(open_char);
    }
}


void Lexer::ParseKeywords(std::istream& istr)
{
    char ch = istr.peek();

    // Ключевые слова и идентификаторы должны начинаться
    // с букв или знака подчеркивания
    if (std::isalpha(ch) || ch == '_')
    {
        std::string keyword;
        // Первый символ уже проверен, дальнейшие символы
        // могут также включать и цифры
        while (istr.get(ch))
        {
            if (std::isalnum(ch) || ch == '_')
            {
                keyword.push_back(ch);
            }
            else
            {
                // Текущим символ не буквы, цифры или _. Возвращаем его и выходим из цикла
                istr.putback(ch);
                break;
            }
        }
        // Если наступил конец потока, это допустимо.

        // Добавляем ключевое слово или ID в вектор токенов
        if (keywords_map_.find(keyword) != keywords_map_.end())
        {
            tokens_.push_back(keywords_map_.at(keyword));
        }
        else
        {
            tokens_.emplace_back(token_type::Id{ keyword });
        }
    }
}


void Lexer::ParseNewLine(std::istream& istr)
{
}


void Lexer::ParseComments(std::istream& istr)
{
    char ch = istr.peek();

    // Если следующий символ в потоке - символ комментария...
    if (ch == '#')
    {
        // Читаем из потока сразу всю строку комментариев до конца
        std::string tmp_str;
        std::getline(istr, tmp_str, '\n');

        // Вернем последний символ \n, если только это не конец потока
        if (istr.rdstate() != std::ios_base::eofbit)
        {
            istr.putback('\n');
        }

        // NB. Этот метод не обрабатывает случай перевода строки в комментарии
        // # comment with \n a newline character in the middle
        // Это будет интерпретировано как новая строка и идентификаторы
    }
    // else это не комментарий, ничего не делаем
}

void Lexer::TrimSpaces(std::istream& istr)
{
    while (istr.peek() == ' ')
    {
        istr.get();
    }
}

}  // namespace parse