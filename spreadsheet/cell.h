#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <functional>
#include <unordered_set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text, const Position pos);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    void SetCache(double val);
    void ClearCache();

    // получает список обратных зависимостей
    std::unordered_set<Cell*> GetDependedCells();
    // добавляет обратную зависимость
    void AddBackLink(Cell* cell_ptr) noexcept;
    // удаляет обратную зависимость
    void DelBackLink(Cell* cell_ptr) noexcept;

    bool HasFormula() noexcept;

private:
//можете воспользоваться нашей подсказкой, но это необязательно.
    class Impl {
    public:
        virtual ~Impl() = default;
        virtual void Set(std::string text) = 0;
        virtual Value GetValue(const Sheet& sheet) const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
        virtual bool HasFormula() const = 0;
    };
    
    class EmptyImpl final : public Impl {
    public:
        void Set(std::string text) override;
        Value GetValue(const Sheet& sheet) const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        bool HasFormula() const override;
    private:
        std::string text_;
    };
    
    class TextImpl final : public Impl {
    public:
        void Set(std::string text) override;
        Value GetValue(const Sheet& sheet) const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        bool HasFormula() const override;
    private:
        std::string text_;
    };

    class FormulaImpl final : public Impl {
    public:
        void Set(std::string text) override;
        Value GetValue(const Sheet& sheet) const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        bool HasFormula() const override;
    private:
        // не знаю, возможно правильнее хранить строку
        std::unique_ptr<FormulaInterface> formula_ptr_;        
       
    };

    std::unique_ptr<Impl> impl_;
    mutable std::optional<double> cache_;
    Sheet& sheet_;
    std::unordered_set<Cell*> dependen_cells_; // хранит указатели на ячейки для которых нужно будет сбрасывать кэш в случае изменения текущей ячейки

    // если есть зацикливание, тогда выбросит исключение: CircularDependencyException
    void CheckCircularDependency(const Position pos);
    // сбросим кэш у зависимых ячеек
    void UnvalidateCache(Cell* cell);
};