/*
 ����� ������������ ����������� ��� ������� ��������� �� ����� Mini-python (Mython). 
 ������ ������ ������������ �� ��������� ������ std::istream
*/

#pragma once

#include <iosfwd>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include <map>
#include <cctype>    // for isalpha() etc.

namespace parse
{

// ������������ ���� ���� ��������� ����� ������� (������ �����)
namespace token_type
{
struct Number
{
    // Lexeme �Number�
    int value;
};

struct Id
{
    // Lexeme �ID�
    ::std::string value;
};

struct Char
{
    // Lexeme �Char�
    char value;
};

struct String
{  
    // Lexem �const String�
    ::std::string value;
};

struct Class
{
    // ������� �class�
};
struct Return
{
    // ������� �return�
};
struct If
{
    // ������� �if�
};
struct Else
{
    // ������� �else�
};
struct Def
{
    // ������� �def�
};
struct Newline
{
    // ������� ������ ������
};
struct Print
{
    // ������� �print�
};
struct Indent
{
    // ������� ����������� �������, ������������� ���� ��������
};
struct Dedent
{
    // ������� ����������� �������
};
struct Eof
{
    // ������� ������ �����
};
struct And
{
    // ������� �and�
};
struct Or
{
    // ������� �or�
};
struct Not
{
    // ������� �not�
};
struct Eq
{
    // ������� �==�
};
struct NotEq
{
    // ������� �!=�
};
struct LessOrEq
{
    // ������� �<=�
};
struct GreaterOrEq
{
    // ������� �>=�
};
struct None
{
    // ������� �None�
};
struct True
{
    // ������� �True�
};
struct False
{
    // ������� �False�
};

}  // namespace token_type

using TokenBase
= std::variant<token_type::Number, token_type::Id, token_type::Char, token_type::String,
    token_type::Class, token_type::Return, token_type::If, token_type::Else,
    token_type::Def, token_type::Newline, token_type::Print, token_type::Indent,
    token_type::Dedent, token_type::And, token_type::Or, token_type::Not,
    token_type::Eq, token_type::NotEq, token_type::LessOrEq, token_type::GreaterOrEq,
    token_type::None, token_type::True, token_type::False, token_type::Eof>;

// ��������� Token - �������� ������� ���������� �������
struct Token : TokenBase
{
    // Makes available all std::variant constructors
    using TokenBase::TokenBase;

    template <typename T>
    [[nodiscard]] bool Is() const
    {
        return std::holds_alternative<T>(*this);
    }

    template <typename T>
    [[nodiscard]] const T& As() const
    {
        return std::get<T>(*this);
    }

    template <typename T>
    [[nodiscard]] const T* TryAs() const
    {
        return std::get_if<T>(this);
    }
};

bool operator==(const Token& lhs, const Token& rhs);
bool operator!=(const Token& lhs, const Token& rhs);

std::ostream& operator<<(std::ostream& os, const Token& rhs);

class LexerError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};


class Lexer
{
public:
    // ����������� ��� ���������� ������ �����
    explicit Lexer(std::istream& input);

    // ���������� ������ �� ������� ����� ��� token_type::Eof, ���� ����� ������� ����������
    [[nodiscard]] const Token& CurrentToken() const;

    // ���������� ��������� �����, ���� token_type::Eof, ���� ����� ������� ����������
    Token NextToken();

    // ���� ������� ����� ����� ��� T, ����� ���������� ������ �� ����.
    // � ��������� ������ ����� ����������� ���������� LexerError
    // USAGE SAMPLE:
    // lexer.Expect<token_type::Class>(); - checks that current token is Class
    template <typename T>
    const T& Expect() const
    {
        using namespace std::literals;
        if (!(*current_token_it_).Is<T>())
        {
            throw LexerError("Token::Expect() method has failed."s);
        }
        return CurrentToken().As<T>();
    }

    // ����� ���������, ��� ������� ����� ����� ��� T, � ��� ����� �������� �������� value.
    // � ��������� ������ ����� ����������� ���������� LexerError
    // USAGE SAMPLE:
    // lexer.Expect<token_type::Char>(':'); - checks that current token is Char with valie ':'
    template <typename T, typename U>
    void Expect(const U& value) const
    {
        using namespace std::literals;
        // ������� �� ������ value ������ ����� ���� T...
        Token other_token(T{ value });
        // ... � ���������� ��� �������� �� ��������� �������� ������
        if (*current_token_it_ != other_token)
        {
            throw LexerError("Token::Expect(value) method has failed."s);
        }
        // ������ ���, �����
    }

    // ���� ��������� ����� ����� ��� T, ����� ���������� ������ �� ����.
    // � ��������� ������ ����� ����������� ���������� LexerError
    // USAGE SAMPLE:
    // auto name = lexer.ExpectNext<token_type::Id>().value;
    template <typename T>
    const T& ExpectNext()
    {
        NextToken();
        return Expect<T>();
    }

    // ����� ���������, ��� ��������� ����� ����� ��� T, � ��� ����� �������� �������� value.
    // � ��������� ������ ����� ����������� ���������� LexerError
    // USAGE SAMPLE:
    // lexer.ExpectNext<token_type::Char>(':'); - checks that next token is Char with valie ':'
    template <typename T, typename U>
    void ExpectNext(const U& value)
    {
        NextToken();
        Expect<T>(value);
    }

private:
    // ���������� �������� �� 1 ������
    const int SPACES_PER_INDENT = 2;

    // ������� �������� ���� ��������� �� ����� Mython,
    // ������� ������������� ������������ �������
    const std::map<std::string, Token> keywords_map_ =
    {
        {std::string{"class"},  token_type::Class{} },
        {std::string{"return"}, token_type::Return{}},
        {std::string{"if"},     token_type::If{}    },
        {std::string{"else"},   token_type::Else{}  },
        {std::string{"def"},    token_type::Def{}   },
        {std::string{"print"},  token_type::Print{} },
        {std::string{"and"},    token_type::And{}   },
        {std::string{"or"},     token_type::Or{}    },
        {std::string{"not"},    token_type::Not{}   },
        {std::string{"None"},   token_type::None{}  },
        {std::string{"True"},   token_type::True{}  },
        {std::string{"False"},  token_type::False{} }
    };

    // ������ ������ ������������ ������ ��������� (��������� ������ �������)
    std::vector<Token> tokens_;
    // ��������, ����������� �� ������� �����
    std::vector<Token>::const_iterator current_token_it_;
    // Const ������ �� ����� ����� �������
    const std::istream& in_stream_;
    // ���������� ������� ��������. ��������, ��������� � ParseInputStream() ��� ������� static
    int global_indent_counter_ = 0;

    // ����� ����� ������� ������ ��������� �� �������
    void ParseInputStream(std::istream& istr);
    //������������� ����� � ��������� ���������
    //std::vector<Token>::const_iterator ParseInputStreamEx(std::istream& istr);

    // ��������� ��������
    void ParseIndent(std::istream& istr);
    // ��������� ����� � �������������� ��������
    void ParseString(std::istream& istr);
    // ��������� �������� ���� � ���������������
    void ParseKeywords(std::istream& istr);
    // ��������� ��������
    void ParseChars(std::istream& istr);
    // ��������� �����
    void ParseNumbers(std::istream& istr);
    // ��������� ������ �� ������� ����� ������
    void ParseNewLine(std::istream& istr);
    // ��������� ������������
    void ParseComments(std::istream& istr);

    // �������� ���������� �������
    void TrimSpaces(std::istream& istr);
};

}  // namespace parse