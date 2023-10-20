#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <vector>

class Sheet : public SheetInterface {
public:

    using DataCells = std::vector<std::vector<std::shared_ptr<Cell*>>>;

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    
private:	
    DataCells cells_;
    
    // если позиция больше размера текущей области, тогда изменим размер
    void CellsAddSize(const Position pos);

    bool PositionInPrintArea(const Position pos) const;

};