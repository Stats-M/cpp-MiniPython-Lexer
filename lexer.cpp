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
    // ��� ��������������� ������� ��������� ������ ��������
    // ��� �������� ������� �������, ���������������� ������ ����
    //ParseInputStream(in_stream_);
}

const Token& Lexer::CurrentToken() const
{
    return *current_token_it_;
}


Token Lexer::NextToken()
{
    //TODO ��������. ���������� ����� ��������������
    throw std::logic_error("Not implemented"s);
}


void Lexer::ParseInputStream(std::istream& istr)
{
    // ������ ������ �������:
    // ���� � ������ ���� �������, ��������������� ��������� ��������
    //  1. ��������� �� ������
    //  2. ��������� �� ������
    //  3. ��������� �� ������� (��������, <=) � �������� (= ��� :) �������
    //  4. ��������� �� �������� ����� � ��������������
    //  5. ��������� �� �����
    //  6. ��������� �� �����������
    //  7. ��������� �� ������� � ����� ������
    //  8. ��������� �� ����� ������

    // ������������� ����������
    global_indent_counter_ = 0;
    tokens_.clear();
    current_token_it_ = tokens_.begin();

    // ������� ���������� ������� �� ������
    TrimSpaces(istr);

    // �������� ���� ��������� ��������� ������
    while (istr.peek())
    {
        ParseIndent(istr);
        ParseString(istr);
        ParseKeywords(istr);
        ParseNewLine(istr);
    }


    // ������ ������ ��������
    tokens_.emplace_back(token_type::Eof{});
    // ��������� ��������, ����������� �� ������� �����
    current_token_it_ = tokens_.begin();
}


void Lexer::ParseIndent(std::istream& istr)
{
    char ch;
    int spaces_processed = 0;

    // ���� ���� ������� � ������ � ��� �������, ��������� ��
    while (istr.get(ch) && (ch == ' '))
    {
        ++spaces_processed;
    }

    // ������ ��������� �� ���������� ������, ���� ������ ��� �� ����� ������
    if (istr.rdstate() != std::ios_base::eofbit)
    {
        istr.putback(ch);
    }

    // ���� �������� ������ ����������� �������� ��������, ��� Indent (1 ��� �����)
    if (global_indent_counter_ * SPACES_PER_INDENT < spaces_processed)
    {
        // ����� ��������� ���������� ������, ���� ����� �������� �� ������ SPACES_PER_INDENT

        // ��������� ����� ��������, ����������� ������� ���������� ������
        spaces_processed -= global_indent_counter_ * SPACES_PER_INDENT;
        // ��������� �� 1 ������� �� ������ ����� �������� (0...SPACES_PER_INDENT]
        while (spaces_processed > 0)
        {
            spaces_processed -= SPACES_PER_INDENT;
            tokens_.emplace_back(token_type::Indent{});
            ++global_indent_counter_;
        }
    }
    // ���� �������� ������ ����������� �������� ��������, ��� Dedent (1 ��� �����) 
    else if (global_indent_counter_ * SPACES_PER_INDENT > spaces_processed)
    {
        // ����� ��������� ���������� ������, ���� ����� �������� �� ������ SPACES_PER_INDENT

        // ��������� ����� ��������, �� ��������� �� �������� ����������� �������
        spaces_processed = global_indent_counter_ * SPACES_PER_INDENT - spaces_processed;
        // ���������� �� 1 ������� �� ������ ����� �������� (0...SPACES_PER_INDENT]
        while (spaces_processed > 0)
        {
            spaces_processed -= SPACES_PER_INDENT;
            tokens_.emplace_back(token_type::Dedent{});
            --global_indent_counter_;
        }
    }

    // ���������� ��� �� ���������, �������� � ����� ������������ ������
    if (global_indent_counter_ < 0)
    {
        // ������. ������� �������� ������ ����
        using namespace std::literals;
        throw LexerError("ParseIndent() produced negative global indent: "s + std::to_string(global_indent_counter_));
    }
}


void Lexer::ParseString(std::istream& istr)
{
    char open_char = istr.get();

    // ���� ����������� ������ - ����� �������...
    if ((open_char == '\'') || (open_char == '\"'))
    {
        // ...��� ������

        char ch;
        std::string result;

        // �������������� ������� ������� ����� ��������� ������ .get()
        //auto begin_it = std::istreambuf_iterator<char>(istr);
        //auto end_it = std::istreambuf_iterator<char>();
        // ������ ������������� std::istreambuf_iterator
        // https://stackoverflow.com/questions/27406789/confused-about-usage-of-stdistreambuf-iterator

        // ������ ������� �� ������ ���� �� �������� ����������� ������
        while (istr.get(ch))
        {
            if (ch == open_char)
            {
                // ������ ����������� ������. ������� �� �����
                break;
            }
            else if (ch == '\\')
            {
                // ����. ���� �������������� ������ ��������� ������
                char esc_ch;
                if (istr.get(esc_ch))
                {
                    // ��� ���������� esc-������� ��������� � ����������
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
                    // ������. ����������� ����� ������
                    using namespace std::literals;
                    throw LexerError("ParseString() has encountered unexpected end of stream after a backslash"s);
                }
            }
            else if ((ch == '\n') || (ch == '\r'))
            {
                // ������. ������������ ������ �������� ������ ��� �������� �������
                using namespace std::literals;
                throw LexerError("ParseString() has encountered NL or CR symbol within a string"s);
            }
            else
            {
                // ...��� ��������� ������� ���������. ���������� ���
                result.push_back(ch);
            }
        }

        // ��������, ��� �� ������ ����������� ������ ��� ����� ���������� ������
        if (open_char == ch)
        {
            // ����������� ������ ��� ������. ���������� �����
            tokens_.emplace_back(token_type::String{ result });
        }
        else
        {
            // ������. �� ����� ����� ��� � �� ����� ����������� ������
            using namespace std::literals;
            throw LexerError("ParseString() has exited without find end-of-string character"s);
        }
    }
    // ... ����� ��� �� ������, ���������� ������ � �����
    else
    {
        istr.putback(open_char);
    }
}


void Lexer::ParseKeywords(std::istream& istr)
{
    char ch = istr.peek();

    // �������� ����� � �������������� ������ ����������
    // � ���� ��� ����� �������������
    if (std::isalpha(ch) || ch == '_')
    {
        std::string keyword;
        // ������ ������ ��� ��������, ���������� �������
        // ����� ����� �������� � �����
        while (istr.get(ch))
        {
            if (std::isalnum(ch) || ch == '_')
            {
                keyword.push_back(ch);
            }
            else
            {
                // ������� ������ �� �����, ����� ��� _. ���������� ��� � ������� �� �����
                istr.putback(ch);
                break;
            }
        }
        // ���� �������� ����� ������, ��� ���������.

        // ��������� �������� ����� ��� ID � ������ �������
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

    // ���� ��������� ������ � ������ - ������ �����������...
    if (ch == '#')
    {
        // ������ �� ������ ����� ��� ������ ������������ �� �����
        std::string tmp_str;
        std::getline(istr, tmp_str, '\n');

        // ������ ��������� ������ \n, ���� ������ ��� �� ����� ������
        if (istr.rdstate() != std::ios_base::eofbit)
        {
            istr.putback('\n');
        }

        // NB. ���� ����� �� ������������ ������ �������� ������ � �����������
        // # comment with \n a newline character in the middle
        // ��� ����� ���������������� ��� ����� ������ � ��������������
    }
    // else ��� �� �����������, ������ �� ������
}

void Lexer::TrimSpaces(std::istream& istr)
{
    while (istr.peek() == ' ')
    {
        istr.get();
    }
}

}  // namespace parse