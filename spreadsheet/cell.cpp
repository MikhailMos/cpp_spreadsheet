#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <set>


Cell::Cell(Sheet& sheet)
	: impl_(std::make_unique<EmptyImpl>())
	, cache_(std::nullopt)
	, sheet_(sheet)
{}

Cell::~Cell() {
	impl_ = nullptr;	
}

void Cell::Set(std::string text, const Position pos) {
	
	if (text.empty()) {
		impl_ = std::make_unique<EmptyImpl>();
	}
	else if (text.size() > 1 && text.at(0) == FORMULA_SIGN) {
		impl_ = std::make_unique<FormulaImpl>();
	}
	else {
		impl_ = std::make_unique<TextImpl>();
	}

	impl_->Set(text);

	if (impl_->HasFormula()) {
		CheckCircularDependency(pos);		
	}

	Cell* old_cell = dynamic_cast<Cell*>(sheet_.GetCell(pos));

	if (old_cell) {
		// сбросим кэш
		UnvalidateCache(old_cell);

		//Удаляем старую ячейку из зависимых
		for (const Position pos_ref_cell : old_cell->GetReferencedCells()) {
			Cell* pre_cell = dynamic_cast<Cell*>(sheet_.GetCell(pos_ref_cell));
			pre_cell->DelBackLink(old_cell);
		}
	}
	
}

void Cell::Clear() {
	impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {

	if (cache_) {
		return cache_.value();
	}

	Cell::Value result = impl_->GetValue(sheet_);
	// установим кэш
	if (std::holds_alternative<double>(result)) {
		cache_ = std::get<double>(result);
	}

	return result;
}

std::string Cell::GetText() const {
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
	if (!impl_) {
		return {};
	}

	return impl_->GetReferencedCells();
}

void Cell::SetCache(double val) {
	cache_ = val;
}

void Cell::ClearCache() {
	cache_ = std::nullopt;
}

std::unordered_set<Cell*> Cell::GetDependedCells() {
	return dependen_cells_;
}

void Cell::AddBackLink(Cell* cell_ptr) noexcept {
	dependen_cells_.insert(std::move(cell_ptr));
}

void Cell::DelBackLink(Cell* cell_ptr) noexcept {
	dependen_cells_.erase(cell_ptr);
}

bool Cell::HasFormula() noexcept {
	return impl_->HasFormula();
}

namespace {
	// Реализация алгоритма поиска в глубину (DFS)
	void DFS(const Position pos, const std::vector<Position>& graph, const Sheet& sheet, std::set<Position>& visited) {
		
		if (visited.find(pos) == visited.end()) {
			visited.insert(pos);
		}
		else {
			throw CircularDependencyException("Circular dependency");
		}

		if (graph.empty()) {
			return;
		}
		
		// Обрабатываем текущий узел
		for (const Position pos : graph) {
			const auto node = sheet.GetCell(pos);
			if (!node) {
				return;
			}
				
			DFS(pos, node->GetReferencedCells(), sheet, visited);
			
		}
	}
}

void Cell::CheckCircularDependency(const Position pos) {
	
	std::set<Position> visitedDFS;
	DFS(pos, impl_->GetReferencedCells(), sheet_, visitedDFS);

}

void Cell::UnvalidateCache(Cell* cell_ptr) {

	for (Cell* curr_cell_ptr : cell_ptr->dependen_cells_) {
		if (!curr_cell_ptr->cache_) {
			continue;
		}

		curr_cell_ptr->ClearCache();

		UnvalidateCache(curr_cell_ptr);
	}

}

void Cell::EmptyImpl::Set(std::string text) {
	// ни чего не делаем
}

Cell::Value Cell::EmptyImpl::GetValue(const Sheet& sheet) const {
	return text_;
}

std::string Cell::EmptyImpl::GetText() const {
	return text_;
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
	return std::vector<Position>();
}

bool Cell::EmptyImpl::HasFormula() const {
	return false;
}

void Cell::TextImpl::Set(std::string text) {
	text_ = std::move(text);
}

Cell::Value Cell::TextImpl::GetValue(const Sheet& sheet) const{
	if (!text_.empty() && text_.at(0) == ESCAPE_SIGN) {
		return text_.substr(1);
	}
		
	return text_;
}

std::string Cell::TextImpl::GetText() const {
	return text_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
	return std::vector<Position>();
}

bool Cell::TextImpl::HasFormula() const {
	return false;
}

void Cell::FormulaImpl::Set(std::string text) {
	try {
		// добавляем без знака "="
		formula_ptr_ = ParseFormula(std::move(text.substr(1)));
	}
	catch (std::exception& e) {
		throw FormulaException(e.what());
	}
}

Cell::Value Cell::FormulaImpl::GetValue(const Sheet& sheet) const {
	
	FormulaInterface::Value result = formula_ptr_->Evaluate(sheet);

	if (std::holds_alternative<double>(result)) {
		return std::get<double>(result);
	}
	else if (std::holds_alternative<FormulaError>(result)) {
		return std::get<FormulaError>(result);
	}
	// заглушка, на случай, если в FormulaInterface::Value добавятся варианты
	return std::string{"Получено не определенное значение"};
}

std::string Cell::FormulaImpl::GetText() const {
	return (FORMULA_SIGN + formula_ptr_->GetExpression());
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {		
	return formula_ptr_->GetReferencedCells();
}

bool Cell::FormulaImpl::HasFormula() const {
	return true;
}
