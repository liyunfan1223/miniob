/* Copyright (c) 2021 Xie Meiyi(xiemeiyi@hust.edu.cn) and OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Meiyi
//

#ifndef __OBSERVER_SQL_PARSER_PARSE_DEFS_H__
#define __OBSERVER_SQL_PARSER_PARSE_DEFS_H__

#include <stddef.h>

#define MAX_NUM 20
#define MAX_REL_NAME 20
#define MAX_ATTR_NAME 20
#define MAX_ERROR_MESSAGE 20
#define MAX_DATA 50

typedef struct _Selects Selects;
typedef struct _Value Value;
typedef struct _RelAttr RelAttr;

typedef enum {
  PLUS_ET,
  MINUS_ET,
  MUL_ET,
  DIVIDE_ET,
  LBRACE_ET,
  RBRACE_ET,
  VALUE_ET,
  ATTR_ET
} ExpressionType;

typedef struct _ExpElement {
//  _ExpElement() {}
//  _ExpElement(ExpressionType type, Value * value, RelAttr * rel_attr):
//  type(type), value(value), rel_attr(rel_attr){}
  ExpressionType type;
  Value * value;
  RelAttr * rel_attr;
} ExpElement;

typedef enum {
  AGG_MAX,
  AGG_MIN,
  AGG_COUNT,
  AGG_AVG,
  AGG_SUM,
  AGG_NONE,
} AggType;

typedef enum {
  FUNC_ROUND,
  FUNC_DATE,
  FUNC_LENGTH,
  FUNC_NONE,
} FuncType;

//属性结构体
typedef struct _RelAttr{
  char *relation_name;   // relation name (may be NULL) 表名
  char *attribute_name;  // attribute name              属性名
  char *alias_name;
  int is_agg;
  AggType aggType;
  int is_exp;
  char *expression;
  FuncType func_type;
  int round_func_param;
  char * date_func_param;
  Value * non_table_value;
} RelAttr;

typedef struct {
  char *relation_name;   // relation name (may be NULL) 表名
  char *attribute_name;  // attribute name              属性名
} GroupAttr;

typedef enum { OrderAsc, OrderDesc } OrderType;

typedef struct {
  char *relation_name;   // relation name (may be NULL) 表名
  char *attribute_name;  // attribute name              属性名
  OrderType type;
} OrderAttr;

typedef enum {
  EQUAL_TO,     //"="     0
  LESS_EQUAL,   //"<="    1
  NOT_EQUAL,    //"<>"    2
  LESS_THAN,    //"<"     3
  GREAT_EQUAL,  //">="    4
  GREAT_THAN,   //">"     5
  LIKE_TO,
  NOT_LIKE_TO,
  IN_OP,
  NOT_IN_OP,
  EXISTS_OP,
  NOT_EXISTS_OP,
  IS_OP,
  IS_NOT_OP,
  NO_OP
} CompOp;

//属性值类型
typedef enum
{
  UNDEFINED,
  CHARS,
  INTS,
  FLOATS,
  DATES,
  TEXTS,
  NULL_T,
  EXPRESSION_T,
} AttrType;

typedef enum {
  CONJ_FIRST,
  CONJ_AND,
  CONJ_OR
} Conjunction;

typedef struct _Value {
  AttrType type;  // type of value
  void *data;     // value
  size_t is_null;
  int is_sub_select;
  Selects * selects;
  int is_set;
  int set_length;
  Value * set_values;
  char * expression;
} Value;

typedef struct _Condition {
  int left_is_attr;    // TRUE if left-hand side is an attribute
                       // 1时，操作符左边是属性名，0时，是属性值
  Value left_value;    // left-hand side value if left_is_attr = FALSE
  RelAttr left_attr;   // left-hand side attribute
  CompOp comp;         // comparison operator
  int right_is_attr;   // TRUE if right-hand side is an attribute
                       // 1时，操作符右边是属性名，0时，是属性值
  RelAttr right_attr;  // right-hand side attribute if right_is_attr = TRUE 右边的属性
  Value right_value;   // right-hand side value if right_is_attr = FALSE

  Conjunction conj;
} Condition;



// struct of select
typedef struct _Selects {
  size_t attr_num;                // Length of attrs in Select clause
  RelAttr attributes[MAX_NUM];    // attrs in Select clause
  size_t relation_num;            // Length of relations in Fro clause
  char *relations[MAX_NUM];       // relations in From clause
  char *relations_alias[MAX_NUM];
  size_t condition_num;           // Length of conditions in Where clause
  Condition conditions[MAX_NUM];  // conditions in Where clause
  size_t group_num;
  GroupAttr group_attributes[MAX_NUM];    // attrs in Select clause
  size_t order_num;
  OrderAttr order_attributes[MAX_NUM];
  size_t having_num;
  Condition having_conditions[MAX_NUM];
} Selects;

typedef struct {
  size_t value_num;       // Length of values
  Value values[MAX_NUM];  // values to insert
} InsertRecord;

// struct of insert
typedef struct {
  char *relation_name;    // Relation to insert into
  size_t record_num;
  InsertRecord records[MAX_NUM];
} Inserts;

// struct of delete
typedef struct {
  char *relation_name;            // Relation to delete from
  size_t condition_num;           // Length of conditions in Where clause
  Condition conditions[MAX_NUM];  // conditions in Where clause
} Deletes;

// struct of update
typedef struct {
  char *relation_name;            // Relation to update
  char *attribute_name[MAX_NUM];           // Attribute to update
  size_t attr_num;
  Value value[MAX_NUM];                    // update value
  size_t value_num;
  size_t condition_num;           // Length of conditions in Where clause
  Condition conditions[MAX_NUM];  // conditions in Where clause
} Updates;

typedef struct {
  char *name;     // Attribute name
  AttrType type;  // Type of attribute
  size_t length;  // Length of attribute
  size_t nullable;
} AttrInfo;

// struct of craete_table
typedef struct {
  char *relation_name;           // Relation name
  size_t attribute_count;        // Length of attribute
  AttrInfo attributes[MAX_NUM];  // attributes
} CreateTable;

// struct of drop_table
typedef struct {
  char *relation_name;  // Relation name
} DropTable;

// struct of create_index
typedef struct {
  char * index_name;      // Index name
  char * relation_name;   // Relation name
//  char * attribute_name;  // Attribute name
  char * attribute_name[MAX_NUM];  // Attribute name
  size_t attr_num;
  size_t is_unique;
} CreateIndex;

// struct of  drop_index
typedef struct {
  const char *index_name;  // Index name
} DropIndex;

typedef struct {
  const char *relation_name;
} DescTable;

typedef struct {
  const char *relation_name;
  const char *file_name;
} LoadData;

typedef struct {
  const char *relation_name;
} ShowIndex;

union Queries {
  Selects selection;
  Inserts insertion;
  Deletes deletion;
  Updates update;
  CreateTable create_table;
  DropTable drop_table;
  CreateIndex create_index;
  DropIndex drop_index;
  DescTable desc_table;
  LoadData load_data;
  ShowIndex show_index;
  char *errors;
};

// 修改yacc中相关数字编码为宏定义
enum SqlCommandFlag {
  SCF_ERROR = 0,
  SCF_SELECT,
  SCF_INSERT,
  SCF_UPDATE,
  SCF_DELETE,
  SCF_CREATE_TABLE,
  SCF_DROP_TABLE,
  SCF_CREATE_INDEX,
  SCF_DROP_INDEX,
  SCF_SYNC,
  SCF_SHOW_TABLES,
  SCF_DESC_TABLE,
  SCF_BEGIN,
  SCF_COMMIT,
  SCF_CLOG_SYNC,
  SCF_ROLLBACK,
  SCF_LOAD_DATA,
  SCF_HELP,
  SCF_EXIT,
  SCF_SHOW_INDEX,
  SCF_SELECT_NONTABLE,
};
// struct of flag and sql_struct
typedef struct Query {
  enum SqlCommandFlag flag;
  union Queries sstr;
} Query;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void relation_attr_init(RelAttr *relation_attr, const char *relation_name, const char *attribute_name);
void relation_attr_exp_init(RelAttr *relation_attr, const char *expression);
void relation_attr_aggr_init(
    RelAttr *relation_attr, const char *relation_name, const char *attribute_name, AggType aggrType);
void relation_attr_destroy(RelAttr *relation_attr);
void group_attr_init(GroupAttr *group_attr, const char *relation_name, const char *attribute_name);
void order_attr_init(OrderAttr *order_attr, const char *relation_name, const char *attribute_name, OrderType orderType);

void value_init_integer(Value *value, int v);
void value_init_float(Value *value, float v);
int value_init_date(Value *value, const char *v);
void value_init_string(Value *value, const char *v);
void value_init_null(Value *value);
void value_init_sub_select(Value *value, Selects * select);
void value_init_set(Value *value, Value * set_values, size_t set_num);

void value_destroy(Value *value);

void condition_init(Condition *condition, CompOp comp, int left_is_attr, RelAttr *left_attr, Value *left_value,
    int right_is_attr, RelAttr *right_attr, Value *right_value);
void condition_conj_init(Condition *condition, CompOp comp, int left_is_attr, RelAttr *left_attr, Value *left_value,
    int right_is_attr, RelAttr *right_attr, Value *right_value, Conjunction conj);
void condition_destroy(Condition *condition);

void attr_info_init(AttrInfo *attr_info, const char *name, AttrType type, size_t length);
void nullable_attr_info_init(AttrInfo *attr_info, const char *name, AttrType type, size_t length);
void attr_info_destroy(AttrInfo *attr_info);

void selects_init(Selects *selects, ...);
void selects_append_attribute(Selects *selects, RelAttr *rel_attr);
void selects_append_relation(Selects *selects, const char *relation_name);
void selects_append_condition(Selects *selects, Condition * condition);
void selects_append_conditions(Selects *selects, Condition conditions[], size_t condition_num);
void selects_append_having_conditions(Selects *selects, Condition conditions[], size_t condition_num);
void selects_append_having_condition(Selects *selects, Condition * condition);
void selects_destroy(Selects *selects);

void group_append_attribute(Selects *selects, GroupAttr *group_attr);
void order_append_attribute(Selects *selects, OrderAttr *order_attr);

void inserts_init(Inserts *inserts, const char *relation_name, InsertRecord records[], size_t record_num);
void insert_record_init(InsertRecord *records, Value values[], size_t value_num);
void inserts_destroy(Inserts *inserts);

void deletes_init_relation(Deletes *deletes, const char *relation_name);
void deletes_set_conditions(Deletes *deletes, Condition conditions[], size_t condition_num);
void deletes_destroy(Deletes *deletes);

void updates_init(Updates *updates, const char *relation_name,
    Condition conditions[], size_t condition_num);
void updates_append_attr_and_value(Updates * updates, const char * attr_name, Value * value);
void updates_destroy(Updates *updates);

void create_table_append_attribute(CreateTable *create_table, AttrInfo *attr_info);
void create_table_init_name(CreateTable *create_table, const char *relation_name);
void create_table_destroy(CreateTable *create_table);

void drop_table_init(DropTable *drop_table, const char *relation_name);
void show_index_init(ShowIndex * show_index, const char * relation_name);

void drop_table_destroy(DropTable *drop_table);

void create_index_init(CreateIndex *create_index, const char *index_name, const char *relation_name, const char *attr_name);
void create_index_multi_rel_init(CreateIndex *create_index, const char *index_name, const char *relation_name, size_t is_unique);
void create_index_append_attr_name(CreateIndex *create_index, const char *attr_name);
void create_index_destroy(CreateIndex *create_index);

void drop_index_init(DropIndex *drop_index, const char *index_name);
void drop_index_destroy(DropIndex *drop_index);
void show_index_destroy(ShowIndex *show_index);

void desc_table_init(DescTable *desc_table, const char *relation_name);
void desc_table_destroy(DescTable *desc_table);

void load_data_init(LoadData *load_data, const char *relation_name, const char *file_name);
void load_data_destroy(LoadData *load_data);

void query_init(Query *query);
Query *query_create();  // create and init
void query_reset(Query *query);
void query_destroy(Query *query);  // reset and delete

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __OBSERVER_SQL_PARSER_PARSE_DEFS_H__
