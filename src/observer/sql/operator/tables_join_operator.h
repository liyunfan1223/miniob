//
// Created by Clouds on 2022/9/24.
//

#pragma once

#include "sql/operator/operator.h"
#include "sql/operator/table_scan_operator.h"

class FilterStmt;

class TablesJoinPredOperator : public Operator
{
public:
//  TablesJoinPredOperator(std::vector<TableScanOperator*> scan_opers, FilterStmt *filter_stmt)
//  :scan_opers_(scan_opers), filter_stmt_(filter_stmt)
//  {
//    current_index_ = 0;
//    field_length_ = 0;
//    total_index_ = 1;
//  }

  TablesJoinPredOperator(std::vector<TableScanOperator*> scan_opers, FilterStmt *filter_stmt,
      Condition *conditions, size_t condition_num, std::unordered_map<std::string, char *> rel_alias)
      :scan_opers_(scan_opers), filter_stmt_(filter_stmt), conditions_(conditions), condition_num_(condition_num),
        rel_alias_(rel_alias)
  {
    current_index_ = 0;
    field_length_ = 0;
    total_index_ = 1;
  }
  virtual ~TablesJoinPredOperator() {
      for (auto scan_oper : scan_opers_) {
        delete scan_oper;
      }
      scan_opers_.clear();

      for (auto vec_record: records_) {
        for (auto record: vec_record) {
          delete record;
        }
        vec_record.clear();
      }
      records_.clear();
  };

  RC open() override;
  RC next() override;
  RC close() override;

  Tuple * current_tuple() override;
private:
  RC cartesian_product_dfs_(int table_index);
  bool is_match(const char * str, const char * pattern, size_t pattern_length);
  bool do_predicate_( std::vector<Record *> &records , int record_length);
  bool do_predicate_by_cond(Tuple &tuple);
  int32_t get_field_index_(FieldExpr * fieldExpr);
  std::vector<TableScanOperator*> scan_opers_;
  int32_t current_index_;
  int32_t total_index_;
  int32_t field_length_;
  std::vector<int32_t> field_lengths_;
  std::vector<std::pair<int32_t, int32_t>> index_mul_;
  std::vector<std::vector<Record *>> records_;
  std::vector<std::vector<Record *>> product_records_;
  std::vector<Record *> current_records_;
  JoinTuple tuple_;
  FilterStmt *filter_stmt_ = nullptr;
  std::vector<TupleCellSpec *> speces_;
  Condition *conditions_;
  size_t condition_num_;
  std::unordered_map<std::string, char *> rel_alias_;
};
