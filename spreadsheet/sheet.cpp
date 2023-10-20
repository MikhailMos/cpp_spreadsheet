#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
        
    if (!pos.IsValid()) {
        throw InvalidPositionException("SetCell method: Position is out of range");
    }

    std::shared_ptr<Cell> cell_ptr = std::make_shared<Cell>(*this);
    cell_ptr->Set(text, pos);

    if (cell_ptr->HasFormula()) {
        //Добавляем новую ячейку в зависимые ячейки
        for (const Position pos_ref_cell : cell_ptr->GetReferencedCells()) {
            Cell* pre_cell = dynamic_cast<Cell*>(Sheet::GetCell(pos_ref_cell));
            if (pre_cell) {
                pre_cell->AddBackLink(cell_ptr.get());
            }
        }
    }

    CellsAddSize(pos);

    // добавим ячейку по позиции строка/колонка
    cells_[pos.row][pos.col] = std::move(cell_ptr);

}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("GetCell method: Position is out of range");
    }

    if (!PositionInPrintArea(pos)) {
        return nullptr;
    }

    return cells_[pos.row][pos.col].get();   
}

CellInterface* Sheet::GetCell(Position pos) {    

    if (!pos.IsValid()) {
        throw InvalidPositionException("GetCell method: Position is out of range");
    }

    if (!PositionInPrintArea(pos)) {
        return nullptr;
    }

    return cells_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
    
    if (!pos.IsValid()) {
        throw InvalidPositionException("ClearCell method: Position is out of range");
    }

    if (!PositionInPrintArea(pos)
        || !cells_[pos.row][pos.col])
    {
        return;
    }
    
    if (cells_[pos.row][pos.col]) {

        Size size_table_befor = GetPrintableSize();

        cells_[pos.row][pos.col].reset();

        Size size_table_after = GetPrintableSize();
        
        if (size_table_befor.rows != size_table_after.rows) {
            cells_.resize(size_table_after.rows);
        }

        if (size_table_befor.cols != size_table_after.cols) {
            for (int i = 0; i < size_table_after.rows; ++i) {
                cells_[i].resize(size_table_after.cols);
            }
        }
        
    }

}

Size Sheet::GetPrintableSize() const {
    
   Size result;
    // начинаем с конца, чтоб было меньше присваиваний
    for (int row = static_cast<int>(cells_.size()) - 1; row >= 0; --row) {    
        
        for (int col = static_cast<int>(cells_[row].size()) - 1; col >= 0; --col) {
            if (cells_[row][col] && (cells_[row][col])->GetText().size()) {
                result.rows = std::max(result.rows, row + 1);
                result.cols = std::max(result.cols, col + 1);
            }
        }
        
    }

    return result;

}

namespace {

    struct print_visitor {
        std::ostream& os;

        print_visitor(std::ostream& os) : os(os) {}

        void operator()(const std::string& v) const { os << v; }
        void operator()(double v) const { os << v; }
        void operator()(const FormulaError& v) const { os << v; }
    };

}

void Sheet::PrintValues(std::ostream& output) const {
    
    Size size_table = GetPrintableSize();
    
    for (int i = 0; i < size_table.rows; ++i) {
        for (int j = 0; j < size_table.cols; ++j) {
            if (j > 0) {
                output << '\t';
            }
            if (cells_[i][j]) {
                std::visit(print_visitor(output), (cells_[i][j])->GetValue());
            }
        }
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    
    Size size_table = GetPrintableSize();
    
    for (int i = 0; i < size_table.rows; ++i) {
        for (int j = 0; j < size_table.cols; ++j) {
            if (j > 0) {
                output << '\t';
            }
            if (cells_[i][j]) {
                output << (cells_[i][j])->GetText();
            }
        }
        output << '\n';
    }
}

void Sheet::CellsAddSize(const Position pos) {

    Size size_table = GetPrintableSize();

    if (pos.row > (size_table.rows - 1)) {        
        
        cells_.resize(pos.row + 1);
        
        // проходим по строкам и заполняем пустыми ячейками
        for (int i = size_table.rows; i <= pos.row; ++i) {
            if (pos.col > (size_table.cols - 1)) {
                cells_.at(i).resize(pos.col + 1);
            } 
            else {
                cells_.at(i).resize(size_table.cols);
            }

            for (int j = size_table.cols; j <= pos.col; ++j) {
                cells_[i][j] = std::make_shared<Cell>(*this);
            }
        }

    }

    if (pos.col > (size_table.cols - 1)) {

        for (int i = 0; i <= pos.row; ++i) {
            cells_.at(i).resize(pos.col + 1);

            for (int j = size_table.cols; j <= pos.col; ++j) {
                cells_[i][j] = std::make_shared<Cell>(*this);
            }
        }

    }

}

bool Sheet::PositionInPrintArea(const Position pos) const {
    Size size_table = GetPrintableSize();
    return ((pos.row >=0 && pos.row < size_table.rows) && (pos.col >= 0 && pos.col < size_table.cols));
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
