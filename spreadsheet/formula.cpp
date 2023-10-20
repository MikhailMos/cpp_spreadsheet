#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression)
        : ast_(ParseFormulaAST(expression))
    {}
    
    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            return ast_.Execute(sheet);
        }
        catch (const FormulaError& exc) {
            return exc;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream out;        
        ast_.PrintFormula(out);

        return out.str();
    }

    std::vector<Position> GetReferencedCells() const {
        
        const std::forward_list<Position> ref_cells = ast_.GetCells();

        std::vector<Position> result;
        
        if (ref_cells.empty()) {
            return result;
        }

        std::transform(ref_cells.begin(), ref_cells.end(), std::back_inserter(result),
            [](const Position& pos) { return pos; });

        //std::sort(result.begin(), result.end()); // сортировка уже есть в конструкторе FormulaAST, оставил на всякий случай
        result.erase(std::unique(result.begin(), result.end()), result.end());  // удаляем дубликаты
        
        return result;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (std::exception& e) {
        throw FormulaException(e.what());
    }
}