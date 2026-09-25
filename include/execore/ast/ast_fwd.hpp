#pragma once

namespace execore {

// Base
class ASTNode;
class Expr;
class Stmt;

// Expressions
class LiteralExpr;
class IdentifierExpr;
class UnaryExpr;
class BinaryExpr;
class AssignExpr;
class CallExpr;
class IndexExpr;
class SliceExpr;
class MethodCallExpr;
class CommaExpr;

// Statements
class ExprStmt;
class BlockStmt;
class VarDeclStmt;
class FunctionDeclStmt;
class IfStmt;
class WhileStmt;
class DoWhileStmt;
class ForStmt;
class ReturnStmt;
class BreakStmt;
class ContinueStmt;
class PassStmt;
class PrintStmt;
class InputStmt;
class ImportStmt;
class Program;

// Visitor
class ASTVisitor;

} // namespace execore
