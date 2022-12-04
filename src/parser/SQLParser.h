
// Generated from .\SQL.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"




class  SQLParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, T__25 = 26, 
    T__26 = 27, T__27 = 28, T__28 = 29, T__29 = 30, T__30 = 31, T__31 = 32, 
    T__32 = 33, T__33 = 34, T__34 = 35, T__35 = 36, T__36 = 37, T__37 = 38, 
    T__38 = 39, T__39 = 40, T__40 = 41, T__41 = 42, T__42 = 43, T__43 = 44, 
    T__44 = 45, T__45 = 46, T__46 = 47, T__47 = 48, T__48 = 49, T__49 = 50, 
    T__50 = 51, EqualOrAssign = 52, Less = 53, LessEqual = 54, Greater = 55, 
    GreaterEqual = 56, NotEqual = 57, Count = 58, Average = 59, Max = 60, 
    Min = 61, Sum = 62, Null = 63, Identifier = 64, Integer = 65, String = 66, 
    Float = 67, Whitespace = 68, Annotation = 69
  };

  enum {
    RuleProgram = 0, RuleStatement = 1, RuleDbStatement = 2, RuleIoStatement = 3, 
    RuleTableStatement = 4, RuleSelectTable = 5, RuleAlterStatement = 6, 
    RuleFieldList = 7, RuleField = 8, RuleType = 9, RuleValueLists = 10, 
    RuleValueList = 11, RuleValue = 12, RuleWhereAndClause = 13, RuleWhereClause = 14, 
    RuleColumn = 15, RuleExpression = 16, RuleSetClause = 17, RuleSelectors = 18, 
    RuleSelector = 19, RuleIdentifiers = 20, RuleOperator_ = 21, RuleAggregator = 22
  };

  explicit SQLParser(antlr4::TokenStream *input);

  SQLParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~SQLParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class ProgramContext;
  class StatementContext;
  class DbStatementContext;
  class IoStatementContext;
  class TableStatementContext;
  class SelectTableContext;
  class AlterStatementContext;
  class FieldListContext;
  class FieldContext;
  class TypeContext;
  class ValueListsContext;
  class ValueListContext;
  class ValueContext;
  class WhereAndClauseContext;
  class WhereClauseContext;
  class ColumnContext;
  class ExpressionContext;
  class SetClauseContext;
  class SelectorsContext;
  class SelectorContext;
  class IdentifiersContext;
  class Operator_Context;
  class AggregatorContext; 

  class  ProgramContext : public antlr4::ParserRuleContext {
  public:
    ProgramContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EOF();
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ProgramContext* program();

  class  StatementContext : public antlr4::ParserRuleContext {
  public:
    StatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    DbStatementContext *dbStatement();
    IoStatementContext *ioStatement();
    TableStatementContext *tableStatement();
    AlterStatementContext *alterStatement();
    antlr4::tree::TerminalNode *Annotation();
    antlr4::tree::TerminalNode *Null();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StatementContext* statement();

  class  DbStatementContext : public antlr4::ParserRuleContext {
  public:
    DbStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    DbStatementContext() = default;
    void copyFrom(DbStatementContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  DropDbContext : public DbStatementContext {
  public:
    DropDbContext(DbStatementContext *ctx);

    antlr4::tree::TerminalNode *Identifier();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  UseDbContext : public DbStatementContext {
  public:
    UseDbContext(DbStatementContext *ctx);

    antlr4::tree::TerminalNode *Identifier();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ShowIndexesContext : public DbStatementContext {
  public:
    ShowIndexesContext(DbStatementContext *ctx);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  CreateDbContext : public DbStatementContext {
  public:
    CreateDbContext(DbStatementContext *ctx);

    antlr4::tree::TerminalNode *Identifier();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ShowTablesContext : public DbStatementContext {
  public:
    ShowTablesContext(DbStatementContext *ctx);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ShowDbsContext : public DbStatementContext {
  public:
    ShowDbsContext(DbStatementContext *ctx);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  DbStatementContext* dbStatement();

  class  IoStatementContext : public antlr4::ParserRuleContext {
  public:
    IoStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    IoStatementContext() = default;
    void copyFrom(IoStatementContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  DumpDataContext : public IoStatementContext {
  public:
    DumpDataContext(IoStatementContext *ctx);

    antlr4::tree::TerminalNode *String();
    antlr4::tree::TerminalNode *Identifier();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LoadDataContext : public IoStatementContext {
  public:
    LoadDataContext(IoStatementContext *ctx);

    antlr4::tree::TerminalNode *String();
    antlr4::tree::TerminalNode *Identifier();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  IoStatementContext* ioStatement();

  class  TableStatementContext : public antlr4::ParserRuleContext {
  public:
    TableStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    TableStatementContext() = default;
    void copyFrom(TableStatementContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  CreateTableContext : public TableStatementContext {
  public:
    CreateTableContext(TableStatementContext *ctx);

    antlr4::tree::TerminalNode *Identifier();
    FieldListContext *fieldList();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  DeleteFromTableContext : public TableStatementContext {
  public:
    DeleteFromTableContext(TableStatementContext *ctx);

    antlr4::tree::TerminalNode *Identifier();
    WhereAndClauseContext *whereAndClause();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  DescribeTableContext : public TableStatementContext {
  public:
    DescribeTableContext(TableStatementContext *ctx);

    antlr4::tree::TerminalNode *Identifier();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  UpdateTableContext : public TableStatementContext {
  public:
    UpdateTableContext(TableStatementContext *ctx);

    antlr4::tree::TerminalNode *Identifier();
    SetClauseContext *setClause();
    WhereAndClauseContext *whereAndClause();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  SelectTable_Context : public TableStatementContext {
  public:
    SelectTable_Context(TableStatementContext *ctx);

    SelectTableContext *selectTable();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  DropTableContext : public TableStatementContext {
  public:
    DropTableContext(TableStatementContext *ctx);

    antlr4::tree::TerminalNode *Identifier();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  InsertIntoTableContext : public TableStatementContext {
  public:
    InsertIntoTableContext(TableStatementContext *ctx);

    antlr4::tree::TerminalNode *Identifier();
    ValueListsContext *valueLists();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  TableStatementContext* tableStatement();

  class  SelectTableContext : public antlr4::ParserRuleContext {
  public:
    SelectTableContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    SelectorsContext *selectors();
    IdentifiersContext *identifiers();
    WhereAndClauseContext *whereAndClause();
    ColumnContext *column();
    std::vector<antlr4::tree::TerminalNode *> Integer();
    antlr4::tree::TerminalNode* Integer(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SelectTableContext* selectTable();

  class  AlterStatementContext : public antlr4::ParserRuleContext {
  public:
    AlterStatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    AlterStatementContext() = default;
    void copyFrom(AlterStatementContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  AlterDropIndexContext : public AlterStatementContext {
  public:
    AlterDropIndexContext(AlterStatementContext *ctx);

    antlr4::tree::TerminalNode *Identifier();
    IdentifiersContext *identifiers();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AlterAddIndexContext : public AlterStatementContext {
  public:
    AlterAddIndexContext(AlterStatementContext *ctx);

    antlr4::tree::TerminalNode *Identifier();
    IdentifiersContext *identifiers();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AlterTableDropForeignKeyContext : public AlterStatementContext {
  public:
    AlterTableDropForeignKeyContext(AlterStatementContext *ctx);

    std::vector<antlr4::tree::TerminalNode *> Identifier();
    antlr4::tree::TerminalNode* Identifier(size_t i);

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AlterTableDropPkContext : public AlterStatementContext {
  public:
    AlterTableDropPkContext(AlterStatementContext *ctx);

    std::vector<antlr4::tree::TerminalNode *> Identifier();
    antlr4::tree::TerminalNode* Identifier(size_t i);

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AlterTableAddPkContext : public AlterStatementContext {
  public:
    AlterTableAddPkContext(AlterStatementContext *ctx);

    std::vector<antlr4::tree::TerminalNode *> Identifier();
    antlr4::tree::TerminalNode* Identifier(size_t i);
    IdentifiersContext *identifiers();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AlterTableAddForeignKeyContext : public AlterStatementContext {
  public:
    AlterTableAddForeignKeyContext(AlterStatementContext *ctx);

    std::vector<antlr4::tree::TerminalNode *> Identifier();
    antlr4::tree::TerminalNode* Identifier(size_t i);
    std::vector<IdentifiersContext *> identifiers();
    IdentifiersContext* identifiers(size_t i);

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AlterTableAddUniqueContext : public AlterStatementContext {
  public:
    AlterTableAddUniqueContext(AlterStatementContext *ctx);

    antlr4::tree::TerminalNode *Identifier();
    IdentifiersContext *identifiers();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  AlterStatementContext* alterStatement();

  class  FieldListContext : public antlr4::ParserRuleContext {
  public:
    FieldListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<FieldContext *> field();
    FieldContext* field(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FieldListContext* fieldList();

  class  FieldContext : public antlr4::ParserRuleContext {
  public:
    FieldContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    FieldContext() = default;
    void copyFrom(FieldContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  ForeignKeyFieldContext : public FieldContext {
  public:
    ForeignKeyFieldContext(FieldContext *ctx);

    std::vector<IdentifiersContext *> identifiers();
    IdentifiersContext* identifiers(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Identifier();
    antlr4::tree::TerminalNode* Identifier(size_t i);

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  PrimaryKeyFieldContext : public FieldContext {
  public:
    PrimaryKeyFieldContext(FieldContext *ctx);

    IdentifiersContext *identifiers();
    antlr4::tree::TerminalNode *Identifier();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  NormalFieldContext : public FieldContext {
  public:
    NormalFieldContext(FieldContext *ctx);

    antlr4::tree::TerminalNode *Identifier();
    TypeContext *type();
    antlr4::tree::TerminalNode *Null();
    ValueContext *value();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  FieldContext* field();

  class  TypeContext : public antlr4::ParserRuleContext {
  public:
    TypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Integer();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeContext* type();

  class  ValueListsContext : public antlr4::ParserRuleContext {
  public:
    ValueListsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ValueListContext *> valueList();
    ValueListContext* valueList(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ValueListsContext* valueLists();

  class  ValueListContext : public antlr4::ParserRuleContext {
  public:
    ValueListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ValueContext *> value();
    ValueContext* value(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ValueListContext* valueList();

  class  ValueContext : public antlr4::ParserRuleContext {
  public:
    ValueContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Integer();
    antlr4::tree::TerminalNode *String();
    antlr4::tree::TerminalNode *Float();
    antlr4::tree::TerminalNode *Null();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ValueContext* value();

  class  WhereAndClauseContext : public antlr4::ParserRuleContext {
  public:
    WhereAndClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<WhereClauseContext *> whereClause();
    WhereClauseContext* whereClause(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  WhereAndClauseContext* whereAndClause();

  class  WhereClauseContext : public antlr4::ParserRuleContext {
  public:
    WhereClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    WhereClauseContext() = default;
    void copyFrom(WhereClauseContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  WhereOperatorSelectContext : public WhereClauseContext {
  public:
    WhereOperatorSelectContext(WhereClauseContext *ctx);

    ColumnContext *column();
    Operator_Context *operator_();
    SelectTableContext *selectTable();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  WhereInSelectContext : public WhereClauseContext {
  public:
    WhereInSelectContext(WhereClauseContext *ctx);

    ColumnContext *column();
    SelectTableContext *selectTable();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  WhereNullContext : public WhereClauseContext {
  public:
    WhereNullContext(WhereClauseContext *ctx);

    ColumnContext *column();
    antlr4::tree::TerminalNode *Null();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  WhereOperatorExpressionContext : public WhereClauseContext {
  public:
    WhereOperatorExpressionContext(WhereClauseContext *ctx);

    ColumnContext *column();
    Operator_Context *operator_();
    ExpressionContext *expression();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  WhereLikeStringContext : public WhereClauseContext {
  public:
    WhereLikeStringContext(WhereClauseContext *ctx);

    ColumnContext *column();
    antlr4::tree::TerminalNode *String();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  WhereInListContext : public WhereClauseContext {
  public:
    WhereInListContext(WhereClauseContext *ctx);

    ColumnContext *column();
    ValueListContext *valueList();

    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  WhereClauseContext* whereClause();

  class  ColumnContext : public antlr4::ParserRuleContext {
  public:
    ColumnContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> Identifier();
    antlr4::tree::TerminalNode* Identifier(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ColumnContext* column();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ValueContext *value();
    ColumnContext *column();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ExpressionContext* expression();

  class  SetClauseContext : public antlr4::ParserRuleContext {
  public:
    SetClauseContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> Identifier();
    antlr4::tree::TerminalNode* Identifier(size_t i);
    std::vector<antlr4::tree::TerminalNode *> EqualOrAssign();
    antlr4::tree::TerminalNode* EqualOrAssign(size_t i);
    std::vector<ValueContext *> value();
    ValueContext* value(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SetClauseContext* setClause();

  class  SelectorsContext : public antlr4::ParserRuleContext {
  public:
    SelectorsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<SelectorContext *> selector();
    SelectorContext* selector(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SelectorsContext* selectors();

  class  SelectorContext : public antlr4::ParserRuleContext {
  public:
    SelectorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ColumnContext *column();
    AggregatorContext *aggregator();
    antlr4::tree::TerminalNode *Count();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  SelectorContext* selector();

  class  IdentifiersContext : public antlr4::ParserRuleContext {
  public:
    IdentifiersContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> Identifier();
    antlr4::tree::TerminalNode* Identifier(size_t i);


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IdentifiersContext* identifiers();

  class  Operator_Context : public antlr4::ParserRuleContext {
  public:
    Operator_Context(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *EqualOrAssign();
    antlr4::tree::TerminalNode *Less();
    antlr4::tree::TerminalNode *LessEqual();
    antlr4::tree::TerminalNode *Greater();
    antlr4::tree::TerminalNode *GreaterEqual();
    antlr4::tree::TerminalNode *NotEqual();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  Operator_Context* operator_();

  class  AggregatorContext : public antlr4::ParserRuleContext {
  public:
    AggregatorContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Count();
    antlr4::tree::TerminalNode *Average();
    antlr4::tree::TerminalNode *Max();
    antlr4::tree::TerminalNode *Min();
    antlr4::tree::TerminalNode *Sum();


    virtual std::any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  AggregatorContext* aggregator();


  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

