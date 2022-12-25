grammar SQL;

EqualOrAssign: '=';
Less: '<';
LessEqual: '<=';
Greater: '>';
GreaterEqual: '>=';
NotEqual: '<>';

Count: 'COUNT';
Average: 'AVG';
Max: 'MAX';
Min: 'MIN';
Sum: 'SUM';
Null: 'NULL';

Identifier: [a-zA-Z_] [a-zA-Z_0-9]*;
Integer: [0-9]+;  // TODO - handle negative numbers
String:  '\'' (~'\'')* '\'';
Float: ('-')? [0-9]+ '.' [0-9]*;
Whitespace: [ \t\n\r]+ -> skip;
Annotation: '-' '-' (~';')+;

program
    : statement* EOF
    ;

statement
    : dbStatement ';'
    | ioStatement ';'
    | tableStatement ';'
    | alterStatement ';'
    | Annotation ';'
    | Null ';'
    ;

dbStatement
    : 'CREATE' 'DATABASE' Identifier    # createDb
    | 'DROP' 'DATABASE' Identifier      # dropDb
    | 'SHOW' 'DATABASES'                # showDbs
    | 'USE' Identifier                  # useDb                  
    | 'SHOW' 'TABLES'                   # showTables
	| 'SHOW' 'INDEXES'					# showIndexes
    ;

ioStatement
    : 'LOAD' 'FROM' 'FILE' String 'TO' 'TABLE' Identifier     # loadData
    | 'DUMP' 'TO' 'FILE' String 'FROM' 'TABLE' Identifier     # dumpData
    ;

tableStatement
    : 'CREATE' 'TABLE' Identifier '(' fieldList ')'                     # createTable
    | 'DROP' 'TABLE' Identifier                                         # dropTable
    | 'DESC' Identifier                                                 # describeTable
    | 'INSERT' 'INTO' Identifier 'VALUES' valueLists                    # insertIntoTable
    | 'DELETE' 'FROM' Identifier 'WHERE' whereAndClause                 # deleteFromTable
    | 'UPDATE' Identifier 'SET' setClause 'WHERE' whereAndClause        # updateTable
    | selectTable                                                       # selectTable_
    ;

selectTable
    : 'SELECT' selectors 'FROM' identifiers ('WHERE' whereAndClause)? ('GROUP' 'BY' column)? ('LIMIT' Integer ('OFFSET' Integer)?)?
    ;

alterStatement
    : 'ALTER' 'TABLE' Identifier 'ADD' 'INDEX' '(' identifiers ')'   			# alterAddIndex
    | 'ALTER' 'TABLE' Identifier 'DROP' 'INDEX' '(' identifiers ')'             # alterDropIndex
    | 'ALTER' 'TABLE' Identifier 'DROP' 'PRIMARY' 'KEY' (Identifier)?           # alterTableDropPk
    | 'ALTER' 'TABLE' Identifier 'DROP' 'FOREIGN' 'KEY' Identifier              # alterTableDropForeignKey
    | 'ALTER' 'TABLE' Identifier 'ADD' 'CONSTRAINT' (Identifier)? 'PRIMARY' 'KEY' '(' identifiers ')'      # alterTableAddPk
    | 'ALTER' 'TABLE' Identifier 'ADD' 'CONSTRAINT' (Identifier)? 'FOREIGN' 'KEY' '(' identifiers ')' 'REFERENCES' Identifier '(' identifiers ')'  # alterTableAddForeignKey
    | 'ALTER' 'TABLE' Identifier 'ADD' 'UNIQUE' '(' identifiers ')'             # alterTableAddUnique
    ;

fieldList
    : field (',' field)*
    ;

field
    : Identifier type ('NOT' Null)? ('DEFAULT' value)?                                                # normalField
    | 'PRIMARY' 'KEY' (Identifier)? '(' identifiers ')'                                               # primaryKeyField
    | 'FOREIGN' 'KEY' (Identifier)? '(' identifiers ')' 'REFERENCES' Identifier '(' identifiers ')'   # foreignKeyField
    ;

type
    : 'INT'
    | 'VARCHAR' '(' Integer ')'
    | 'FLOAT'
    ;

valueLists
    : valueList (',' valueList)*
    ;

valueList
    : '(' value (',' value)* ')'
    ;

value
    : Integer
    | String
    | Float
    | Null
    ;

whereAndClause
    : whereClause ('AND' whereClause)*
    ;

whereClause
    : column operator_ expression            # whereOperatorExpression
    | column operator_ '(' selectTable ')'   # whereOperatorSelect
    | column 'IS' ('NOT')? Null              # whereNull
    | column 'IN' valueList                  # whereInList
    | column 'IN' '(' selectTable ')'        # whereInSelect
    | column 'LIKE' String                   # whereLikeString
    ;

column
    : (Identifier '.')? Identifier
    ;

expression
    : value
    | column
    ;

setClause
    : Identifier EqualOrAssign value (',' Identifier EqualOrAssign value)*
    ;

selectors
    : '*'
    | selector (',' selector)*
    ;

selector
    : column
    | aggregator '(' column ')'
    | Count '(' '*' ')'
    ;

identifiers
    : Identifier (',' Identifier)*
    ;

operator_
    : EqualOrAssign
    | Less
    | LessEqual
    | Greater
    | GreaterEqual
    | NotEqual
    ;

aggregator
    : Count
    | Average
    | Max
    | Min
    | Sum
    ;
