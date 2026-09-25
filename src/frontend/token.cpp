#include "execore/frontend/token.hpp"

namespace execore {

std::string_view to_string(TokenKind kind) noexcept {
    switch (kind) {
        case TokenKind::Eof:           return "EOF";
        case TokenKind::Newline:       return "NEWLINE";
        case TokenKind::Indent:        return "INDENT";
        case TokenKind::Dedent:        return "DEDENT";
        case TokenKind::IntLiteral:    return "integer literal";
        case TokenKind::FloatLiteral:  return "float literal";
        case TokenKind::CharLiteral:   return "character literal";
        case TokenKind::StringLiteral: return "string literal";
        case TokenKind::Identifier:    return "identifier";
        case TokenKind::KwChar:        return "'char'";
        case TokenKind::KwInt:         return "'int'";
        case TokenKind::KwFloat:       return "'float'";
        case TokenKind::KwStr:         return "'str'";
        case TokenKind::KwList:        return "'list'";
        case TokenKind::KwDef:         return "'def'";
        case TokenKind::KwIf:          return "'if'";
        case TokenKind::KwElse:        return "'else'";
        case TokenKind::KwWhile:       return "'while'";
        case TokenKind::KwDo:          return "'do'";
        case TokenKind::KwFor:         return "'for'";
        case TokenKind::KwIn:          return "'in'";
        case TokenKind::KwReturn:      return "'return'";
        case TokenKind::KwPass:        return "'pass'";
        case TokenKind::KwBreak:       return "'break'";
        case TokenKind::KwContinue:    return "'continue'";
        case TokenKind::KwPrint:       return "'print'";
        case TokenKind::KwInput:       return "'input'";
        case TokenKind::KwImport:      return "'import'";
        case TokenKind::KwAnd:         return "'and'";
        case TokenKind::KwOr:          return "'or'";
        case TokenKind::KwNot:         return "'not'";
        case TokenKind::Plus:          return "'+'";
        case TokenKind::Minus:         return "'-'";
        case TokenKind::Star:          return "'*'";
        case TokenKind::Slash:         return "'/'";
        case TokenKind::Percent:       return "'%'";
        case TokenKind::Assign:        return "'='";
        case TokenKind::PlusAssign:    return "'+='";
        case TokenKind::MinusAssign:   return "'-='";
        case TokenKind::StarAssign:    return "'*='";
        case TokenKind::SlashAssign:   return "'/='";
        case TokenKind::PercentAssign: return "'%='";
        case TokenKind::EqualEqual:    return "'=='";
        case TokenKind::NotEqual:      return "'!='";
        case TokenKind::Less:          return "'<'";
        case TokenKind::LessEqual:     return "'<='";
        case TokenKind::Greater:       return "'>'";
        case TokenKind::GreaterEqual:  return "'>='";
        case TokenKind::Bang:          return "'!'";
        case TokenKind::Comma:         return "','";
        case TokenKind::Colon:         return "':'";
        case TokenKind::Dot:           return "'.'";
        case TokenKind::LeftParen:     return "'('";
        case TokenKind::RightParen:    return "')'";
        case TokenKind::LeftBracket:   return "'['";
        case TokenKind::RightBracket:  return "']'";
        case TokenKind::Unknown:       return "unknown token";
    }
    return "unknown token";
}

} // namespace execore
