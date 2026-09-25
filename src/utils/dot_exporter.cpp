#include "execore/utils/dot_exporter.hpp"
#include "execore/ast/expressions.hpp"
#include "execore/ast/statements.hpp"
#include "execore/ast/visitor.hpp"
#include <fstream>
#include <sstream>

namespace execore {

namespace {

class DotVisitor : public ASTVisitor {
public:
    explicit DotVisitor(std::ostream& out) : out_(out) {}

    size_t last_id() const noexcept { return current_id_; }

    void visit(LiteralExpr& node) override {
        size_t id = next_id();
        std::string val = escape(node.value());
        out_ << "  node_" << id << " [label=\"Literal (" << to_string(node.literal_type())
             << "): " << val << "\", shape=ellipse, style=filled, fillcolor=\"#fff2cc\", color=\"#d6b656\"];\n";
    }

    void visit(IdentifierExpr& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Identifier: " << escape(node.name())
             << "\", shape=ellipse, style=filled, fillcolor=\"#dae8fc\", color=\"#6c8ebf\"];\n";
    }

    void visit(UnaryExpr& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Unary (" << to_string(node.op())
             << ")\", shape=box, style=filled, fillcolor=\"#d5e8d4\", color=\"#82b366\"];\n";
        if (node.operand()) {
            node.operand()->accept(*this);
            connect(id, last_id());
        }
    }

    void visit(BinaryExpr& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Binary (" << to_string(node.op())
             << ")\", shape=box, style=filled, fillcolor=\"#d5e8d4\", color=\"#82b366\"];\n";
        if (node.left()) {
            node.left()->accept(*this);
            connect(id, last_id(), "left");
        }
        if (node.right()) {
            node.right()->accept(*this);
            connect(id, last_id(), "right");
        }
    }

    void visit(AssignExpr& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Assign (" << to_string(node.op())
             << ")\", shape=box, style=filled, fillcolor=\"#ffe6cc\", color=\"#d79b00\"];\n";
        if (node.target()) {
            node.target()->accept(*this);
            connect(id, last_id(), "target");
        }
        if (node.value()) {
            node.value()->accept(*this);
            connect(id, last_id(), "value");
        }
    }

    void visit(CallExpr& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Call: " << escape(node.callee())
             << "()\", shape=box, style=filled, fillcolor=\"#e1d5e7\", color=\"#9673a6\"];\n";
        for (size_t i = 0; i < node.args().size(); ++i) {
            node.args()[i]->accept(*this);
            connect(id, last_id(), "arg_" + std::to_string(i));
        }
    }

    void visit(IndexExpr& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Index []\", shape=box, style=filled, fillcolor=\"#dae8fc\"];\n";
        if (node.sequence()) {
            node.sequence()->accept(*this);
            connect(id, last_id(), "seq");
        }
        if (node.index()) {
            node.index()->accept(*this);
            connect(id, last_id(), "idx");
        }
    }

    void visit(SliceExpr& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Slice [:]\", shape=box, style=filled, fillcolor=\"#dae8fc\"];\n";
        if (node.sequence()) {
            node.sequence()->accept(*this);
            connect(id, last_id(), "seq");
        }
        if (node.start()) {
            node.start()->accept(*this);
            connect(id, last_id(), "start");
        }
        if (node.end()) {
            node.end()->accept(*this);
            connect(id, last_id(), "end");
        }
    }

    void visit(MethodCallExpr& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Method: ." << escape(node.method_name())
             << "()\", shape=box, style=filled, fillcolor=\"#e1d5e7\", color=\"#9673a6\"];\n";
        if (node.object()) {
            node.object()->accept(*this);
            connect(id, last_id(), "obj");
        }
        for (size_t i = 0; i < node.args().size(); ++i) {
            node.args()[i]->accept(*this);
            connect(id, last_id(), "arg_" + std::to_string(i));
        }
    }

    void visit(CommaExpr& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"CommaExpr\", shape=box];\n";
        for (const auto& expr : node.expressions()) {
            expr->accept(*this);
            connect(id, last_id());
        }
    }

    void visit(ExprStmt& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"ExprStmt\", shape=box, style=\"rounded,filled\", fillcolor=\"#f5f5f5\"];\n";
        if (node.expr()) {
            node.expr()->accept(*this);
            connect(id, last_id());
        }
    }

    void visit(BlockStmt& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Block\", shape=box, style=\"rounded,filled\", fillcolor=\"#e6e6e6\"];\n";
        for (const auto& stmt : node.statements()) {
            stmt->accept(*this);
            connect(id, last_id());
        }
    }

    void visit(VarDeclStmt& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"VarDecl: " << to_string(node.type())
             << "\", shape=box, style=\"rounded,filled\", fillcolor=\"#d5e8d4\", color=\"#82b366\"];\n";
        for (const auto& v : node.variables()) {
            size_t vid = next_id();
            out_ << "  node_" << vid << " [label=\"" << escape(v.name) << "\", shape=ellipse];\n";
            connect(id, vid);
            if (v.init_value) {
                v.init_value->accept(*this);
                connect(vid, last_id(), "init");
            }
        }
    }

    void visit(FunctionDeclStmt& node) override {
        size_t id = next_id();
        std::string params;
        for (size_t i = 0; i < node.params().size(); ++i) {
            if (i > 0) params += ", ";
            params += node.params()[i];
        }
        out_ << "  node_" << id << " [label=\"Def " << escape(node.name()) << "(" << escape(params)
             << ")\", shape=box, style=\"rounded,filled\", fillcolor=\"#d5e8d4\", color=\"#82b366\"];\n";
        if (node.body()) {
            node.body()->accept(*this);
            connect(id, last_id(), "body");
        }
    }

    void visit(IfStmt& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"If\", shape=diamond, style=filled, fillcolor=\"#fff2cc\", color=\"#d6b656\"];\n";
        if (node.condition()) {
            node.condition()->accept(*this);
            connect(id, last_id(), "cond");
        }
        if (node.then_branch()) {
            node.then_branch()->accept(*this);
            connect(id, last_id(), "then");
        }
        if (node.else_branch()) {
            node.else_branch()->accept(*this);
            connect(id, last_id(), "else");
        }
    }

    void visit(WhileStmt& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"While\", shape=diamond, style=filled, fillcolor=\"#fff2cc\", color=\"#d6b656\"];\n";
        if (node.condition()) {
            node.condition()->accept(*this);
            connect(id, last_id(), "cond");
        }
        if (node.body()) {
            node.body()->accept(*this);
            connect(id, last_id(), "body");
        }
    }

    void visit(DoWhileStmt& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"DoWhile\", shape=diamond, style=filled, fillcolor=\"#fff2cc\", color=\"#d6b656\"];\n";
        if (node.body()) {
            node.body()->accept(*this);
            connect(id, last_id(), "body");
        }
        if (node.condition()) {
            node.condition()->accept(*this);
            connect(id, last_id(), "cond");
        }
    }

    void visit(ForStmt& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"For: " << escape(node.var_name())
             << " in\", shape=diamond, style=filled, fillcolor=\"#fff2cc\", color=\"#d6b656\"];\n";
        if (node.sequence()) {
            node.sequence()->accept(*this);
            connect(id, last_id(), "seq");
        }
        if (node.body()) {
            node.body()->accept(*this);
            connect(id, last_id(), "body");
        }
    }

    void visit(ReturnStmt& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Return\", shape=box, style=\"rounded,filled\", fillcolor=\"#f8cecc\", color=\"#b85450\"];\n";
        if (node.value()) {
            node.value()->accept(*this);
            connect(id, last_id());
        }
    }

    void visit(BreakStmt&) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Break\", shape=box, style=\"rounded,filled\", fillcolor=\"#f8cecc\", color=\"#b85450\"];\n";
    }

    void visit(ContinueStmt&) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Continue\", shape=box, style=\"rounded,filled\", fillcolor=\"#f8cecc\", color=\"#b85450\"];\n";
    }

    void visit(PassStmt&) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Pass\", shape=box, style=\"rounded,filled\"];\n";
    }

    void visit(PrintStmt& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Print" << (node.is_raw() ? " (raw)" : "")
             << "\", shape=box, style=\"rounded,filled\", fillcolor=\"#dae8fc\", color=\"#6c8ebf\"];\n";
        for (const auto& expr : node.expressions()) {
            expr->accept(*this);
            connect(id, last_id());
        }
    }

    void visit(InputStmt& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Input\", shape=box, style=\"rounded,filled\", fillcolor=\"#dae8fc\", color=\"#6c8ebf\"];\n";
        for (const auto& item : node.items()) {
            size_t item_id = next_id();
            out_ << "  node_" << item_id << " [label=\"" << escape(item.var_name)
                 << " (prompt: " << escape(item.prompt) << ")\", shape=ellipse];\n";
            connect(id, item_id);
        }
    }

    void visit(ImportStmt& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Import\", shape=box, style=\"rounded,filled\", fillcolor=\"#e1d5e7\"];\n";
        for (const auto& mod : node.modules()) {
            mod->accept(*this);
            connect(id, last_id());
        }
    }

    void visit(Program& node) override {
        size_t id = next_id();
        out_ << "  node_" << id << " [label=\"Program\", shape=box, style=\"rounded,filled\", fillcolor=\"#bbdefb\", color=\"#1976d2\"];\n";
        for (const auto& stmt : node.statements()) {
            stmt->accept(*this);
            connect(id, last_id());
        }
    }

private:
    size_t next_id() noexcept { return ++current_id_; }

    void connect(size_t from, size_t to, const std::string& label = "") {
        out_ << "  node_" << from << " -> node_" << to;
        if (!label.empty()) {
            out_ << " [label=\"" << label << "\"]";
        }
        out_ << ";\n";
    }

    static std::string escape(const std::string& str) {
        std::string res;
        for (char c : str) {
            if (c == '"' || c == '\\') res.push_back('\\');
            if (c == '\n') { res += "\\n"; continue; }
            if (c == '\r') { res += "\\r"; continue; }
            if (c == '\t') { res += "\\t"; continue; }
            res.push_back(c);
        }
        return res;
    }

    std::ostream& out_;
    size_t current_id_{0};
};

} // namespace

void DotExporter::export_to_stream(const Program& program, std::ostream& out) {
    out << "digraph ExecoreAST {\n";
    out << "  node [fontname=\"Helvetica,Arial,sans-serif\", fontsize=10];\n";
    out << "  edge [fontname=\"Helvetica,Arial,sans-serif\", fontsize=9];\n";
    DotVisitor visitor(out);
    const_cast<Program&>(program).accept(visitor);
    out << "}\n";
}

void DotExporter::export_to_file(const Program& program, const std::string& filepath) {
    std::ofstream file(filepath);
    if (file) {
        export_to_stream(program, file);
    }
}

} // namespace execore
