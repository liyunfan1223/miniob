//
// Created by Clouds on 2022/9/24.
//

#include "tables_join_operator.h"
#include "table_scan_operator.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/common/field.h"
#include "util/comparator.h"

RC TablesJoinPredOperator::open()
{
  for (auto it = scan_opers_.rbegin(); it != scan_opers_.rend(); it++)
  {
    auto &scan_oper = *it;
    std::vector<Record *> table_records;
    RC rc = scan_oper->open();
    if (rc == RC::SUCCESS) {
      while (scan_oper->next() == RC::SUCCESS) {
        auto tuple = static_cast<RowTuple*>(scan_oper->current_tuple());
        table_records.push_back(new Record(tuple->record().rid(), tuple->record().data()));
      }

      for (auto &field : *scan_oper->table()->table_meta().field_metas())
      {
        speces_.push_back(new TupleCellSpec(new FieldExpr(scan_oper->table(), &field)));
      }

      tuple_.add_schema(scan_oper->table(), scan_oper->table()->table_meta().field_metas());
      tuple_.add_sum(field_length_);

      field_length_ += scan_oper->table()->table_meta().field_num();
      field_lengths_.push_back(field_length_);

      index_mul_.push_back({(int32_t)table_records.size(), total_index_});
      total_index_ *= table_records.size();
      records_.push_back(table_records);
    } else {
      return rc;
    }
  }

  current_records_.clear();
  cartesian_product_dfs_(0);
  return RC::SUCCESS;
}

RC TablesJoinPredOperator::next()
{
  if (current_index_ >= (int32_t)product_records_.size()) {
    return RC::RECORD_EOF;
  }
  current_records_ = product_records_[current_index_];
  current_index_++;
  return RC::SUCCESS;
}

RC TablesJoinPredOperator::close()
{
  RC rc = RC::SUCCESS;
  for (auto scan_oper : scan_opers_) {
    if ((rc = scan_oper->close()) != RC::SUCCESS) {
      return rc;
    }
  }
  return rc;
}


Tuple *TablesJoinPredOperator::current_tuple()
{
  tuple_.set_records(&current_records_);
  return new JoinTuple(tuple_);
}

RC TablesJoinPredOperator::cartesian_product_dfs_(int table_index)
{
  // 剪枝
  if (!do_predicate_(current_records_,  table_index == 0 ? 0 : field_lengths_[table_index - 1])) {
    return SUCCESS;
  }
  // 回溯
  if (table_index == (int32_t)field_lengths_.size()) {
    product_records_.push_back(current_records_);
    return SUCCESS;
  }
  // 遍历
  auto current_row = records_[table_index];
  for (auto rec : records_[table_index]) {
    current_records_.push_back(rec);
    cartesian_product_dfs_(table_index + 1);
    current_records_.pop_back();
  }
  return SUCCESS;
}

int32_t TablesJoinPredOperator::get_field_index_(FieldExpr * fieldExpr)
{
  const char *table_name = fieldExpr->field().table_name();
  const char *field_name = fieldExpr->field().field_name();
  for (size_t i = 0; i < speces_.size(); ++i) {
    const FieldExpr * field_expr = (const FieldExpr *)speces_[i]->expression();
    const Field &field = field_expr->field();
    if ((0 == strcmp(table_name, field.table_name())) and (0 == strcmp(field_name, field.field_name()))) {
      return i;
    }
  }
  return INT32_MAX;
}

bool TablesJoinPredOperator::is_match(const char * str, const char * pattern, size_t pattern_length)
{
  size_t str_length = strlen(str);
  bool match_dp[str_length + 1][pattern_length + 1];
  memset(match_dp, 0, sizeof(match_dp));
  match_dp[0][0] = 1;
  for (size_t i = 0; i <= str_length; i++) {
    for (size_t j = 1; j <= pattern_length; j++) {
      switch (pattern[j - 1]) {
        case '%': {
          for (size_t k = 0; k <= i; k++) {
            if (match_dp[k][j - 1] == 1) {
              match_dp[i][j] = 1;
            }
          }
        } break;
        case '_': {
          if (i >= 1) {
            match_dp[i][j] = match_dp[i - 1][j - 1];
          }
        } break;
        default: {
          if (i >= 1 && pattern[j - 1] == str[i - 1]) {
            match_dp[i][j] = match_dp[i - 1][j - 1];
          }
        }
      }
    }
  }
  return match_dp[str_length][pattern_length];
}

bool TablesJoinPredOperator::do_predicate_( std::vector<Record *> &records, int record_length)
{
  if (filter_stmt_ == nullptr || filter_stmt_->filter_units().empty()) {
    return true;
  }

  for (const FilterUnit *filter_unit : filter_stmt_->filter_units()) {
    Expression *left_expr = filter_unit->left();
    Expression *right_expr = filter_unit->right();
    // 如果表达式为字段，且当前不可对其判断（未访问到相关表），则跳过
    if (left_expr->type() == ExprType::FIELD && get_field_index_((FieldExpr *)left_expr) >= record_length) {
      continue;
    }
    if (right_expr->type() == ExprType::FIELD && get_field_index_((FieldExpr *)right_expr) >= record_length) {
      continue;
    }

    tuple_.set_records(&records);
    CompOp comp = filter_unit->comp();
    TupleCell left_cell;
    TupleCell right_cell;
    left_expr->get_value(tuple_, left_cell);
    right_expr->get_value(tuple_, right_cell);
    if (!left_cell.condition_satisfy(comp, right_cell)) {
      return false;
    }
  }
  return true;
}